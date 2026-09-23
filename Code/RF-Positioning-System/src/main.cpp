/**
 * @file main.cpp
 * @author Max Heinekamp
 * @brief main file for the project.
 * @version 0.1
 * @date 27-01-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

// Include required libraries
#include <Arduino.h>
#include "config.h"
#include "Vrekrer_scpi_parser.h"
#include "scpi.h"
#include "flash.h"
#include "esp_system.h"

//Sored in RTC memory to persist across soft resets, but not power cycles. Used to determine if the system has just booted or if it has been reset after boot.
RTC_DATA_ATTR bool hasResetAfterBoot = false;

//Sored in RTC memory to persist across soft resets, but not power cycles. Counts boots since the
//device was last fully power-cycled, so repeated involuntary resets (e.g. brownouts, watchdog
//triggers) during otherwise-normal operation are visible via DIAGnostic:SYStem?.
RTC_DATA_ATTR uint32_t bootCount = 0;


//Create Flash object
FLASH flash_memory;

// Cretate global pointers for SCPI and Stepper objects
SCPI* scpi_instrument = nullptr;
STEPPER* stepper1 = nullptr;
STEPPER* stepper2 = nullptr;

const char* resetReasonToString(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_UNKNOWN: return "UNKNOWN";
    case ESP_RST_POWERON: return "POWERON";
    case ESP_RST_EXT: return "EXT";
    case ESP_RST_SW: return "SW";
    case ESP_RST_PANIC: return "PANIC";
    case ESP_RST_INT_WDT: return "INT_WDT";
    case ESP_RST_TASK_WDT: return "TASK_WDT";
    case ESP_RST_WDT: return "WDT";
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
    case ESP_RST_BROWNOUT: return "BROWNOUT";
    case ESP_RST_SDIO: return "SDIO";
    default: return "UNMAPPED";
  }
}


void setup() {
  bootCount++;

  pinMode(POWER_GOOD_PIN, INPUT); // Not currently used

  flash_memory.initFlashMemory();

  scpi_instrument = new SCPI();

  //Retrieve ethernet settings from flash memory
  int ethernet_reset_pin = flash_memory.preferences.getInt("ETHERNET-RESET-PIN", ETHERNET_RESET_PIN);

  //Wait for power to stabilize and then reset ethernet module
  delay(500);
  pinMode(ethernet_reset_pin, OUTPUT);
  digitalWrite(ethernet_reset_pin, LOW);
  delay(100);
  digitalWrite(ethernet_reset_pin, HIGH);
  delay(500);

  // Retrieve stepper settings from flash memory
  int address1 = flash_memory.preferences.getInt("STEPPER1-ADDRESS", STEPPER1_ADDRESS);
  int address2 = flash_memory.preferences.getInt("STEPPER2-ADDRESS", STEPPER2_ADDRESS);
  int step_pin1 = flash_memory.preferences.getInt("MOTOR1-STEP-PIN", MOTOR1_STEP_PIN);
  int enable_pin1 = flash_memory.preferences.getInt("MOTOR1-ENABLE-PIN", MOTOR1_ENABLE_PIN);
  int step_pin2 = flash_memory.preferences.getInt("MOTOR2-STEP-PIN", MOTOR2_STEP_PIN);
  int enable_pin2 = flash_memory.preferences.getInt("MOTOR2-ENABLE-PIN", MOTOR2_ENABLE_PIN);
  
  // Initialize stepper objects with the retrieved settings
  stepper1 = new STEPPER(address1, step_pin1, enable_pin1);
  stepper2 = new STEPPER(address2, step_pin2, enable_pin2);

  // Start serial communication for USB interface
  Serial.begin(9600);
   // Start serial interface for stepper motor driver
  Serial1.begin(9600);
  //Setup SCPI communication
  scpi_instrument->setup();
  
  //Setup Stepper motor
  stepper1->setup();
  stepper2->setup();

  esp_reset_reason_t resetReason = esp_reset_reason();
  Serial.println("Test123");
  delay(500);
  Serial.println("Test1234");
  delay(500);
  Serial.printf("Reset reason: %d\n", esp_reset_reason());

  if ((resetReason == ESP_RST_POWERON || resetReason == ESP_RST_BROWNOUT || resetReason == ESP_RST_UNKNOWN || resetReason == ESP_RST_EXT) && hasResetAfterBoot == false) {
    Serial.println("Cold/unstable boot detected, scheduling hardware reset...");
    Serial.flush();  // Ensure message is sent before reset

    hasResetAfterBoot = true;
    
    delay(5000);  // wait for power to stabilize
    
    esp_restart();
    
    // // Perform hardware reset by pulling EN pin low via GPIO
    // pinMode(RESET_CONTROL_PIN, OUTPUT);
    // digitalWrite(RESET_CONTROL_PIN, LOW);
    // delay(500);  // Hold low long enough for reset
    // digitalWrite(RESET_CONTROL_PIN, HIGH);  // Release (safety, though we're gone by now)
    
  }
}
  
void loop() {
  //Check Serial for incoming SCPI commands 
  scpi_instrument->processSerial();
  //Check Ethernet for incoming SCPI commands
  scpi_instrument->processEthernet();
  
}

/*
import pyvisa
rm = pyvisa.ResourceManager('@py')
EthInstrument = rm.open_resource('TCPIP0::192.168.0.13::5025::SOCKET')
EthInstrument.write_termination = '\n'
EthInstrument.read_termination = '\n'
EthInstrument.query("*IDN?") 
*/