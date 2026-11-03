#pragma once
#include <Arduino.h>

// ==========================================
// ESP32-C3 SUPER MINI HARDWARE CONFIGURATION
// (Khớp chuẩn 100% theo sơ đồ nguyên lý U6 ESP32C3-SUPERMINI)
// ==========================================

// I2C Pins for TCA9548A & AS5600
// Theo schematic: Pin 9 (GPIO 0) -> SCL | Pin 10 (GPIO 1) -> SDA
#define I2C_SDA_PIN         1   // GPIO 1 -> SDA
#define I2C_SCL_PIN         0   // GPIO 0 -> SCL
#define I2C_FREQ_HZ         400000  // Fast mode 400kHz

// Buzzer pin (Pin 4: GPIO 8)
#define BUZZER_PIN          8

// RGB LED (WS2812) Pin (Pin 11: GPIO 2 theo sơ đồ)
#define RGB_LED_PIN         2
#define NUM_LEDS            4   // 4 đèn LED ở 4 góc bo mạch

// TCA9548A I2C Multiplexer Address
#define TCA9548A_ADDR       0x70

// Channel mapping on TCA9548A for 4 Stepper Axes
#define TCA_CH_AXIS_X       0  // Base (Axis X)
#define TCA_CH_AXIS_Y       1  // Shoulder (Axis Y)
#define TCA_CH_AXIS_Z       2  // Elbow (Axis Z)
#define TCA_CH_AXIS_A       3  // Wrist (Axis A)
#define NUM_AXES            4

// UART Communication with Main MCU (ESP32 Dev Module)
// Sơ đồ mạch:
// - Main MCU TX2 (GPIO 17 - net TX_C3) nối sang Pin 8 (GPIO 21) của C3 -> C3 RX = 21
// - Main MCU RX2 (GPIO 16 - net RX_C3) nối sang Pin 7 (GPIO 20) của C3 -> C3 TX = 20
#define UART_TO_MAIN_RX     21  // GPIO 21 nhận từ Main MCU TX2 (GPIO 17)
#define UART_TO_MAIN_TX     20  // GPIO 20 gửi sang Main MCU RX2 (GPIO 16)
#define UART_BAUDRATE       115200

// Sampling & Telemetry interval (ms)
#define REPORT_INTERVAL_MS  20  // 50Hz update rate
