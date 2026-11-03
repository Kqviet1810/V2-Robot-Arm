#include <Arduino.h>
#include "config.h"
#include "tmc_driver.h"
#include "motion_controller.h"
#include "servo_manager.h"
#include "c3_receiver.h"
#include "web_interface.h"

// Core subsystems
TmcManager tmc;
MotionController motion;
ServoManager servos;
C3Receiver c3Receiver;
WebInterface web(motion, servos, tmc, c3Receiver);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==============================================");
    Serial.println("🦾 ESP32 ROBOT ARM V2 - MAIN CONTROLLER");
    Serial.println("==============================================");

    // 1. Initialize TMC2209 Drivers via UART1
    Serial.println("[MAIN] Initializing TMC2209 drivers...");
    tmc.begin();

    // 2. Initialize Servos (Gripper + Rotate)
    Serial.println("[MAIN] Initializing RC Servos...");
    servos.begin();

    // 3. Initialize Motion Controller (FastAccelStepper RMT)
    Serial.println("[MAIN] Initializing Motion Engine...");
    motion.begin();

    // 4. Initialize C3 UART receiver (AS5600 Feedback)
    Serial.println("[MAIN] Initializing C3 Sensor Interface (UART2)...");
    c3Receiver.begin();

    // 5. Initialize WiFi & Web Dashboard
    Serial.println("[MAIN] Initializing Web Interface & WebSocket...");
    web.begin();

    Serial.println("[MAIN] System Ready! Connect to WiFi 'RobotArm_V2_AP' (pass 12345678)");
    Serial.println("[MAIN] Open browser at: http://192.168.4.1\n");
}

void loop() {
    // 1. Process UART feedback from ESP32-C3
    c3Receiver.update();

    // 2. Poll motion limits and safety checks
    motion.update();

    // 3. Handle WebServer & WebSocket clients
    web.update();
}

