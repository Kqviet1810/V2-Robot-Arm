#pragma once
#include <Arduino.h>
#include "config.h"

struct ArmActualState {
    float actualAngle[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    bool sensorHealthy[4] = {false, false, false, false};
    uint8_t rawStatus = 0;
    unsigned long lastUpdateTime = 0;
    bool isConnected = false;
};

class C3Receiver {
public:
    C3Receiver();
    void begin();
    void update();
    
    const ArmActualState& getState() const { return state; }
    void sendCalibrateCommand(uint8_t axis);

private:
    HardwareSerial c3Serial;
    ArmActualState state;
    String rxBuffer;

    void parsePacket(const String& line);
};

