#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

class ServoManager {
public:
    ServoManager();
    void begin();
    
    void setRotateAngle(int angleDeg);
    void setGripperAngle(int angleDeg);
    
    void openGripper();
    void closeGripper();

    int getRotateAngle() const { return currentRotateAngle; }
    int getGripperAngle() const { return currentGripperAngle; }

private:
    Servo servoRotate;
    Servo servoGripper;
    int currentRotateAngle;
    int currentGripperAngle;
};

