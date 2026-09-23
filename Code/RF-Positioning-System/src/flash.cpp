/**
 * @file flash.cpp
 * @author Max Heinekamp
 * @brief This file contains the function definitions for the FLASH class, which handles the flash memory of the ESP32.
 * @details The FLASH class uses the Preferences library to access the flash memory of the ESP32. It initializes the flash memory with default values if it has not been initialized before.
 * @version 0.1
 * @date 29-04-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "flash.h"

FLASH::FLASH() : preferences() {
}

void FLASH::initFlashMemory() {
    preferences.begin("config", false);
    if(!preferences.getBool("initialized", false)) {
        preferences.putBool("initialized", true);

        // Set default values for SCPI settings
        preferences.putInt("SCPI-PORT", SCPI_PORT);
        preferences.putString("IP-ADDRESS", IP_ADDRESS);
        const byte mac[] = MAC_ADDRESS;
        preferences.putBytes("MAC-ADDRESS", mac, 6);

        // Set default values for Ethernet settings
        preferences.putInt("ETHERNET-RESET-PIN", ETHERNET_RESET_PIN);

        // Set default values for stepper settings
        preferences.putInt("STEPS-PER-REV", STEPS_PER_REV);
        preferences.putInt("MOTOR1-STEP-PIN", MOTOR1_STEP_PIN);
        preferences.putInt("MOTOR1-ENABLE-PIN", MOTOR1_ENABLE_PIN);
        preferences.putInt("MOTOR2-STEP-PIN", MOTOR2_STEP_PIN);
        preferences.putInt("MOTOR2-ENABLE-PIN", MOTOR2_ENABLE_PIN);
        preferences.putFloat("R-SENSE", R_SENSE);
        preferences.putInt("STEP-DELAY", STEP_DELAY);
        preferences.putInt("RMS-CURRENT", RMS_CURRENT);
        preferences.putInt("HOLD-CURRENT", HOLD_CURRENT);
        preferences.putInt("MICROSTEPS", MICROSTEPS);
        preferences.putInt("STEPPER1-ADDRESS", STEPPER1_ADDRESS);
        preferences.putInt("STEPPER2-ADDRESS", STEPPER2_ADDRESS);
        preferences.putInt("AXIS1-GEAR-RATIO", AXIS1_GEAR_RATIO);
        preferences.putInt("AXIS2-GEAR-RATIO", AXIS2_GEAR_RATIO);
        preferences.putFloat("AXIS2-DIR-CHANGE-CAL-FACTOR", AXIS2_DIR_CHANGE_CAL_FACTOR);
        preferences.putInt("SPEED", SPEED);
        preferences.putInt("ACCELERATION", ACCELERATION);
    }
}