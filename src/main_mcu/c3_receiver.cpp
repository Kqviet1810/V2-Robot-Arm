#include "c3_receiver.h"

C3Receiver::C3Receiver() : c3Serial(2) {}

void C3Receiver::begin() {
    c3Serial.begin(C3_UART_BAUD, SERIAL_8N1, PIN_C3_UART_RX, PIN_C3_UART_TX);
    rxBuffer.reserve(128);
}

void C3Receiver::parsePacket(const String& line) {
    // Format: $ARM,x,y,z,a,status
    if (!line.startsWith("$ARM,")) return;

    int commaIdx[5];
    int start = 5;
    for (int i = 0; i < 5; i++) {
        commaIdx[i] = line.indexOf(',', start);
        if (commaIdx[i] == -1 && i < 4) return; // Malformed
        start = commaIdx[i] + 1;
    }

    float x = line.substring(5, commaIdx[0]).toFloat();
    float y = line.substring(commaIdx[0] + 1, commaIdx[1]).toFloat();
    float z = line.substring(commaIdx[1] + 1, commaIdx[2]).toFloat();
    float a = line.substring(commaIdx[2] + 1, commaIdx[3]).toFloat();
    uint8_t st = line.substring(commaIdx[3] + 1).toInt();

    state.actualAngle[0] = x;
    state.actualAngle[1] = y;
    state.actualAngle[2] = z;
    state.actualAngle[3] = a;
    state.rawStatus = st;
    for (int i = 0; i < 4; i++) {
        state.sensorHealthy[i] = (st & (1 << i)) != 0;
    }
    state.lastUpdateTime = millis();
    state.isConnected = true;
}

void C3Receiver::update() {
    while (c3Serial.available()) {
        char c = (char)c3Serial.read();
        if (c == '\n') {
            rxBuffer.trim();
            if (rxBuffer.length() > 0) {
                parsePacket(rxBuffer);
            }
            rxBuffer = "";
        } else if (c != '\r') {
            if (rxBuffer.length() < 120) {
                rxBuffer += c;
            } else {
                rxBuffer = ""; // buffer overflow protection
            }
        }
    }

    // Timeout check (no data for 500ms -> disconnected)
    if (millis() - state.lastUpdateTime > 500) {
        state.isConnected = false;
    }
}

void C3Receiver::sendCalibrateCommand(uint8_t axis) {
    c3Serial.printf("$CALIB,%u\n", axis);
}

