/**
 * @file scpi_commands.cpp
 * @author Max Heinekamp
 * @brief This file contains the function definitions for the SCPI commands that are available for the instrument.
 * @details The functions are called when a SCPI command is received. They handle the command and send a response back to the client.
 * @version 0.1
 * @date 29-04-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "flash.h"
#include "scpi.h"

extern FLASH flash_memory;
extern SCPI* scpi_instrument;
extern STEPPER* stepper1;
extern STEPPER* stepper2;

void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  static char response[100]; // Buffer for formatted output
  snprintf(response, sizeof(response), "%s, %s, V%s, #%s", DEVICE_MANUFACTURER, DEVICE_NAME, DEVICE_VERSION, DEVICE_SERIAL);
  interface.print(response);
  interface.write('\n');
}

void Reset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  esp_restart();
}

void OPC(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  interface.print("1");
  interface.write('\n');
}

void GetError(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  const char* response =  scpi_instrument->getError();
  interface.print(response);
  interface.write('\n');
}

void GetHelp(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  scpi_instrument->printAllHeaders(interface);
}

void GetHelpSyntax(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  scpi_instrument->printHeaderSyntax(interface, parameters[0]);
}

void GetVersion(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  static char response[10]; // Buffer for formatted output
  snprintf(response, sizeof(response), "V%s", DEVICE_VERSION);
  interface.print(response);
  interface.write('\n');
}

void GetDebug(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  scpi_instrument->PrintDebugInfo(interface);
  interface.write('\n');
}

void Preset(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  // Reset the device to factory settings
  flash_memory.preferences.clear();
  esp_restart();
}

void SetPort(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //Parse string from input
  String portString = String(parameters[0]);
  int port = portString.toInt();

  //Check if the port is valid
  if (port < 0 || port > 65535) {
    scpi_instrument->logError(220, "Invalid port number");
    return;
  }
  //Set the port number
  flash_memory.preferences.putInt("SCPI-PORT", port);
}

void GetPort(SCPI_C commands, SCPI_P parameters, Stream& interface) {
  //Retrieve the port number from flash memory
  int port = scpi_instrument->getPort();
  interface.print(port);
  interface.write('\n');
}

void SetIP(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String ipString = String(parameters[0]);

  //Check if the string is a valid IP address
  IPAddress ip;
  if (!ip.fromString(ipString)) {
    scpi_instrument->logError(220, "Invalid IP address");
    return;
  }
  //Set the IP address
  flash_memory.preferences.putString("IP-ADDRESS", ipString);

}

void GetIP(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  IPAddress myAddress = Ethernet.localIP();
  interface.print(myAddress);
  interface.write('\n');
}

void SetMAC(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String macString = String(parameters[0]);
  byte myAddress[6];
  sscanf(macString.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &myAddress[0], &myAddress[1], &myAddress[2], &myAddress[3], &myAddress[4], &myAddress[5]);
  
  //Set the MAC address
  flash_memory.preferences.putBytes("MAC-ADDRESS", myAddress, sizeof(myAddress));
}

void GetMAC(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  byte myAddress[6];
  Ethernet.MACAddress(myAddress);
  for (byte octet = 0; octet < 6; octet++) {
    interface.print(myAddress[octet], HEX);
    if (octet < 5) {
      interface.print(':');
    }
  }
  interface.write('\n');
}

void GetGW(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  IPAddress myGateWay = Ethernet.gatewayIP();
  interface.print(myGateWay);
  interface.write('\n');
}

void Perpetual(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  while (true) {
    // Perform perpetual motion until device is reset or powered off
    stepper1->step();
    stepper2->step();
    delay(1); // Adjust delay as needed to control speed
  }
}

void Step(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Get the numeric suffix/index (if any) from the commands
  String header = String(commands.Last());
  header.toUpperCase();
  interface.print(header);
  interface.write('\n');
  int suffix = -1;
  sscanf(header.c_str(),"%*[STEP]%u", &suffix);
  
  //Check if the suffix is valid
  if (suffix != 1 && suffix != 2) {
    scpi_instrument->logError(131, "Invalid Suffix");
    return;
  }

  if (suffix == 1) {
    for (int i = 0; i < 3200; i++) {
      stepper1->step();
    }
    
    interface.print(F("STEP1"));
    interface.write('\n');
  }

  if (suffix == 2) {
    for (int i = 0; i < 19200; i++) {
      stepper2->step();
    }
    
    //stepper2->move(1000, 5000, 360, (STEPS_PER_REV * AXIS2_GEAR_RATIO * MICROSTEPS),stepper1, 6); 
    //stepper2.runStepperSCurve(2000, 500, 9600);

    interface.print(F("STEP2"));
    interface.write('\n');
  }
}

void SetRelativePosition(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String positionString = String(parameters[0]);
  int position = positionString.toInt();
  
  //Check if the position is valid
  if (position < 0 || position > 360) {
    scpi_instrument->logError(220, "Invalid position value");
    return;
  }

  //Get the numeric suffix/index (if any) from the commands
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[RELATIVE]%u", &suffix);
  
  //Check if the suffix is valid
  if (suffix != 1 && suffix != 2) {
    char errorMessage[30];
    sprintf(errorMessage, "Invalid Suffix: %d", suffix);
    scpi_instrument->logError(131, errorMessage);
    return;
  }

  //Get speed and acceleration from flash memory
  int speed = flash_memory.preferences.getInt("SPEED", 1500);
  int acceleration = flash_memory.preferences.getInt("ACCELERATION", 500);

  if (suffix == 1) {
    stepper1->move(speed, acceleration, position, (STEPS_PER_REV * AXIS1_GEAR_RATIO * MICROSTEPS));
  }

  if (suffix == 2) {
    stepper2->move(speed, acceleration, position, (STEPS_PER_REV * AXIS2_GEAR_RATIO * MICROSTEPS), stepper1, 6);
  }
}

void GetAbsolutePosition(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Get the numeric suffix/index (if any) from the commands
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[ABSOLUTE]%u", &suffix);
  
  //Check if the suffix is valid
  if (suffix != 1 && suffix != 2) {
    scpi_instrument->logError(131, "Invalid Suffix");
    // A query must always produce a response, or the client is left waiting indefinitely.
    interface.print(F("NaN"));
    interface.write('\n');
    return;
  }

  if (suffix == 1) {
    float position = stepper1->getCurrentPosition();
    
    interface.print(position);
    interface.write('\n');
  }

  if (suffix == 2) {
    float position = stepper2->getCurrentPosition();
    
    interface.print(position);
    interface.write('\n');
  }
}

void SetAbsolutePosition(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String positionString = String(parameters[0]);
  int position = positionString.toInt();
  
  //Check if the position is valid
  if (position < 0 || position > 360) {
    scpi_instrument->logError(220, "Invalid position value");
    return;
  }

  //Get the numeric suffix/index (if any) from the commands
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[ABSOLUTE]%u", &suffix);
  
  //Check if the suffix is valid
  if (suffix != 1 && suffix != 2) {
    char errorMessage[30];
    sprintf(errorMessage, "Invalid Suffix: %d", suffix);
    scpi_instrument->logError(131, errorMessage);
    return;
  }

  int speed = flash_memory.preferences.getInt("SPEED", 1500);
  int acceleration = flash_memory.preferences.getInt("ACCELERATION", 500);

  if (suffix == 1) {
    stepper1->absoluteMove(speed, acceleration, position, (STEPS_PER_REV * AXIS1_GEAR_RATIO * MICROSTEPS));
  }

  if (suffix == 2) {
    stepper2->absoluteMove(speed, acceleration, position, (STEPS_PER_REV * AXIS2_GEAR_RATIO * MICROSTEPS), stepper1, 6);
  }
}

void SetDirection(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Get the numeric suffix/index (if any) from the commands
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DIRECTION]%u", &suffix);
  
  //Check if the suffix is valid
  if (suffix != 1 && suffix != 2) {
    char errorMessage[30];
    sprintf(errorMessage, "Invalid Suffix: %d", suffix);
    scpi_instrument->logError(131, errorMessage);
    return;
  }

  if(suffix == 1) {
    if (String(parameters[0]) == "CW") {
      stepper1->SetDirection(false);
    }
    else if (String(parameters[0]) == "CCW") {
      stepper1->SetDirection(true);
    } 
    else {
      char errorMessage[30];
      sprintf(errorMessage, "Illegal parameter value: %s", String(parameters[0]));
      scpi_instrument->logError(224, errorMessage);
    }
  }
  if(suffix == 2) {
    String param = String(parameters[0]);
    if (param == "CW" || param == "CCW") {
      bool newDir = (param == "CCW");   // matches STEPPER::SetDirection convention: true=CCW, false=CW
      bool oldDir = stepper2->GetDirection();
      stepper2->SetDirection(newDir);

      if (newDir != oldDir) {
        // Drive the motor a little bit to adjust for play in the gears — only on an actual reversal
        float cal_factor = flash_memory.preferences.getFloat("AXIS2-DIR-CHANGE-CAL-FACTOR", AXIS2_DIR_CHANGE_CAL_FACTOR);
        int cal_steps = (int)(cal_factor * STEPS_PER_REV * AXIS2_GEAR_RATIO * MICROSTEPS / 360.0);
        for (int i = 0; i < cal_steps; i++) {
            stepper2->step();
            delayMicroseconds(200);
        }
      }
    }
    else {
      char errorMessage[30];
      sprintf(errorMessage, "Illegal parameter value: %s", String(parameters[0]));
      scpi_instrument->logError(224, errorMessage);
    }
  }
}

void GetDirection(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Get the numeric suffix/index (if any) from the commands
  String header = String(commands.Last());
  header.toUpperCase();
  int suffix = -1;
  sscanf(header.c_str(),"%*[DIRECTION]%u", &suffix);
  
  //Check if the suffix is valid
  if (suffix != 1 && suffix != 2) {
    char errorMessage[30];
    sprintf(errorMessage, "Invalid Suffix: %d", suffix);
    scpi_instrument->logError(131, errorMessage);
    // A query must always produce a response, or the client is left waiting indefinitely.
    interface.print(F("ERR"));
    interface.write('\n');
    return;
  }

  if (suffix == 1) {
    if (stepper1->GetDirection()) {
    interface.print(F("CCW"));
    interface.write('\n');
  } 
    else {
      interface.print(F("CW"));
      interface.write('\n');
    }
  }

  if (suffix == 2) {
    if (stepper2->GetDirection()) {
      interface.print(F("CCW"));
      interface.write('\n');
    } 
    else {
      interface.print(F("CW"));
      interface.write('\n');
    }
  }
}

void SetRMSCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String rmsString = String(parameters[0]);
  int rms = rmsString.toInt();

  //Check if the RMS current is valid
  if (rms < 0 || rms > 2400) {
    scpi_instrument->logError(220, "Invalid RMS current value");
    return;
  }
  //Set the RMS current
  flash_memory.preferences.putInt("RMS-CURRENT", rms);

  //Update the stepper driver with the new RMS current
  stepper1->updateRMSCurrent();
  stepper2->updateRMSCurrent();
}

void GetRMSCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Report the actual current applied by the driver (both axes are always driven identically)
  interface.print(stepper1->getRMSCurrent());
  interface.write('\n');
}

void SetHoldCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String holdString = String(parameters[0]);
  int hold = holdString.toInt();

  //Check if the hold current is valid
  if (hold < 0 || hold > 32) {
    scpi_instrument->logError(220, "Invalid hold current value");
    return;
  }
  //Set the hold current
  flash_memory.preferences.putInt("HOLD-CURRENT", hold);

  //Update the stepper driver with the new hold current
  stepper1->updateHoldCurrent();
  stepper2->updateHoldCurrent();
}

void GetHoldCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Report the actual hold current applied by the driver (both axes are always driven identically)
  interface.print(stepper1->getHoldCurrent());
  interface.write('\n');
}

void SetMicrosteps(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String microstepsString = String(parameters[0]);
  int microsteps = microstepsString.toInt();

  //Check if the microsteps value is valid
  if (microsteps < 1 || microsteps > 256) {
    scpi_instrument->logError(220, "Invalid microsteps value");
    return;
  }
  //Set the microsteps value
  flash_memory.preferences.putInt("MICROSTEPS", microsteps);

  //Update the stepper driver with the new microsteps value
  stepper1->updateMicrosteps();
  stepper2->updateMicrosteps();
}

void GetMicrosteps(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Retrieve the microsteps value from flash memory
  int microsteps = flash_memory.preferences.getInt("MICROSTEPS", MICROSTEPS);
  interface.print(microsteps);
  interface.write('\n');
}

void SetSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String speedString = String(parameters[0]);
  int speed = speedString.toInt();

  //Check if the speed value is valid
  if (speed < 0 || speed > 100000) {
    scpi_instrument->logError(220, "Invalid speed value");
    return;
  }
  //Set the speed value
  flash_memory.preferences.putInt("SPEED", speed);
}

void GetSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Retrieve the speed value from flash memory
  int speed = flash_memory.preferences.getInt("SPEED", SPEED);
  interface.print(speed);
  interface.write('\n');
}

void SetAcceleration(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Parse string from input
  String accelerationString = String(parameters[0]);
  int acceleration = accelerationString.toInt();

  //Check if the acceleration value is valid
  if (acceleration < 0 || acceleration > 10000) {
    scpi_instrument->logError(220, "Invalid acceleration value");
    return;
  }
  //Set the acceleration value
  flash_memory.preferences.putInt("ACCELERATION", acceleration);
}

void GetAcceleration(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  //Retrieve the acceleration value from flash memory
  int acceleration = flash_memory.preferences.getInt("ACCELERATION", ACCELERATION);
  interface.print(acceleration);
  interface.write('\n');
}

void DebugMotorPins(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  int step_pin1 = flash_memory.preferences.getInt("MOTOR1-STEP-PIN", MOTOR1_STEP_PIN);
  int enable_pin1 = flash_memory.preferences.getInt("MOTOR1-ENABLE-PIN", MOTOR1_ENABLE_PIN);
  int step_pin2 = flash_memory.preferences.getInt("MOTOR2-STEP-PIN", MOTOR2_STEP_PIN);
  int enable_pin2 = flash_memory.preferences.getInt("MOTOR2-ENABLE-PIN", MOTOR2_ENABLE_PIN);
  interface.print("Motor 1 Step Pin: "); interface.print(step_pin1); interface.write('\n');
  interface.print("Motor 1 Enable Pin: "); interface.print(enable_pin1); interface.write('\n');
  interface.print("Motor 2 Step Pin: "); interface.print(step_pin2); interface.write('\n');
  interface.print("Motor 2 Enable Pin: "); interface.print(enable_pin2); interface.write('\n');
}

void GetMotorDebug(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  MOTOR_SERIAL_PORT.flush();  // Clear any pending data
  delay(20);
  
  int status1 = stepper1->Debug();
  delay(20);  // Add delay between reads
  
  int status2 = stepper2->Debug();
  
  interface.print("Motor 1 Status: "); interface.print(status1, BIN); interface.write('\n');
  interface.print("Motor 2 Status: "); interface.print(status2, BIN); interface.write('\n');
}

void ResetEthernet(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  int ethernet_reset_pin = flash_memory.preferences.getInt("ETHERNET-RESET-PIN", ETHERNET_RESET_PIN);

  //Reset ethernet module
  digitalWrite(ethernet_reset_pin, LOW);
  delay(100);
  digitalWrite(ethernet_reset_pin, HIGH);
  delay(500);

  //Re-initialize the Ethernet stack and drop any stale client sessions
  scpi_instrument->reinitEthernet();

  interface.print(ethernet_reset_pin);
  interface.print(" Ethernet module reset");
  interface.write('\n');
}

void GetSessionList(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  scpi_instrument->printSessionList(interface);
}

void GetSessionLog(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  scpi_instrument->printSessionLog(interface);
}

void GetSocketList(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  scpi_instrument->printSocketList(interface);
}

void GetDiagSystem(SCPI_C commands, SCPI_P parameters, Stream &interface) {
  scpi_instrument->printSystemDiagnostics(interface);
}