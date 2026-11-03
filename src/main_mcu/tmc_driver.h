#pragma once
#include <Arduino.h>
#include <TMCStepper.h>
#include "config.h"

class TmcManager {
public:
    TmcManager();
    void begin();
    void enableSteppers(bool enable);
    bool checkDrivers();
    void setStallguardThreshold(uint8_t threshold);
    
    TMC2209Stepper* getDriver(uint8_t index);

private:
    HardwareSerial tmcSerial;
    TMC2209Stepper driverX;
    TMC2209Stepper driverY;
    TMC2209Stepper driverZ;
    TMC2209Stepper driverA;
    TMC2209Stepper* drivers[4];
};

