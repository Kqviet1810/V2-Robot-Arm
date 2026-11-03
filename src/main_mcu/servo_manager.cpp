#include "servo_manager.h"

ServoManager::ServoManager() : currentRotateAngle(90), currentGripperAngle(90) {}

void ServoManager::begin() {
    // Allocate standard ESP32 PWM timers for servos
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    servoRotate.setPeriodHertz(SERVO_PWM_FREQ);
    servoGripper.setPeriodHertz(SERVO_PWM_FREQ);

    servoRotate.attach(PIN_SERVO_ROTATE, SERVO_MIN_US, SERVO_MAX_US);
    servoGripper.attach(PIN_SERVO_GRIPPER, SERVO_MIN_US, SERVO_MAX_US);

    // Initial safe positions
    setRotateAngle(90);
    setGripperAngle(90);
}

void ServoManager::setRotateAngle(int angleDeg) {
    currentRotateAngle = constrain(angleDeg, 0, 180);
    servoRotate.write(currentRotateAngle);
}

void ServoManager::setGripperAngle(int angleDeg) {
    currentGripperAngle = constrain(angleDeg, 0, 180);
    servoGripper.write(currentGripperAngle);
}

void ServoManager::openGripper() {
    setGripperAngle(160);
}

void ServoManager::closeGripper() {
    setGripperAngle(30);
}

