#include "rgb_status.h"

RgbStatus::RgbStatus() 
    : strip(NUM_LEDS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800),
      currentState(C3Status::BOOTING),
      lastUpdate(0),
      calibStartTime(0),
      isCalibrating(false),
      animStep(0) 
{
    for (int i = 0; i < 4; i++) axisHealthy[i] = false;
}

void RgbStatus::begin() {
    strip.begin();
    strip.setBrightness(100); // 0 - 255 (Safe & crisp brightness)
    strip.clear();
    strip.show();
}

void RgbStatus::setAllColor(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

void RgbStatus::setCornerColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < NUM_LEDS) {
        strip.setPixelColor(index, strip.Color(r, g, b));
    }
}

void RgbStatus::setState(C3Status newState) {
    if (currentState != newState && !isCalibrating) {
        currentState = newState;
    }
}

void RgbStatus::setSensorHealth(bool xOk, bool yOk, bool zOk, bool aOk) {
    axisHealthy[0] = xOk;
    axisHealthy[1] = yOk;
    axisHealthy[2] = zOk;
    axisHealthy[3] = aOk;
}

void RgbStatus::triggerCalibBlink() {
    isCalibrating = true;
    calibStartTime = millis();
}

void RgbStatus::renderBootAnimation() {
    // 4-corner Amber spinner with tail
    uint8_t activeCorner = (animStep / 3) % 4;
    for (uint8_t i = 0; i < 4; i++) {
        if (i == activeCorner) {
            setCornerColor(i, 220, 100, 0); // Head (Bright Amber)
        } else if (i == (activeCorner + 3) % 4) {
            setCornerColor(i, 60, 25, 0);  // Tail
        } else {
            setCornerColor(i, 5, 2, 0);    // Background dim
        }
    }
    strip.show();
}

void RgbStatus::renderNormalAnimation() {
    // Cyber Neon Runner around the 4 corners:
    // Base color: Cyber Neon Cyan (0, 30, 60)
    // Running pulse: Emerald Green (0, 240, 120) sweeping through corners
    uint8_t head = (animStep / 4) % 4;
    float subPhase = (animStep % 4) / 4.0f;

    for (uint8_t i = 0; i < 4; i++) {
        if (i == head) {
            // Bright leading comet
            uint8_t g = (uint8_t)(120 + 130 * (1.0f - subPhase));
            uint8_t b = (uint8_t)(40 + 80 * subPhase);
            setCornerColor(i, 0, g, b);
        } else if (i == (head + 3) % 4) {
            // Smooth trailing corner
            setCornerColor(i, 0, 70, 80);
        } else {
            // Idle ambient corners (Breathing cyan)
            float breath = sin((animStep + i * 2) * 0.1f);
            uint8_t dimG = (uint8_t)(15 + 15 * breath);
            uint8_t dimB = (uint8_t)(35 + 25 * breath);
            setCornerColor(i, 0, dimG, dimB);
        }
    }
    strip.show();
}

void RgbStatus::renderSensorWarnAnimation() {
    // For each of the 4 corners (Axis X, Y, Z, A):
    // - Healthy axis: Solid calm Cyan/Green
    // - Faulty axis (lost magnet): Flashing RED alert on that specific corner!
    bool blinkOn = (animStep / 6) % 2 == 0;

    for (uint8_t i = 0; i < 4; i++) {
        if (axisHealthy[i]) {
            setCornerColor(i, 0, 80, 40); // Healthy: Calm Emerald Green
        } else {
            if (blinkOn) {
                setCornerColor(i, 255, 10, 0); // Faulty: Alert RED
            } else {
                setCornerColor(i, 30, 0, 0);  // Dim dark red
            }
        }
    }
    strip.show();
}

void RgbStatus::renderTcaErrorAnimation() {
    // All 4 corners fast strobe RED
    bool flash = (animStep / 3) % 2 == 0;
    if (flash) {
        setAllColor(255, 0, 0);
    } else {
        setAllColor(0, 0, 0);
    }
}

void RgbStatus::renderCalibAnimation() {
    // High-speed white spinner around 4 corners
    uint8_t active = (animStep / 2) % 4;
    for (uint8_t i = 0; i < 4; i++) {
        if (i == active) {
            setCornerColor(i, 255, 255, 255);
        } else {
            setCornerColor(i, 20, 20, 20);
        }
    }
    strip.show();
}

void RgbStatus::update() {
    unsigned long now = millis();

    // Priority: Calibration sweep (lasts 1 second)
    if (isCalibrating) {
        if (now - calibStartTime > 1000) {
            isCalibrating = false;
        } else {
            if (now - lastUpdate >= 30) {
                lastUpdate = now;
                animStep++;
                renderCalibAnimation();
            }
            return;
        }
    }

    if (now - lastUpdate < 30) return; // ~33 FPS smooth animation
    lastUpdate = now;
    animStep++;

    switch (currentState) {
        case C3Status::BOOTING:
            renderBootAnimation();
            break;

        case C3Status::TCA_ERROR:
            renderTcaErrorAnimation();
            break;

        case C3Status::SENSOR_WARN:
            renderSensorWarnAnimation();
            break;

        case C3Status::NORMAL_OK:
            renderNormalAnimation();
            break;

        default:
            strip.clear();
            strip.show();
            break;
    }
}
