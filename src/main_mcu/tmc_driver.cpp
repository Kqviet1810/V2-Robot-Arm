#include "tmc_driver.h"

TmcManager::TmcManager() 
    : tmcSerial(1),
      driverX(&tmcSerial, R_SENSE, TMC_ADDR_X),
      driverY(&tmcSerial, R_SENSE, TMC_ADDR_Y),
      driverZ(&tmcSerial, R_SENSE, TMC_ADDR_Z),
      driverA(&tmcSerial, R_SENSE, TMC_ADDR_A) 
{
    drivers[0] = &driverX;
    drivers[1] = &driverY;
    drivers[2] = &driverZ;
    drivers[3] = &driverA;
}

void TmcManager::begin() {
    // Setup Shared Stepper Disable Pin (Active LOW: LOW = Enabled, HIGH = Disabled)
    pinMode(PIN_SHARED_DISABLE, OUTPUT);
    enableSteppers(false); // Start safely disabled

    // Initialize UART1 for TMC2209 communication
    tmcSerial.begin(TMC_UART_BAUD, SERIAL_8N1, PIN_TMC_UART_RX, PIN_TMC_UART_TX);

    for (int i = 0; i < 4; i++) {
        drivers[i]->begin();
        drivers[i]->toff(4);
        drivers[i]->blank_time(24);
        drivers[i]->rms_current(TMC_RUN_CURRENT_MA, (float)TMC_HOLD_CURRENT_MA / (float)TMC_RUN_CURRENT_MA);
        drivers[i]->microsteps(TMC_MICROSTEPS);
        drivers[i]->en_spreadCycle(false); // StealthChop enabled for silent & smooth operation
        drivers[i]->pwm_autoscale(true);
        drivers[i]->TCOOLTHRS(0xFFFFF);    // 20bit max
        drivers[i]->SGTHRS(TMC_STALLGUARD_VAL); // Stallguard threshold
    }

    enableSteppers(true);
}

void TmcManager::enableSteppers(bool enable) {
    digitalWrite(PIN_SHARED_DISABLE, enable ? LOW : HIGH);
}

bool TmcManager::checkDrivers() {
    bool allOk = true;
    for (int i = 0; i < 4; i++) {
        uint8_t version = drivers[i]->version();
        if (version == 0 || version == 0xFF) {
            allOk = false;
        }
    }
    return allOk;
}

void TmcManager::setStallguardThreshold(uint8_t threshold) {
    for (int i = 0; i < 4; i++) {
        drivers[i]->SGTHRS(threshold);
    }
}

TMC2209Stepper* TmcManager::getDriver(uint8_t index) {
    if (index >= 4) return nullptr;
    return drivers[index];
}

