#include "motion_controller.h"

MotionController::MotionController() {}

void MotionController::begin() {
    engine.init();

    // Limit switch pins (ESP32 pins 34, 35, 36, 39 are input-only, no internal pullup)
    for (int i = 0; i < 4; i++) {
        pinMode(limitPins[i], INPUT);
    }

    // Connect steppers to pins
    stepperX = engine.stepperConnectToPin(PIN_X_STEP);
    if (stepperX) {
        stepperX->setDirectionPin(PIN_X_DIR, false); // Direction false = low
        stepperX->setSpeedInHz(X_MAX_SPEED);
        stepperX->setAcceleration(X_ACCEL * X_STEPS_PER_UNIT);
        steppers[0] = stepperX;
    }

    stepperY = engine.stepperConnectToPin(PIN_Y_STEP);
    if (stepperY) {
        stepperY->setDirectionPin(PIN_Y_DIR, false);
        stepperY->setSpeedInHz(Y_MAX_SPEED);
        stepperY->setAcceleration(Y_ACCEL * Y_STEPS_PER_UNIT);
        steppers[1] = stepperY;
    }

    stepperZ = engine.stepperConnectToPin(PIN_Z_STEP);
    if (stepperZ) {
        stepperZ->setDirectionPin(PIN_Z_DIR, false);
        stepperZ->setSpeedInHz(Z_MAX_SPEED);
        stepperZ->setAcceleration(Z_ACCEL * Z_STEPS_PER_UNIT);
        steppers[2] = stepperZ;
    }

    stepperA = engine.stepperConnectToPin(PIN_A_STEP);
    if (stepperA) {
        stepperA->setDirectionPin(PIN_A_DIR, true);
        stepperA->setSpeedInHz(A_MAX_SPEED);
        stepperA->setAcceleration(A_ACCEL * A_STEPS_PER_UNIT);
        steppers[3] = stepperA;
    }
}

void MotionController::moveTo(float x, float y, float z, float a) {
    float targets[4] = {x, y, z, a};
    for (int i = 0; i < 4; i++) {
        if (steppers[i]) {
            int32_t targetSteps = (int32_t)(targets[i] * stepsPerUnit[i]);
            steppers[i]->moveTo(targetSteps);
        }
    }
}

void MotionController::moveAxisTo(uint8_t axis, float target) {
    if (axis < 4 && steppers[axis]) {
        int32_t targetSteps = (int32_t)(target * stepsPerUnit[axis]);
        steppers[axis]->moveTo(targetSteps);
    }
}

void MotionController::jogAxis(uint8_t axis, float delta) {
    if (axis < 4 && steppers[axis]) {
        int32_t deltaSteps = (int32_t)(delta * stepsPerUnit[axis]);
        steppers[axis]->move(deltaSteps);
    }
}

void MotionController::stopAll() {
    for (int i = 0; i < 4; i++) {
        if (steppers[i]) {
            steppers[i]->stopMove();
        }
    }
}

void MotionController::emergencyStop() {
    for (int i = 0; i < 4; i++) {
        if (steppers[i]) {
            steppers[i]->forceStop();
        }
    }
}

void MotionController::setCurrentPositionAsZero() {
    for (int i = 0; i < 4; i++) {
        if (steppers[i]) {
            steppers[i]->setCurrentPosition(0);
        }
    }
}

void MotionController::update() {
    // Check limit switches
    for (int i = 0; i < 4; i++) {
        // Read limit pin
        int val = digitalRead(limitPins[i]);
        bool triggered = (val == HIGH); // or LOW based on limit switch wiring
        if (triggered && steppers[i] && steppers[i]->isRunning()) {
            // Optional: halt if limit reached in motion direction
        }
    }
}

AxisMotionState MotionController::getAxisState(uint8_t axis) const {
    AxisMotionState state;
    if (axis < 4 && steppers[axis]) {
        state.currentPos = (float)steppers[axis]->getCurrentPosition() / stepsPerUnit[axis];
        state.targetPos = (float)steppers[axis]->targetPos() / stepsPerUnit[axis];
        state.isMoving = steppers[axis]->isRunning();
        state.limitHit = (digitalRead(limitPins[axis]) == HIGH);
    }
    return state;
}

bool MotionController::isAnyMoving() const {
    for (int i = 0; i < 4; i++) {
        if (steppers[i] && steppers[i]->isRunning()) return true;
    }
    return false;
}

