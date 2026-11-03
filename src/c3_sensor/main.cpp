#include <Arduino.h>
#include "c3_config.h"
#include "tca9548a.h"
#include "as5600_manager.h"
#include "rgb_status.h"

// Serial port to Main MCU (UART2 on Main MCU)
HardwareSerial MainMcuSerial(1);

AS5600Manager sensorManager;
RgbStatus rgb;
bool tcaHealthy = false;
unsigned long lastReportTime = 0;

void handleIncomingCommands() {
    while (MainMcuSerial.available()) {
        String cmd = MainMcuSerial.readStringUntil('\n');
        cmd.trim();
        if (cmd.startsWith("$CALIB")) {
            // $CALIB,0 -> Calibrate axis X
            int axis = cmd.substring(7).toInt();
            sensorManager.zeroCalibration(axis);
            rgb.triggerCalibBlink();
            MainMcuSerial.printf("$OK,CALIB,%d\n", axis);
        } else if (cmd.equals("$PING")) {
            MainMcuSerial.println("$PONG");
        }
    }
}

void setup() {
    // Debug Serial (USB CDC or UART0)
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[ESP32-C3] Robot Arm V2 - AS5600 Multiplexer Starting...");

    // Setup RGB LED & Buzzer
    rgb.begin();
    rgb.setState(C3Status::BOOTING);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // Communication UART to Main ESP32
    MainMcuSerial.begin(UART_BAUDRATE, SERIAL_8N1, UART_TO_MAIN_RX, UART_TO_MAIN_TX);

    // Init I2C & AS5600
    tcaHealthy = sensorManager.begin();
    if (tcaHealthy) {
        Serial.println("[ESP32-C3] TCA9548A detected successfully!");
        rgb.setState(C3Status::NORMAL_OK);
        // Beep 2 short beeps for success
        digitalWrite(BUZZER_PIN, HIGH); delay(60); digitalWrite(BUZZER_PIN, LOW); delay(60);
        digitalWrite(BUZZER_PIN, HIGH); delay(60); digitalWrite(BUZZER_PIN, LOW);
    } else {
        Serial.println("[ESP32-C3] [ERROR] TCA9548A not responding at 0x70! Check wiring.");
        rgb.setState(C3Status::TCA_ERROR);
        // Beep 1 long beep for warning
        digitalWrite(BUZZER_PIN, HIGH); delay(400); digitalWrite(BUZZER_PIN, LOW);
    }
}

void loop() {
    handleIncomingCommands();
    rgb.update();

    unsigned long now = millis();
    if (now - lastReportTime >= REPORT_INTERVAL_MS) {
        lastReportTime = now;

        if (!tcaHealthy) {
            rgb.setState(C3Status::TCA_ERROR);
            return;
        }

        sensorManager.updateAll();

        const auto& x = sensorManager.getAxisData(TCA_CH_AXIS_X);
        const auto& y = sensorManager.getAxisData(TCA_CH_AXIS_Y);
        const auto& z = sensorManager.getAxisData(TCA_CH_AXIS_Z);
        const auto& a = sensorManager.getAxisData(TCA_CH_AXIS_A);

        uint8_t statusByte = (x.healthy ? 1 : 0) |
                             ((y.healthy ? 1 : 0) << 1) |
                             ((z.healthy ? 1 : 0) << 2) |
                             ((a.healthy ? 1 : 0) << 3);

        // Update RGB LED Status based on AS5600 sensor health:
        // statusByte == 0x0F (15) means all 4 magnets detected properly
        if (statusByte == 0x0F) {
            rgb.setState(C3Status::NORMAL_OK);
        } else {
            rgb.setState(C3Status::SENSOR_WARN);
        }

        // Packet format: $ARM,<degX>,<degY>,<degZ>,<degA>,<status>\n
        char packet[64];
        snprintf(packet, sizeof(packet), "$ARM,%.2f,%.2f,%.2f,%.2f,%u\n",
                 x.degree, y.degree, z.degree, a.degree, statusByte);

        // Send to Main MCU
        MainMcuSerial.print(packet);

        // Also print to USB Serial for debugging
        static uint16_t debugCounter = 0;
        if (++debugCounter % 25 == 0) { // every ~500ms
            Serial.printf("[C3 FEEDBACK] X:%.1f° Y:%.1f° Z:%.1f° A:%.1f° | Status: 0x%02X\n",
                          x.degree, y.degree, z.degree, a.degree, statusByte);
        }
    }
}
