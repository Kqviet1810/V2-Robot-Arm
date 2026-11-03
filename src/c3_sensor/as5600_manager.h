#pragma once
#include <Arduino.h>

struct AxisSensorData {
    uint16_t rawAngle = 0;       // 12-bit raw angle (0 - 4095)
    float degree = 0.0f;          // Current single-turn angle (0.0 - 359.9°)
    float continuousDegree = 0.0f;// Multi-turn continuous angle
    int32_t turns = 0;           // Number of full rotations
    bool magnetDetected = false;
    bool magnetTooStrong = false;
    bool magnetTooWeak = false;
    bool healthy = false;
};

class AS5600Manager {
public:
    AS5600Manager();
    bool begin();
    void updateAll();
    
    const AxisSensorData& getAxisData(uint8_t axisIndex) const;
    void resetContinuousTurns(uint8_t axisIndex);
    void zeroCalibration(uint8_t axisIndex);

private:
    static constexpr uint8_t AS5600_ADDR = 0x36;
    static constexpr uint8_t REG_STATUS = 0x0B;
    static constexpr uint8_t REG_RAW_ANGLE = 0x0C; // 0x0C high, 0x0D low (filtered angle)

    AxisSensorData sensors[4];
    uint16_t lastRaw[4] = {0, 0, 0, 0};
    uint16_t zeroOffsets[4] = {0, 0, 0, 0};

    bool readSensor(uint8_t channel, AxisSensorData& data, uint16_t& prevRaw, uint16_t zeroOffset);
};

