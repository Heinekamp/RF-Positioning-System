# RF-Positioning System

The positioning system uses 2 stepper motors controlled by an ESP32 via SCPI over Ethernet to rotate the DUT in two axes.

## SCPI Command Reference

| Command                        | Description                                                                 |
|--------------------------------|-----------------------------------------------------------------------------|
| `*IDN?`                        | Queries the identification string of the device.                           |
| `*RST`                         | Resets the device to its default state.                                    |
| `*OPC?`                        | Returns 1 once all previously queued commands have completed.              |
| `SYSTem:ERRor?`                | Retrieves the last error from the error queue.                             |
| `SYSTem:HELP:HEADers?`         | Provides a list of available SCPI commands.                                |
| `SYSTem:HELP:SYNTax?`          | Returns detailed syntax help for a single command header.                  |
| `SYSTem:VERSion?`              | Queries the firmware version of the device.                                |
| `SYSTem:DEBUG?`                | Retrieves debug information from the system.                               |
| `SYSTem:PRESet`                | Resets the system to its preset configuration.                             |
| `SYSTem:COMMunicate:ETHernet:PORT` | Sets the Ethernet communication port.                                   |
| `SYSTem:COMMunicate:ETHernet:PORT?`| Queries the Ethernet communication port.                                |
| `SYSTem:COMMunicate:ETHernet:ADDRess` | Sets the IP address of the device.                                   |
| `SYSTem:COMMunicate:ETHernet:ADDRess?`| Queries the IP address of the device.                                |
| `SYSTem:COMMunicate:ETHernet:MAC` | Sets the MAC address of the device.                                      |
| `SYSTem:COMMunicate:ETHernet:MAC?`| Queries the MAC address of the device.                                   |
| `SYSTem:COMMunicate:ETHernet:DGATeway?`| Queries the default gateway address.                               |
| `SYSTem:COMMunicate:ETHernet:RESET`| Resets the W5500 Ethernet module and re-initializes the network stack. |
| `CONTRol:ROTation:STEP#`       | Jogs the specified axis one full revolution, for testing.                  |
| `CONTRol:ROTation:PERPetual#`  | Runs the specified axis continuously, for testing.                         |
| `CONTRol:ROTation:DIRection#`  | Sets the rotation direction for the DUT.                                   |
| `CONTRol:ROTation:DIRection#?` | Queries the current rotation direction of the DUT.                        |
| `CONTRol:ROTation:RELative#`   | Sets the relative position of the DUT.                                     |
| `CONTRol:ROTation:ABSolute#`   | Sets the absolute position of the DUT.                                     |
| `CONTRol:ROTation:ABSolute#?`  | Queries the absolute position of the DUT.                                  |
| `CONFigure:MOTor:RMScurrent`   | Sets the RMS current for the stepper motors.                               |
| `CONFigure:MOTor:RMScurrent?`  | Queries the RMS current for the stepper motors.                            |
| `CONFigure:MOTor:HOLDcurrent`  | Sets the hold current for the stepper motors.                              |
| `CONFigure:MOTor:HOLDcurrent?` | Queries the hold current for the stepper motors.                           |
| `CONFigure:MOTor:MICROsteps`   | Sets the microstepping configuration for the stepper motors.               |
| `CONFigure:MOTor:MICROsteps?`  | Queries the microstepping configuration for the stepper motors.            |
| `CONFigure:MOTor:SPEED`        | Sets the motion speed used by subsequent moves.                            |
| `CONFigure:MOTor:SPEED?`       | Queries the configured motion speed.                                       |
| `CONFigure:MOTor:ACCELeration` | Sets the motion acceleration used by subsequent moves.                     |
| `CONFigure:MOTor:ACCELeration?`| Queries the configured motion acceleration.                                |
| `CONFigure:MOTor:STATus?`      | Returns raw TMC2209 driver status registers for both axes.                 |
| `DIAGnostic:SESSion:LIST?`     | Lists live SCPI sessions over Ethernet.                                    |
| `DIAGnostic:SESSion:LOG?`      | Lists recent SCPI session closures.                                        |
| `DIAGnostic:SOCKet:LIST?`      | Lists the raw state of all W5500 hardware sockets.                         |
| `DIAGnostic:SYSTem?`           | Returns system and reset health diagnostics.                               |

For the full behavior, parameter ranges, and side effects of each command, see the [Service Manual](../../Documentation/manual/ServiceManual.pdf).

## Documentation
The full firmware documentation is available [here](https://heinekamp.github.io/RF-Positioning-System/index.html).