#pragma once
#include <Arduino.h>

// ==========================================
// ESP32-C3 SUPER MINI HARDWARE CONFIGURATION
// ==========================================

// I2C Pins for TCA9548A & AS5600
// Note: ESP32-C3 Super Mini standard I2C is SDA=8, SCL=9 (or SDA=4, SCL=5 depending on PCB)
#define I2C_SDA_PIN         8
#define I2C_SCL_PIN         9
#define I2C_FREQ_HZ         400000  // Fast mode 400kHz

// TCA9548A I2C Multiplexer Address
#define TCA9548A_ADDR       0x70

// Channel mapping on TCA9548A for 4 Stepper Axes
#define TCA_CH_AXIS_X       0  // Base (Axis X)
#define TCA_CH_AXIS_Y       1  // Shoulder (Axis Y)
#define TCA_CH_AXIS_Z       2  // Elbow (Axis Z)
#define TCA_CH_AXIS_A       3  // Wrist (Axis A)
#define NUM_AXES            4

// UART Communication with Main MCU (ESP32 Dev Module)
// Main MCU: UART2 (TX=17, RX=16)
// ESP32-C3: RX=20, TX=21 (Default USB/JTAG can also use Serial for debug)
#define UART_TO_MAIN_RX     20
#define UART_TO_MAIN_TX     21
#define UART_BAUDRATE       115200

// Sampling & Telemetry interval (ms)
#define REPORT_INTERVAL_MS  20  // 50Hz update rate

