#pragma once
#include <Arduino.h>

// ==========================================
// MAIN MCU (ESP32 DEV MODULE) PIN & SYSTEM CONFIG
// ==========================================

// --- SHARED STEPPER ENABLE/DISABLE PIN ---
#define PIN_SHARED_DISABLE      13  // LOW = Enabled, HIGH = Disabled

// --- TMC2209 HARDWARE UART (UART1) ---
#define PIN_TMC_UART_TX         23
#define PIN_TMC_UART_RX         22
#define TMC_UART_BAUD           115200
#define R_SENSE                 0.11f  // Standard sense resistor on most TMC2209

// --- STEPPER AXES CONFIGURATION ---
// AXIS X (Base)
#define PIN_X_STEP              14
#define PIN_X_DIR               27
#define PIN_X_LIMIT_NEG         35
#define TMC_ADDR_X              0
#define X_STEPS_PER_UNIT        40.0f
#define X_MAX_SPEED             5000.0f
#define X_ACCEL                 1000.0f
#define X_MAX_TRAVEL            45.0f

// AXIS Y (Shoulder)
#define PIN_Y_STEP              26
#define PIN_Y_DIR               25
#define PIN_Y_LIMIT_NEG         34
#define TMC_ADDR_Y              1
#define Y_STEPS_PER_UNIT        40.0f
#define Y_MAX_SPEED             5000.0f
#define Y_ACCEL                 1000.0f
#define Y_MAX_TRAVEL            45.0f

// AXIS Z (Elbow)
#define PIN_Z_STEP              21
#define PIN_Z_DIR               19
#define PIN_Z_LIMIT_NEG         36
#define TMC_ADDR_Z              2
#define Z_STEPS_PER_UNIT        40.0f
#define Z_MAX_SPEED             5000.0f
#define Z_ACCEL                 1000.0f
#define Z_MAX_TRAVEL            360.0f

// AXIS A (Wrist)
#define PIN_A_STEP              18
#define PIN_A_DIR               4
#define PIN_A_LIMIT_POS         39
#define TMC_ADDR_A              3
#define A_STEPS_PER_UNIT        80.0f
#define A_MAX_SPEED             5000.0f
#define A_ACCEL                 1000.0f
#define A_MAX_TRAVEL            380.0f

// Stepper Current & Microsteps
#define TMC_MICROSTEPS          16
#define TMC_RUN_CURRENT_MA      1000
#define TMC_HOLD_CURRENT_MA     200
#define TMC_STALLGUARD_VAL      30

// --- RC SERVO CONFIGURATION ---
// Axis B: Gripper Rotate (Xoay kẹp)
#define PIN_SERVO_ROTATE        32
// Axis C: Gripper Open/Close (Kẹp)
#define PIN_SERVO_GRIPPER       33
#define SERVO_PWM_FREQ          50
#define SERVO_MIN_US            1000
#define SERVO_MAX_US            2000

// --- SENSOR SUB-MCU (ESP32-C3) UART (UART2) ---
#define PIN_C3_UART_TX          17  // ESP32 TX -> C3 RX
#define PIN_C3_UART_RX          16  // ESP32 RX <- C3 TX
#define C3_UART_BAUD            115200

// --- WIFI & NETWORK CONFIG ---
#define WIFI_AP_SSID            "RobotArm_V2_AP"
#define WIFI_AP_PASS            "12345678"
#define WEB_SERVER_PORT         80
#define WS_SERVER_PORT          81

