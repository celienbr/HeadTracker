#!/usr/bin/env python3
"""Compile the ACTUAL patched firmware serialWriteJSON and validate its wire bytes.

This catches a v2.40 bug: outgoing CRC used an uninitialised leading byte and
missed the final JSON byte; CRC was also transmitted MSB first. The Expo GUI
validates replies and expects the same LE CRC as the firmware input decoder.
"""
import pathlib
import subprocess
import sys
import tempfile

serial = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
src_dir = pathlib.Path(sys.argv[2]).resolve()

def extract(signature):
    start = serial.find(signature)
    if start < 0:
        raise RuntimeError("Missing actual firmware definition " + signature)
    brace = serial.find("{", start + len(signature))
    if brace < 0:
        raise RuntimeError("No brace: " + signature)
    depth = 0
    for i in range(brace, len(serial)):
        if serial[i] == "{":
            depth += 1
        elif serial[i] == "}":
            depth -= 1
            if depth == 0:
                return serial[start:i + 1]
    raise RuntimeError("Unbalanced braces")

escape = extract("\nuint16_t escapeCRC(uint16_t crc)")
send = extract("\nvoid serialWriteJSON(JsonDocument &json)")
cpp = r"""
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "ucrc16lib.h"

#define TX_RNGBUF_SIZE 2048
#define K_FOREVER 0

int ring_tx_mutex = 0;
std::vector<unsigned char> wire;
void k_mutex_lock(int*, int) {}
void k_mutex_unlock(int*) {}

struct JsonDocument { std::string body; };
int serializeJson(JsonDocument &json, char *dest, size_t capacity) {
    if (json.body.size() > capacity) return 0;
    memcpy(dest, json.body.data(), json.body.size());
    return static_cast<int>(json.body.size());
}
void serialWrite(const char *buf, int len) {
    wire.assign(reinterpret_cast<const unsigned char*>(buf),
                reinterpret_cast<const unsigned char*>(buf) + len);
}
""" + escape + "\n" + send + r"""

int main() {
    for (const std::string &json : {
        std::string("{\"Cmd\":\"FW\",\"Hard\":\"arduino_nano_33_ble\",\"Vers\":\"v2.40\"}"),
        std::string("{\"Cmd\":\"Set\",\"panexpo\":35,\"tltexpo\":50,\"rllexpo\":0}"),
        std::string("{\"Cmd\":\"FE\",\"FEAT\":[\"BT\",\"PPM\",\"CRSF\"]}")
    }) {
        JsonDocument doc{json};
        serialWriteJSON(doc);
        assert(wire.size() == json.size() + 6);
        assert(wire.at(0) == 0x02);
        assert(wire.at(json.size() + 3) == 0x03);
        assert(wire.at(json.size() + 4) == '\r');
        assert(wire.at(json.size() + 5) == '\n');
        const std::string payload(wire.begin() + 1,
                                  wire.begin() + 1 + json.size());
        assert(payload == json);
        // This matches BoardJson::dataIn()'s expected CRC over the JSON.
        std::string crc_input = payload;
        const uint16_t calculated =
            escapeCRC(uCRC16Lib::calculate(&crc_input[0], crc_input.size()));
        const uint16_t received =
            uint16_t(wire.at(json.size() + 1)) |
            (uint16_t(wire.at(json.size() + 2)) << 8);
        assert(received == calculated);
        // Modifying an in-flight payload must invalidate the CRC.
        crc_input.at(8) ^= 1;
        const uint16_t damaged =
            escapeCRC(uCRC16Lib::calculate(&crc_input[0], crc_input.size()));
        assert(received != damaged);
    }
    puts("PASS: real firmware serialWriteJSON uses correct CRC data range, "
         "little-endian byte order, STX/ETX and CRLF; GUI-compatible FW/Set/FE packets.");
}
"""
with tempfile.TemporaryDirectory(prefix="ht-wire-") as temp:
    source = pathlib.Path(temp) / "wire_test.cpp"
    binary = pathlib.Path(temp) / "wire_test"
    source.write_text(cpp, encoding="utf-8")
    subprocess.run([
        "g++", "-std=c++11", "-O1", "-Wall", "-Wextra",
        "-fsanitize=undefined", "-fno-sanitize-recover=all",
        "-I", str(src_dir / "include"),
        str(source), str(src_dir / "ucrc16lib.cpp"),
        "-o", str(binary)
    ], check=True)
    subprocess.run([str(binary)], check=True)
