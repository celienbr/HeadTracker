#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include "expo_curve.h"
#include "basetrackersettings.h"
#include "flash_settings.h"

int main() {
    unsigned long checks = 0;
    // All integer percentages, asymmetric travels, offsets, zero-side travel.
    for (unsigned c : {1000u, 1201u, 1500u, 1789u, 2000u}) {
        for (unsigned e = 0; e <= 100; ++e) {
            uint16_t previous = 0;
            for (int i = -12000; i <= 12000; ++i) {
                const float d = i / 4.0f;
                const uint16_t y = expoChannel(d, c, 1000, 2000, e);
                assert(y >= 1000 && y <= 2000 && y >= previous);
                previous = y;
                if (e == 0) {
                    const float raw = c + d;
                    const uint16_t expected = raw < 1000 ? 1000 : raw > 2000 ? 2000 : uint16_t(raw);
                    assert(y == expected);
                }
                ++checks;
            }
            assert(expoChannel(0, c, 1000, 2000, e) == c);
            if (c > 1000) assert(expoChannel(-float(c - 1000), c, 1000, 2000, e) == 1000);
            if (c < 2000) assert(expoChannel(float(2000 - c), c, 1000, 2000, e) == 2000);
        }
    }
    assert(expoChannel(250,1500,1000,2000,0) == 1750);
    assert(expoChannel(250,1500,1000,2000,50) == 1656);
    assert(expoChannel(250,1500,1000,2000,100) == 1562);
    assert(expoChannel(-250,1500,1000,2000,100) == 1437);
    assert(expoChannel(.75f,1500,1000,2000,0) == 1500);
    assert(expoChannel(-100000,1500,1000,2000,0) == 1000);
    assert(expoChannel(100000,1500,1000,2000,0) == 2000);
    assert(expoChannel(0,1500,1000,1000,50) == 1000);
    assert(expoChannel(0,500,1000,2000,50) == 1000);
    assert(expoChannel(0,2500,1000,2000,50) == 2000);
    assert(expoChannel(std::numeric_limits<float>::quiet_NaN(),1500,1000,2000,50) == 1500);
    assert(expoChannel(std::numeric_limits<float>::infinity(),1500,1000,2000,50) == 1500);
    assert(expoChannel(250,1500,1000,2000,255) == expoChannel(250,1500,1000,2000,100));
    // Gain and inversion feed signed deltas before Expo; mirrored outputs differ <=1 us (truncation).
    for (float gain : {0.1f, 5.f, 35.f}) for(int angle=-180;angle<=180;++angle) {
        int a=expoChannel(angle*gain,1500,1000,2000,65);
        int b=expoChannel(-angle*gain,1500,1000,2000,65);
        assert(std::abs(a+b-3000)<=1);
    }
    BaseTrackerSettings s;
    assert(s.getPanExpo()==0 && s.getTltExpo()==0 && s.getRllExpo()==0);
    JsonDocument old;
    deserializeJson(old,"{\"pan_gain\":7,\"pan_cnt\":1537,\"servoreverse\":7}");
    s.loadJSONSettings(old);
    assert(s.getPanExpo()==0 && s.getPan_Gain()==7 && s.getPan_Cnt()==1537);
    for (int e=0;e<=100;++e) {
        JsonDocument j;
        j["panexpo"]=e; j["tltexpo"]=100-e; j["rllexpo"]=e/2;
        s.loadJSONSettings(j);
        assert(s.getPanExpo()==e && s.getTltExpo()==100-e && s.getRllExpo()==e/2);
        assert(s.getPan_Gain()==7 && s.getPan_Cnt()==1537 && s.getServoReverse()==7);
        JsonDocument stored; s.setJSONSettings(stored);
        uint8_t bytes[2000];
        size_t len=serializeFlashSettings(stored,bytes,sizeof(bytes));
        assert(len && len%4==0 && bytes[measureJson(stored)]==0);
        JsonDocument restored;
        assert(deserializeJson(restored,bytes,len)==DeserializationError::Ok);
        BaseTrackerSettings rebooted; rebooted.loadJSONSettings(restored);
        assert(rebooted.getPanExpo()==e && rebooted.getTltExpo()==100-e && rebooted.getRllExpo()==e/2);
        assert(rebooted.getPan_Gain()==7 && rebooted.getPan_Cnt()==1537);
    }
    for(const char *bad : {"-1","101","255","256","99999999999999","35.5","\"bad\"","true","null"}) {
        s.setPanExpo(42); s.setTltExpo(43); s.setRllExpo(44);
        std::string text=std::string("{\"panexpo\":")+bad+",\"tltexpo\":"+bad+",\"rllexpo\":"+bad+"}";
        JsonDocument j; assert(!deserializeJson(j,text)); s.loadJSONSettings(j);
        assert(s.getPanExpo()==42 && s.getTltExpo()==43 && s.getRllExpo()==44);
    }
    JsonDocument j; s.setJSONSettings(j); uint8_t small[16]; memset(small,0xAB,sizeof small);
    assert(serializeFlashSettings(j,small,sizeof small)==0 && small[0]==0xAB);
    printf("PASS: %lu curve samples; 101 percentages; gain/inversion; bounds; nonfinite inputs; old JSON; invalid inputs; 101 serialized flash round trips. JSON=%zu bytes. Physical flash NOT tested.\n", checks,measureJson(j));
}
