/**
 * @file scpi_commands.h
 * @author Max Heinekamp
 * @brief This file contains the function declarations for the functions whic are called when a SCPI command is received.
 * @version 0.1
 * @date 22-04-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SCPI_COMMANDS_H
#define SCPI_COMMANDS_H

#include "scpi.h"

/**
 * @brief Sends an identification message over the given interface.
 * 
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void Identify(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
 * @brief Triggers a full reset of the ESP32.
 * 
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void Reset(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
 * @brief Sends a 1 over the given interface to indicate that the previous command has finished executing.
 * 
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 * 
 * @details As the programm is run sequentially on a single core, this function works by immediately sending a 1 over the interface, as soon controller gets around to processing the command.
 */
void OPC(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
 * @brief Sends the oldest error message over the given interface.
 * 
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void GetError(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
* @brief Sends a help message with all available SCPI commands over the given interface.
* 
* @param commands The SCPI commands that were received.
* @param parameters The parameters that were passed with the SCPI command.
* @param interface Pointer to the Stream interface where the SCPI command was received.
*/
void GetHelp(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
 * @brief Sends syntax help for a single command header, given as a parameter (e.g.
 * SYSTem:HELP:SYNTax? *IDN?), over the given interface.
 *
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command — parameters[0] is the
 * header to look up, matched against its full spelling as printed by SYSTem:HELP:HEADers?.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void GetHelpSyntax(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
 * @brief Sends the current firmware version of the device over the given interface.
 * 
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 * 
 * @todo This could be exapnded to also include a hardware version, which would be set in the config.h file.
 * @note The firmware version is defined in the config.h file.
 */
void GetVersion(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
 * @brief Sends the debug information from Vrekrer-SCPI-Parser over the given interface.
 * 
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void GetDebug(SCPI_C commands, SCPI_P parameters, Stream& interface);

/**
 * @brief Rewrites the parameters stored in flash memory with the defaults from the config.h file.
 * 
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void Preset(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SetPort(SCPI_C commands, SCPI_P parameters, Stream& interface);
void GetPort(SCPI_C commands, SCPI_P parameters, Stream& interface);
void SetIP(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetIP(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetMAC(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetMAC(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetGW(SCPI_C commands, SCPI_P parameters, Stream &interface);
void Step(SCPI_C commands, SCPI_P parameters, Stream &interface);
void Perpetual(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetRelativePosition(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetAbsolutePosition(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetAbsolutePosition(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetDirection(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetDirection(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetRMSCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetRMSCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetHoldCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetHoldCurrent(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetMicrosteps(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetMicrosteps(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetSpeed(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetAcceleration(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetAcceleration(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetGearRatio1(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetGearRatio1(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetGearRatio2(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetGearRatio2(SCPI_C commands, SCPI_P parameters, Stream &interface);
void SetStepsPerRev(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetStepsPerRev(SCPI_C commands, SCPI_P parameters, Stream &interface);
void DebugMotorPins(SCPI_C commands, SCPI_P parameters, Stream &interface);
void GetMotorDebug(SCPI_C commands, SCPI_P parameters, Stream &interface);
void ResetEthernet(SCPI_C commands, SCPI_P parameters, Stream &interface);

/**
 * @brief Sends the live SCPI Ethernet session table over the given interface.
 *
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void GetSessionList(SCPI_C commands, SCPI_P parameters, Stream &interface);

/**
 * @brief Sends the ring buffer of recent SCPI session closures (with reason) over the given interface.
 *
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void GetSessionLog(SCPI_C commands, SCPI_P parameters, Stream &interface);

/**
 * @brief Sends the raw state of all W5500 hardware sockets over the given interface.
 *
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void GetSocketList(SCPI_C commands, SCPI_P parameters, Stream &interface);

/**
 * @brief Sends system/reset health diagnostics (uptime, heap, reset reason, boot count) over the given interface.
 *
 * @param commands The SCPI commands that were received.
 * @param parameters The parameters that were passed with the SCPI command.
 * @param interface Pointer to the Stream interface where the SCPI command was received.
 */
void GetDiagSystem(SCPI_C commands, SCPI_P parameters, Stream &interface);

#endif // SCPI_COMMANDS_H