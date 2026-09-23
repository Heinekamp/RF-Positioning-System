/**
 * @file config.h
 * @author Max Heinekamp
 * @brief Configuration file for the entire project.
 * @note This information will be copied to the flash memory of the ESP32 on first boot by FLASH::initFlashMemory()
 * @version 0.1
 * @date 26-01-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "Ethernet.h"

// SCPI parser command-hash width. The Vrekrer SCPI parser's default hash type (uint8_t) has
// only 256 possible values; with ~35 registered commands spanning a multi-level tree, hash
// collisions are a real risk. Confirmed: under uint8_t, "CONFigure:MOTor" itself hashes to
// exactly 0, which the parser's own code treats as its "no tree base set" sentinel, silently
// breaking every command registered under that entire branch (RMScurrent, HOLDcurrent,
// MICROsteps, SPEED, ACCELeration and their queries). Must be defined before
// Vrekrer_scpi_parser.h is first included anywhere in the project (this file is included
// ahead of the parser everywhere it's used).
#define SCPI_HASH_TYPE uint16_t

///////////////////////////////////////////////////////////////////////////////
// DEVICE INFORMATION                                                       //
/////////////////////////////////////////////////////////////////////////////

#define DEVICE_MANUFACTURER "EMES"
#define DEVICE_NAME "RF-Positioning-System"
#define DEVICE_VERSION "1.0.0"
#define DEVICE_SERIAL "001"
#define POWER_GOOD_PIN 26 // Output from the eFuse, not currently used

// SCPI server settings
#define SCPI_PORT 5025
#define IP_ADDRESS "192.168.0.55"
#define MAC_ADDRESS { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }
#define ERROR_QUEUE_SIZE 10

// Ethernet module settings
#define ETHERNET_RESET_PIN 25

// Hardware reset control
#define RESET_CONTROL_PIN 4  // GPIO pin to pull EN low for hardware reset

// Stepper motor settings
#define STEPS_PER_REV 200
#define MOTOR1_STEP_PIN 15
#define MOTOR1_ENABLE_PIN 32
#define MOTOR2_STEP_PIN 13
#define MOTOR2_ENABLE_PIN 12
#define MOTOR_SERIAL_PORT Serial1
#define R_SENSE 0.11f
#define STEP_DELAY 4 // Was 160
#define RMS_CURRENT 1600 // in mA
#define HOLD_CURRENT 8 // as a fraction /32 of RMS_CURRENT
#define MICROSTEPS 16 // 1/16th microstepping
#define STEPPER1_ADDRESS 0
#define STEPPER2_ADDRESS 1
#define AXIS1_GEAR_RATIO 1
#define AXIS2_GEAR_RATIO 6
#define AXIS2_DIR_CHANGE_CAL_FACTOR 3 // degrees to drive for backlash compensation
#define SPEED 1000 // in steps/s
#define ACCELERATION 5000 // in steps/s^2


#endif // CONFIG_H