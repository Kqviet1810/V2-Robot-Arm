#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "c3_config.h"

class TCA9548A {
public:
    static bool selectChannel(uint8_t channel) {
        if (channel > 7) return false;
        Wire.beginTransmission(TCA9548A_ADDR);
        Wire.write(1 << channel);
        return (Wire.endTransmission() == 0);
    }

    static bool disableAllChannels() {
        Wire.beginTransmission(TCA9548A_ADDR);
        Wire.write(0);
        return (Wire.endTransmission() == 0);
    }

    static bool isPresent() {
        Wire.beginTransmission(TCA9548A_ADDR);
        return (Wire.endTransmission() == 0);
    }
};

