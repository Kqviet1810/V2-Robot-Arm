#pragma once
#include <Arduino.h>
#include <FastAccelStepper.h>
#include "config.h"

struct AxisMotionState {
    float currentPos = 0.0f; // in degrees or mm
    float targetPos = 0.0f;
    bool isMoving = false;
    bool limitHit = false;
};

class MotionController {
public:
    MotionController();
    void begin();
    
    void moveTo(float x, float y, float z, float a);
    void moveAxisTo(uint8_t axis, float target);
    void jogAxis(uint8_t axis, float delta);
    
    void stopAll();
    void emergencyStop();
    void setCurrentPositionAsZero();

    void update(); // Poll limits & state

    AxisMotionState getAxisState(uint8_t axis) const;
    bool isAnyMoving() const;

private:
    FastAccelStepperEngine engine;
    FastAccelStepper* stepperX = nullptr;
    FastAccelStepper* stepperY = nullptr;
    FastAccelStepper* stepperZ = nullptr;
    FastAccelStepper* stepperA = nullptr;
    FastAccelStepper* steppers[4] = {nullptr, nullptr, nullptr, nullptr};

    float stepsPerUnit[4] = {X_STEPS_PER_UNIT, Y_STEPS_PER_UNIT, Z_STEPS_PER_UNIT, A_STEPS_PER_UNIT};
    uint8_t limitPins[4] = {PIN_X_LIMIT_NEG, PIN_Y_LIMIT_NEG, PIN_Z_LIMIT_NEG, PIN_A_LIMIT_POS};
    bool limitActiveHigh[4] = {false, false, false, false}; // Low when triggered or high when triggered
};

