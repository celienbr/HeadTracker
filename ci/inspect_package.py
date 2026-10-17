#!/usr/bin/env python3
"""Checks intended payload, firmware vectors and hashes; no device access."""
import argparse, hashlib, json, pathlib, struct

p = argparse.ArgumentParser()
p.add_argument("directory", type=pathlib.Path)
p.add_argument("--firmware", action="store_true")
args = p.parse_args()
root = args.directory
manifest = {}
for path in sorted(root.rglob("*")):
    if not path.is_file() or path.name == "SHA256SUMS.json": continue
    content = path.read_bytes()
    manifest[path.relative_to(root).as_posix()] = {"size":len(content),"sha256":hashlib.sha256(content).hexdigest()}
    if path.suffix.lower() == ".bin":
        sp, reset = struct.unpack_from("<II", content)
        assert 0x20000000 <= sp <= 0x20040000 and sp % 8 == 0, (path,sp)
        assert reset & 1 and 0x10000 <= (reset & ~1) < 0x10000 + len(content), (path,reset)
        assert len(content) < 0xe8000
        assert all(k in content for k in (b"panexpo",b"tltexpo",b"rllexpo"))
if args.firmware:
    assert {p.name for p in root.glob("*.bin")} == {"Nano33BLE.bin","Nano33BLE-Rev2.bin"}
    assert (root/"Nano33BLE.bin").read_bytes() != (root/"Nano33BLE-Rev2.bin").read_bytes()
else:
    for name in ["HeadTracker.exe","bossac.exe","Qt6Core.dll","Qt6Gui.dll","Qt6Widgets.dll",
                 "Qt6Network.dll","Qt6SerialPort.dll","Qt6OpenGLWidgets.dll","platforms/qwindows.dll",
                 "libgcc_s_seh-1.dll","libstdc++-6.dll","libwinpthread-1.dll"]:
        assert (root/name).is_file(), name
    assert not list(root.rglob("*.o")) and not list(root.rglob("*.cpp"))
(root/"SHA256SUMS.json").write_text(json.dumps(manifest,indent=2)+"\n")
print(f"PASS: {len(manifest)} payload files and hashes; firmware vectors checked where present")
