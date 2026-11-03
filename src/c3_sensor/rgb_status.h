#pragma once
#include <Arduino.h>
#include <esp32-hal-rgb-led.h>
#include "c3_config.h"

enum class C3Status {
    BOOTING,        // Khởi động (Màu vàng / Cyan)
    TCA_ERROR,      // Lỗi I2C Mux TCA9548A (Nhấp nháy Đỏ nhanh)
    SENSOR_WARN,    // Lỗi nam châm AS5600 ở 1 hoặc nhiều trục (Màu Cam / Magenta nháy)
    CALIBRATING,    // Đang cân chỉnh Zero (Trắng nháy nhanh)
    NORMAL_OK       // Hoạt động tốt 100% (Xanh lá cây nhịp tim / breathing)
};

class RgbStatus {
public:
    RgbStatus();
    void begin();
    void setState(C3Status newState);
    void update();
    void triggerCalibBlink();

    void setColor(uint8_t r, uint8_t g, uint8_t b);

private:
    C3Status currentState;
    unsigned long lastUpdate;
    unsigned long calibStartTime;
    bool isCalibrating;
    uint8_t step;
};

