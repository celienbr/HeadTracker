#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <limits>
#include <initializer_list>
#include "expo_deadzone.h"

// Firmware zone-morte: independent directions, full endpoints,
// zero-width bitwise-compatible with original Expo, monotonic movement.
int main() {
    unsigned long long n = 0;
    for (unsigned lo : {988u,1050u,1200u})
    for (unsigned hi : {1800u,1950u,2012u})
    for (unsigned center : {1300u,1500u,1700u})
    for (unsigned expo : {0u,25u,65u,100u})
    for (unsigned lower : {0u,5u,30u,120u,250u})
    for (unsigned upper : {0u,5u,30u,120u,250u}) {
        if (!(lo<center && center<hi)) continue;
        uint16_t prev = 0;
        for (int i=-3000;i<=3000;i+=5) {
            float delta=i/2.f;
            uint16_t out=expoDeadzoneChannel(delta,center,lo,hi,expo,lower,upper);
            assert(out>=lo && out<=hi);
            assert(out>=prev);
            prev=out;
            if(lower==0 && upper==0)
                assert(out==expoChannel(delta,center,lo,hi,expo));
            if(delta>=-float(lower) && delta<=float(upper) &&
               -delta <= float(center-lo)-1 && delta <= float(hi-center)-1)
                assert(out==center);
            ++n;
        }
        assert(expoDeadzoneChannel(0,center,lo,hi,expo,lower,upper)==center);
        assert(expoDeadzoneChannel(-10000,center,lo,hi,expo,lower,upper)==lo);
        assert(expoDeadzoneChannel(10000,center,lo,hi,expo,lower,upper)==hi);
    }
    assert(expoDeadzoneChannel(-12,1500,1000,2000,40,20,0)==1500);
    assert(expoDeadzoneChannel(12,1500,1000,2000,40,20,0)>1500);
    assert(expoDeadzoneChannel(12,1500,1000,2000,40,0,20)==1500);
    assert(expoDeadzoneChannel(-12,1500,1000,2000,40,0,20)<1500);
    assert(expoDeadzoneChannel(std::numeric_limits<float>::quiet_NaN(),1500,1000,2000,80,20,20)==1500);
    puts("PASS independent deadzones, 0us backward compatibility, end-stop preservation and monotonicity");
    printf("checked %llu input cases\n",n);
}
