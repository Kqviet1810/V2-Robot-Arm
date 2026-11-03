#include "rgb_status.h"

RgbStatus::RgbStatus() 
    : currentState(C3Status::BOOTING),
      lastUpdate(0),
      calibStartTime(0),
      isCalibrating(false),
      step(0) {}

void RgbStatus::begin() {
    setColor(0, 0, 0); // Off initially
}

void RgbStatus::setColor(uint8_t r, uint8_t g, uint8_t b) {
    rgbLedWrite(RGB_LED_PIN, r, g, b);
}

void RgbStatus::setState(C3Status newState) {
    if (currentState != newState && !isCalibrating) {
        currentState = newState;
    }
}

void RgbStatus::triggerCalibBlink() {
    isCalibrating = true;
    calibStartTime = millis();
}

void RgbStatus::update() {
    unsigned long now = millis();

    // Priority: Calibration Blink
    if (isCalibrating) {
        if (now - calibStartTime > 800) { // 800ms calibration indication
            isCalibrating = false;
        } else {
            // Blink White fast
            if ((now / 100) % 2 == 0) {
                setColor(120, 120, 120);
            } else {
                setColor(0, 0, 0);
            }
            return;
        }
    }

    if (now - lastUpdate < 30) return; // ~33Hz animation update
    lastUpdate = now;
    step++;

    switch (currentState) {
        case C3Status::BOOTING:
            // Pulsing Cyan / Yellow
            {
                uint8_t val = (uint8_t)(30 + 30 * sin(step * 0.1));
                setColor(val, val, 0); // Yellow pulse
            }
            break;

        case C3Status::TCA_ERROR:
            // Fast Blinking Red (200ms period)
            if ((now / 200) % 2 == 0) {
                setColor(180, 0, 0);
            } else {
                setColor(0, 0, 0);
            }
            break;

        case C3Status::SENSOR_WARN:
            // Blinking Orange / Yellow (magnet issue on 1 or more axes)
            if ((now / 350) % 2 == 0) {
                setColor(200, 60, 0); // Orange
            } else {
                setColor(0, 0, 0);
            }
            break;

        case C3Status::NORMAL_OK:
            // Smooth Green Breathing (heartbeat)
            {
                float angle = step * 0.08f;
                int brightness = (int)(25 + 45 * sin(angle));
                if (brightness < 0) brightness = 0;
                setColor(0, (uint8_t)brightness, 0);
            }
            break;

        default:
            setColor(0, 0, 0);
            break;
    }
}

