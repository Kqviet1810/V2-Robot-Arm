#include "as5600_manager.h"
#include "tca9548a.h"
#include <Wire.h>

AS5600Manager::AS5600Manager() {}

bool AS5600Manager::begin() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    delay(50);
    return TCA9548A::isPresent();
}

bool AS5600Manager::readSensor(uint8_t channel, AxisSensorData& data, uint16_t& prevRaw, uint16_t zeroOffset) {
    if (!TCA9548A::selectChannel(channel)) {
        data.healthy = false;
        return false;
    }

    // 1. Read Magnet Status (0x0B)
    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(REG_STATUS);
    if (Wire.endTransmission() != 0) {
        data.healthy = false;
        return false;
    }

    Wire.requestFrom(AS5600_ADDR, (uint8_t)1);
    if (Wire.available()) {
        uint8_t status = Wire.read();
        data.magnetDetected  = (status & 0x20) != 0; // MD bit 5
        data.magnetTooWeak   = (status & 0x10) != 0; // ML bit 4
        data.magnetTooStrong = (status & 0x08) != 0; // MH bit 3
    } else {
        data.healthy = false;
        return false;
    }

    // 2. Read 12-bit Angle (0x0C & 0x0D)
    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(REG_RAW_ANGLE);
    if (Wire.endTransmission() != 0) {
        data.healthy = false;
        return false;
    }

    Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
    if (Wire.available() >= 2) {
        uint8_t high = Wire.read();
        uint8_t low = Wire.read();
        uint16_t currentRaw = ((high & 0x0F) << 8) | low;

        data.rawAngle = currentRaw;
        
        // Zero offset adjustment (mod 4096)
        int32_t calibratedRaw = (int32_t)currentRaw - zeroOffset;
        if (calibratedRaw < 0) calibratedRaw += 4096;
        data.degree = (float)calibratedRaw * (360.0f / 4096.0f);

        // Multi-turn unwrapping logic
        int32_t diff = (int32_t)currentRaw - (int32_t)prevRaw;
        if (diff > 2048) {
            data.turns--; // Wrapped backwards through 0
        } else if (diff < -2048) {
            data.turns++; // Wrapped forwards through 4095
        }
        prevRaw = currentRaw;

        data.continuousDegree = ((float)data.turns * 360.0f) + data.degree;
        data.healthy = data.magnetDetected;
        return true;
    }

    data.healthy = false;
    return false;
}

void AS5600Manager::updateAll() {
    for (uint8_t i = 0; i < NUM_AXES; i++) {
        readSensor(i, sensors[i], lastRaw[i], zeroOffsets[i]);
    }
}

const AxisSensorData& AS5600Manager::getAxisData(uint8_t axisIndex) const {
    if (axisIndex >= NUM_AXES) return sensors[0];
    return sensors[axisIndex];
}

void AS5600Manager::resetContinuousTurns(uint8_t axisIndex) {
    if (axisIndex < NUM_AXES) {
        sensors[axisIndex].turns = 0;
    }
}

void AS5600Manager::zeroCalibration(uint8_t axisIndex) {
    if (axisIndex < NUM_AXES) {
        zeroOffsets[axisIndex] = sensors[axisIndex].rawAngle;
        sensors[axisIndex].turns = 0;
    }
}

