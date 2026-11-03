#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "c3_config.h"

enum class C3Status {
    BOOTING,        // Khởi động (Hiệu ứng đèn xoay 4 góc màu Amber)
    TCA_ERROR,      // Lỗi I2C Mux (4 góc chớp Đỏ cảnh báo)
    SENSOR_WARN,    // Lỗi nam châm từng trục cụ thể (Góc lỗi chớp Đỏ/Cam, góc tốt xanh)
    CALIBRATING,    // Hiệu chuẩn Zero (Quét tia sáng Trắng quanh 4 góc)
    NORMAL_OK       // 4 góc hoàn hảo (Hiệu ứng Neon Cyber Runner / Xoay vòng mượt)
};

class RgbStatus {
public:
    RgbStatus();
    void begin();
    void setState(C3Status newState);
    void setSensorHealth(bool xOk, bool yOk, bool zOk, bool aOk);
    void update();
    void triggerCalibBlink();

    void setAllColor(uint8_t r, uint8_t g, uint8_t b);
    void setCornerColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

private:
    Adafruit_NeoPixel strip;
    C3Status currentState;
    bool axisHealthy[4];
    unsigned long lastUpdate;
    unsigned long calibStartTime;
    bool isCalibrating;
    uint16_t animStep;

    void renderBootAnimation();
    void renderNormalAnimation();
    void renderSensorWarnAnimation();
    void renderTcaErrorAnimation();
    void renderCalibAnimation();
};
