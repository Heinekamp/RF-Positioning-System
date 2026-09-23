/**
 * @file scpi.cpp
 * @author Max Heinekamp
 * @brief Contains function definitions related to SCPI communication.
 * @version 0.1
 * @date 27-01-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "scpi.h"
#include "scpi_commands.h"
#include "flash.h"
#include "esp_system.h"
#include <utility/w5100.h>  // SnSR:: raw TCP socket-status constants, for diagnostics

int brightness = 0;
extern FLASH flash_memory;

// Defined in main.cpp — reused here so DIAGnostic:SYStem? doesn't duplicate the reset-reason
// mapping, and so a repeated involuntary reboot mid-session is visible across soft resets.
extern const char* resetReasonToString(esp_reset_reason_t reason);
extern RTC_DATA_ATTR uint32_t bootCount;

static const char* socketStatusName(uint8_t status) {
  switch (status) {
    case SnSR::CLOSED:      return "CLOSED";
    case SnSR::LISTEN:      return "LISTEN";
    case SnSR::SYNSENT:     return "SYNSENT";
    case SnSR::SYNRECV:     return "SYNRECV";
    case SnSR::ESTABLISHED: return "ESTABLISHED";
    case SnSR::FIN_WAIT:    return "FIN_WAIT";
    case SnSR::CLOSING:     return "CLOSING";
    case SnSR::TIME_WAIT:   return "TIME_WAIT";
    case SnSR::CLOSE_WAIT:  return "CLOSE_WAIT";
    case SnSR::LAST_ACK:    return "LAST_ACK";
    case SnSR::UDP:         return "UDP";
    default:                return "UNKNOWN";
  }
}

SCPI::SCPI() {
}

// Single source of truth for every registered SCPI command: full absolute path, handler, and a
// one-line description used by SYSTem:HELP:HEADers?/SYSTem:HELP:SYNTax?. Each command is
// registered by its complete path rather than via SetCommandTreeBase — this makes registration
// hash exactly the way a fully-typed user command hashes at runtime, with no tree-base shortcut
// left to accidentally collide (this project has hit that bug class twice: CONFigure:MOTor
// hashing to the parser's "no tree set" sentinel, and a SYStem/SYSTem capitalization collision).
struct CommandInfo {
  const char* path;        // full absolute SCPI path, e.g. "CONTRol:ROTation:DIRection#?"
  SCPI_caller_t handler;
  const char* description; // shown by SYSTem:HELP:SYNTax?
};

static const CommandInfo kCommands[] = {
  {"*IDN?", &Identify,
   "Returns device identification string.\n"
   "Returns: \"<manufacturer>, <name>, V<version>, #<serial>\", comma-separated."},

  {"*RST", &Reset,
   "Performs a full device reset (reboot).\n"
   "Parameter: none.\n"
   "Affects: restarts the ESP32 immediately; stored settings in flash are unchanged. Does not "
   "send a response — the connection drops as the device reboots."},

  {"*OPC?", &OPC,
   "Signals that all previously queued commands have completed.\n"
   "Returns: \"1\", always, immediately.\n"
   "Note: this device processes commands synchronously on a single thread, so reaching this "
   "command already implies prior commands finished; it does not track any asynchronous operation."},

  {"SYSTem:ERRor?", &GetError,
   "Returns and dequeues the oldest error from the error queue.\n"
   "Returns: \"<code>, \\\"<message>\\\"\", or \"0, \\\"No error\\\"\" if the queue is empty.\n"
   "Note: the queue holds up to 10 entries; if it overflows, newer errors are discarded and a "
   "queue-overflow error is reported once."},

  {"SYSTem:HELP:HEADers?", &GetHelp,
   "Lists all available SCPI command headers.\n"
   "Parameter: none.\n"
   "Returns: \"#HEADERS <n>\" followed by one header per line."},

  {"SYSTem:HELP:SYNTax?", &GetHelpSyntax,
   "Returns detailed syntax help for a single command header.\n"
   "Parameter: string, the exact header as printed by SYSTem:HELP:HEADers? (e.g. \"*IDN?\"). "
   "Case-insensitive; surrounding quotes are stripped if present.\n"
   "Returns: a multi-line description of that command, or \"UNKNOWN\" plus a queued error if the "
   "header isn't recognized."},

  {"SYSTem:VERSion?", &GetVersion,
   "Returns the device firmware version.\n"
   "Returns: \"V<version>\", e.g. \"V1.0.0\"."},

  {"SYSTem:DEBUG?", &GetDebug,
   "Returns internal SCPI parser debug info.\n"
   "Returns: registered token/command counts and message buffer usage, for firmware development use."},

  {"SYSTem:PRESet", &Preset,
   "Performs a factory reset.\n"
   "Parameter: none.\n"
   "Affects: clears ALL stored preferences in flash (IP, MAC, port, motor current, speed, "
   "acceleration, calibration, etc. — everything reverts to config.h defaults) and reboots immediately."},

  {"SYSTem:COMMunicate:ETHernet:PORT", &SetPort,
   "Sets the SCPI TCP listen port.\n"
   "Parameter: integer, 0-65535.\n"
   "Affects: stored to flash immediately; takes effect on the next reboot, not the current session."},

  {"SYSTem:COMMunicate:ETHernet:PORT?", &GetPort,
   "Returns the configured SCPI TCP listen port.\n"
   "Returns: integer, the port currently in use (as loaded at boot)."},

  {"SYSTem:COMMunicate:ETHernet:ADDRess", &SetIP,
   "Sets the device's static IP address.\n"
   "Parameter: string, dotted-quad IPv4 format, e.g. \"192.168.0.55\".\n"
   "Affects: stored to flash; takes effect on the next Ethernet re-init "
   "(SYSTem:COMMunicate:ETHernet:RESET) or reboot."},

  {"SYSTem:COMMunicate:ETHernet:ADDRess?", &GetIP,
   "Returns the device's current live IP address.\n"
   "Returns: dotted-quad IPv4 string, as currently assigned to the Ethernet interface."},

  {"SYSTem:COMMunicate:ETHernet:MAC", &SetMAC,
   "Sets the device's MAC address.\n"
   "Parameter: string, colon-separated hex octets, e.g. \"DE:AD:BE:EF:FE:ED\".\n"
   "Affects: stored to flash; takes effect on the next Ethernet re-init or reboot."},

  {"SYSTem:COMMunicate:ETHernet:MAC?", &GetMAC,
   "Returns the device's current MAC address.\n"
   "Returns: colon-separated hex octets, e.g. \"DE:AD:BE:EF:FE:ED\"."},

  {"SYSTem:COMMunicate:ETHernet:DGATeway?", &GetGW,
   "Returns the configured default gateway IP address.\n"
   "Returns: dotted-quad IPv4 string."},

  {"SYSTem:COMMunicate:ETHernet:RESET", &ResetEthernet,
   "Resets the W5500 Ethernet module and re-initializes the network stack.\n"
   "Parameter: none.\n"
   "Affects: toggles the module's hardware reset pin, re-reads IP/MAC from flash, restarts the "
   "SCPI listener, and drops all currently tracked SCPI sessions (including the one issuing this command)."},

  {"CONTRol:ROTation:STEP#", &Step,
   "Jogs axis <n> exactly one full mechanical revolution at fixed speed, with no acceleration ramp.\n"
   "Parameter: <n> is 1 or 2 (axis 1 or 2); no other parameters.\n"
   "Affects: physically moves the axis; does not update the axis's tracked absolute position. "
   "Test/development use only, not intended for normal operation."},

  {"CONTRol:ROTation:DIRection#", &SetDirection,
   "Sets rotation direction of axis <n>.\n"
   "Parameter: <n> is 1 or 2. Value: \"CW\" or \"CCW\".\n"
   "Affects: axis <n>'s direction for subsequent moves. On axis 2, an actual direction reversal "
   "also triggers an automatic backlash-compensation move before the next command; repeating the "
   "same direction is a no-op."},

  {"CONTRol:ROTation:DIRection#?", &GetDirection,
   "Queries the current rotation direction of axis <n>.\n"
   "Parameter: <n> is 1 or 2.\n"
   "Returns: \"CW\" or \"CCW\"."},

  {"CONTRol:ROTation:RELative#", &SetRelativePosition,
   "Moves axis <n> by a relative angle.\n"
   "Parameter: <n> is 1 or 2. Value: integer degrees, 0-360.\n"
   "Affects: physically moves the axis using the currently configured SPEED and ACCELeration."},

  {"CONTRol:ROTation:ABSolute#", &SetAbsolutePosition,
   "Moves axis <n> to an absolute angle.\n"
   "Parameter: <n> is 1 or 2. Value: integer degrees, 0-360.\n"
   "Affects: physically moves the axis to the target angle using the currently configured SPEED "
   "and ACCELeration; updates the axis's tracked absolute position."},

  {"CONTRol:ROTation:ABSolute#?", &GetAbsolutePosition,
   "Queries the current absolute position of axis <n>.\n"
   "Parameter: <n> is 1 or 2.\n"
   "Returns: float, degrees — the device's internally tracked position (dead-reckoned from "
   "commanded steps, not measured by any sensor)."},

  {"CONTRol:ROTation:PERPetual#", &Perpetual,
   "Runs axis <n> continuously at fixed speed.\n"
   "Parameter: <n> is 1 or 2; no other parameters.\n"
   "Affects: physically moves the axis indefinitely. Does not return a response and blocks all "
   "further command processing (Serial and Ethernet) until the device is reset or powered off. "
   "Test/development use only, not intended for normal operation."},

  {"CONFigure:MOTor:RMScurrent", &SetRMSCurrent,
   "Sets the stepper driver RMS run current.\n"
   "Parameter: integer, 0-2400 (mA).\n"
   "Affects: both axes simultaneously — there is no per-axis current setting."},

  {"CONFigure:MOTor:RMScurrent?", &GetRMSCurrent,
   "Queries the actual RMS run current applied by the driver.\n"
   "Returns: integer, mA — read back from the driver hardware, not just the last requested "
   "value, so it reflects the driver's real current-scale quantization."},

  {"CONFigure:MOTor:HOLDcurrent", &SetHoldCurrent,
   "Sets the stepper driver hold current.\n"
   "Parameter: integer, 0-32 — a raw IHOLD driver scale code (not mA), roughly a fraction /32 of "
   "the run current.\n"
   "Affects: both axes simultaneously."},

  {"CONFigure:MOTor:HOLDcurrent?", &GetHoldCurrent,
   "Queries the actual hold current applied by the driver.\n"
   "Returns: integer, 0-32 — the raw IHOLD driver scale code currently applied."},

  {"CONFigure:MOTor:MICROsteps", &SetMicrosteps,
   "Sets the stepper driver microstepping resolution.\n"
   "Parameter: integer, 1-256 (typically a power of two, e.g. 16).\n"
   "Affects: both axes simultaneously."},

  {"CONFigure:MOTor:MICROsteps?", &GetMicrosteps,
   "Queries the configured microstepping resolution.\n"
   "Returns: integer, microsteps per full step."},

  {"CONFigure:MOTor:SPEED", &SetSpeed,
   "Sets the target motion speed used by subsequent moves.\n"
   "Parameter: integer, 0-100000 (steps/s).\n"
   "Affects: both axes; applied to the next RELative#/ABSolute# move issued after this command, "
   "not to any move already in progress."},

  {"CONFigure:MOTor:SPEED?", &GetSpeed,
   "Queries the configured target motion speed.\n"
   "Returns: integer, steps/s."},

  {"CONFigure:MOTor:ACCELeration", &SetAcceleration,
   "Sets the motion acceleration used by subsequent moves.\n"
   "Parameter: integer, 0-10000 (steps/s^2).\n"
   "Affects: both axes; applied to the next RELative#/ABSolute# move issued after this command."},

  {"CONFigure:MOTor:ACCELeration?", &GetAcceleration,
   "Queries the configured motion acceleration.\n"
   "Returns: integer, steps/s^2."},

  // Named "STATus?" rather than "Debug?" deliberately: "Debug" (only the first letter
  // capitalized) is case-insensitively identical to the unrelated "SYSTem:DEBUG?" token (which is
  // fully capitalized, so its short and long forms are both "DEBUG"). The parser's long-form
  // matching is case-insensitive but its token table is case-sensitive, so this command's short
  // form ("MOT:D?") would silently resolve through SYSTem:DEBUG?'s token instead of its own,
  // never actually matching what got registered — confirmed via the hash simulation script (same
  // bug class as the earlier SYStem/SYSTem collision). "STATus?" is also the more conventional
  // SCPI name for a raw status query.
  {"CONFigure:MOTor:STATus?", &GetMotorDebug,
   "Returns raw TMC2209 driver status registers for both axes.\n"
   "Returns: two lines, \"Motor 1 Status: <binary>\" and \"Motor 2 Status: <binary>\" — raw "
   "DRV_STATUS-style bits, for diagnostic use."},

  {"DIAGnostic:SESSion:LIST?", &GetSessionList,
   "Lists live SCPI Ethernet sessions.\n"
   "Returns: \"#SESSIONS <n>\" then one CSV line per active slot — index, classification "
   "(SCPI/PING), TCP state, remote IP, remote port, local port, age_ms, idle_ms, rx bytes, tx bytes."},

  {"DIAGnostic:SESSion:LOG?", &GetSessionLog,
   "Lists recent SCPI session closures.\n"
   "Returns: \"#CLOSES <n>\" then one CSV line per closure (oldest first, up to the last 16) — "
   "time since closed, remote IP:port, reason code (EVICTED_BY_NEW/CLIENT_FIN/RST/OTHER), session "
   "duration, idle time at close, rx/tx bytes."},

  {"DIAGnostic:SOCKet:LIST?", &GetSocketList,
   "Lists the raw state of all 8 W5500 hardware sockets.\n"
   "Returns: \"#SOCKETS 8\" then one CSV line per socket — index, TCP state, and whether it's "
   "currently tracked as an SCPI session (SCPI-TRACKED) or not (UNTRACKED)."},

  {"DIAGnostic:SYSTem?", &GetDiagSystem,
   "Returns system and reset health diagnostics.\n"
   "Returns: comma-separated key=value pairs — uptime_s, free_heap, min_free_heap, reset_reason, "
   "boot_count, link_flaps, eth_reinits."},
};
static const int kCommandCount = sizeof(kCommands) / sizeof(kCommands[0]);

// Registers all the SCPI commands that are available for the instrument and assigns them to
// the corresponding functions. (Functions are located in scpi_commands.h/cpp)

void SCPI::registerCommands() {
  for (int i = 0; i < kCommandCount; i++) {
    my_instrument.RegisterCommand(kCommands[i].path, kCommands[i].handler);
  }
}

void SCPI::setup() {
  //Retrieve the IP address from flash memory
  IPAddress ip;
  String ipString = flash_memory.preferences.getString("IP-ADDRESS", IP_ADDRESS);
  ip.fromString(ipString);

  // Retrieve the MAC address from flash memory
  byte mac[6];
  flash_memory.preferences.getBytes("MAC-ADDRESS", mac, sizeof(mac));

  // Retrieve the port number from flash memory
  port = flash_memory.preferences.getInt("SCPI-PORT", SCPI_PORT);

  // Dynamically initialize scpi_server
  if (scpi_server == nullptr) {
      scpi_server = new EthernetServer(port);
  }
  
  registerCommands();
  Ethernet.init(33);
  Ethernet.begin((uint8_t*)mac, ip);
  scpi_server->begin();

}

void SCPI::processEthernet() {
  EthernetLinkStatus linkStatus = Ethernet.linkStatus();
  if (lastLinkStatus == LinkON && linkStatus != LinkON) {
    linkFlapCount++;
  }
  lastLinkStatus = linkStatus;

  if (linkStatus == LinkON) {
    if (!server_started) {
      scpi_server->begin();
      server_started = true;
    }

    // Always drain the listener every loop, so a probing/second connection is either adopted
    // into a free session slot or immediately rejected — never left stranding a hardware socket.
    //
    // Note: sessions are NOT matched/evicted by remote IP. Behind a router/VPN concentrator that
    // NATs all client traffic to one apparent source IP, every new connection (a real reconnect or
    // a health-check probe) would otherwise match and evict the active session by IP alone. A new
    // TCP connection always gets a fresh ephemeral source port regardless of source IP, so an
    // IP-based (or even IP+port-based) match against an already-tracked session essentially never
    // legitimately occurs — stale sessions are instead reclaimed below once .connected() goes false.
    //
    // IMPORTANT: must use accept(), not available(). EthernetServer::available() scans every
    // socket ever bound to this port — including ones already accepted and tracked below — and
    // returns any of them that currently has unread data, with no regard for whether we already
    // own it. That silently re-hands back an already-active session as if it were a brand-new
    // connection any time it has pending bytes, corrupting the slot table (confirmed via field
    // data: see docs/code-review-scpi-ethernet-sessions.md). accept() marks a socket as claimed
    // (EthernetServer.cpp: "server_port[i] = 0; // only return the client once") so it is never
    // handed back again — the correct way to detect only genuinely new connections.
    EthernetClient newClient = scpi_server->accept();
    if (newClient) {
      int freeSlot = -1;
      for (int i = 0; i < MAX_SCPI_CLIENTS; i++) {
        if (!scpi_clients[i].active) {
          freeSlot = i;
          break;
        }
      }

      if (freeSlot == -1) {
        // At capacity — evict the oldest session to make room for the new connection.
        freeSlot = 0;
        for (int i = 1; i < MAX_SCPI_CLIENTS; i++) {
          if (scpi_clients[i].connectedAt < scpi_clients[freeSlot].connectedAt) {
            freeSlot = i;
          }
        }
        logSessionClose(scpi_clients[freeSlot], "EVICTED_BY_NEW");
        scpi_clients[freeSlot].client.stop();
      }

      scpi_clients[freeSlot].client = newClient;
      scpi_clients[freeSlot].active = true;
      scpi_clients[freeSlot].connectedAt = millis();
      scpi_clients[freeSlot].lastActivity = millis();
      scpi_clients[freeSlot].rxBytes = 0;
      scpi_clients[freeSlot].txBytes = 0;
      scpi_clients[freeSlot].remoteIP = newClient.remoteIP();
      scpi_clients[freeSlot].remotePort = newClient.remotePort();
    }

    // Service every active session.
    for (int i = 0; i < MAX_SCPI_CLIENTS; i++) {
      if (scpi_clients[i].active) {
        if (scpi_clients[i].client.connected()) {
          //! IMPORTANT: MUST SET CORRECT TERMINATION CHARACTERS
          SessionStream stream(scpi_clients[i]);
          my_instrument.ProcessInput(stream, "\r\n");
        } else {
          // Capture the raw socket state before .stop() forces it to CLOSED, so a clean FIN
          // (CLOSE_WAIT) can be told apart from an abrupt RST (straight to CLOSED).
          uint8_t stat = scpi_clients[i].client.status();
          const char* reason = (stat == SnSR::CLOSE_WAIT) ? "CLIENT_FIN"
                              : (stat == SnSR::CLOSED)     ? "RST"
                              : "OTHER";
          logSessionClose(scpi_clients[i], reason);
          scpi_clients[i].client.stop();
          scpi_clients[i].active = false;
        }
      }
    }
  }
}

void SCPI::logSessionClose(const SCPIClient& slot, const char* reason) {
  SessionCloseLog& entry = sessionLog[sessionLogHead];
  entry.closedAt = millis();
  entry.remoteIP = slot.remoteIP;
  entry.remotePort = slot.remotePort;
  strncpy(entry.reason, reason, sizeof(entry.reason) - 1);
  entry.reason[sizeof(entry.reason) - 1] = '\0';
  entry.durationMs = millis() - slot.connectedAt;
  entry.idleMsAtClose = millis() - slot.lastActivity;
  entry.rxBytes = slot.rxBytes;
  entry.txBytes = slot.txBytes;
  sessionLogHead = (sessionLogHead + 1) % SESSION_LOG_SIZE;
  if (sessionLogCount < SESSION_LOG_SIZE) sessionLogCount++;
}

void SCPI::reinitEthernet() {
  reinitCount++;

  IPAddress ip;
  String ipString = flash_memory.preferences.getString("IP-ADDRESS", IP_ADDRESS);
  ip.fromString(ipString);

  byte mac[6];
  flash_memory.preferences.getBytes("MAC-ADDRESS", mac, sizeof(mac));

  for (int i = 0; i < MAX_SCPI_CLIENTS; i++) {
    if (scpi_clients[i].active) {
      scpi_clients[i].client.stop();
      scpi_clients[i].active = false;
    }
  }

  Ethernet.begin((uint8_t*)mac, ip);
  server_started = false;   // let processEthernet() re-arm the listener next loop
}

void SCPI::processSerial() {
  my_instrument.ProcessInput(Serial, "\n");
}

void SCPI::PrintDebugInfo(Stream& interface) {
  my_instrument.PrintDebugInfo(interface);
}

void SCPI::logError(int code, const char* message) {
    if (errorCount < ERROR_QUEUE_SIZE) {
        // Normal case: Add error and move head
        errorQueue[errorHead].code = code;
        strncpy(errorQueue[errorHead].message, message, sizeof(errorQueue[errorHead].message) - 1);
        errorQueue[errorHead].message[sizeof(errorQueue[errorHead].message) - 1] = '\0';
        errorHead = (errorHead + 1) % ERROR_QUEUE_SIZE;
        errorCount++;
    } else {
        // Overflow handling: Keep the oldest errors, discard the newest
        queueOverflow = true;
    }
}

const char* SCPI::getError() {
    static char response[100]; // Buffer for formatted output

    if (errorCount == 0) {
        queueOverflow = false; // Reset overflow flag after queue is cleared
        return "0, \"No error\"";
    }

    // If overflow occurred, return the queue overflow error
    if (queueOverflow) {
        queueOverflow = false; // Only return overflow once
        return "-350, \"Queue overflow\"";
    }

    // Get and remove the oldest error
    snprintf(response, sizeof(response), "%d, \"%s\"", errorQueue[errorTail].code, errorQueue[errorTail].message);
    errorTail = (errorTail + 1) % ERROR_QUEUE_SIZE;
    errorCount--;

    return response;
}

int SCPI::getPort() {
    return port;
}

void SCPI::printSessionList(Stream& interface) {
  int count = 0;
  for (int i = 0; i < MAX_SCPI_CLIENTS; i++) {
    if (scpi_clients[i].active) count++;
  }
  interface.print(F("#SESSIONS "));
  interface.print(count);
  interface.write('\n');

  unsigned long now = millis();
  for (int i = 0; i < MAX_SCPI_CLIENTS; i++) {
    if (!scpi_clients[i].active) continue;
    SCPIClient& s = scpi_clients[i];
    // Classification heuristic: a slot that has never received a byte is presumed to be a
    // non-SCPI probe (e.g. a router health check) rather than a real client.
    const char* classification = (s.rxBytes > 0) ? "SCPI" : "PING";
    interface.print(i); interface.print(',');
    interface.print(classification); interface.print(',');
    interface.print(socketStatusName(s.client.status())); interface.print(',');
    interface.print(s.remoteIP); interface.print(',');
    interface.print(s.remotePort); interface.print(',');
    interface.print(port); interface.print(',');
    interface.print(F("age=")); interface.print(now - s.connectedAt); interface.print(',');
    interface.print(F("idle=")); interface.print(now - s.lastActivity); interface.print(',');
    interface.print(F("rx=")); interface.print(s.rxBytes); interface.print(',');
    interface.print(F("tx=")); interface.print(s.txBytes);
    interface.write('\n');
  }
}

void SCPI::printSessionLog(Stream& interface) {
  interface.print(F("#CLOSES "));
  interface.print(sessionLogCount);
  interface.write('\n');

  unsigned long now = millis();
  int start = (sessionLogHead - sessionLogCount + SESSION_LOG_SIZE) % SESSION_LOG_SIZE;
  for (int n = 0; n < sessionLogCount; n++) {
    SessionCloseLog& e = sessionLog[(start + n) % SESSION_LOG_SIZE];
    interface.print(F("ago_ms=")); interface.print(now - e.closedAt); interface.print(',');
    interface.print(F("remote=")); interface.print(e.remoteIP); interface.print(':'); interface.print(e.remotePort); interface.print(',');
    interface.print(F("reason=")); interface.print(e.reason); interface.print(',');
    interface.print(F("duration_ms=")); interface.print(e.durationMs); interface.print(',');
    interface.print(F("idle_ms=")); interface.print(e.idleMsAtClose); interface.print(',');
    interface.print(F("rx=")); interface.print(e.rxBytes); interface.print(',');
    interface.print(F("tx=")); interface.print(e.txBytes);
    interface.write('\n');
  }
}

void SCPI::printSocketList(Stream& interface) {
  interface.print(F("#SOCKETS "));
  interface.print(MAX_SOCK_NUM);
  interface.write('\n');

  for (uint8_t i = 0; i < MAX_SOCK_NUM; i++) {
    EthernetClient probe(i);  // wraps an existing hardware socket index for a read-only status query
    bool tracked = false;
    for (int j = 0; j < MAX_SCPI_CLIENTS; j++) {
      if (scpi_clients[j].active && scpi_clients[j].client.getSocketNumber() == i) {
        tracked = true;
        break;
      }
    }
    interface.print(i); interface.print(',');
    interface.print(socketStatusName(probe.status())); interface.print(',');
    interface.print(tracked ? F("SCPI-TRACKED") : F("UNTRACKED"));
    interface.write('\n');
  }
}

void SCPI::printSystemDiagnostics(Stream& interface) {
  interface.print(F("uptime_s=")); interface.print(millis() / 1000);
  interface.print(F(",free_heap=")); interface.print(ESP.getFreeHeap());
  interface.print(F(",min_free_heap=")); interface.print(esp_get_minimum_free_heap_size());
  interface.print(F(",reset_reason=")); interface.print(resetReasonToString(esp_reset_reason()));
  interface.print(F(",boot_count=")); interface.print(bootCount);
  interface.print(F(",link_flaps=")); interface.print(linkFlapCount);
  interface.print(F(",eth_reinits=")); interface.print(reinitCount);
  interface.write('\n');
}

void SCPI::printAllHeaders(Stream& interface) {
  interface.print(F("#HEADERS "));
  interface.print(kCommandCount);
  interface.write('\n');
  for (int i = 0; i < kCommandCount; i++) {
    interface.print(kCommands[i].path);
    interface.write('\n');
  }
}

void SCPI::printHeaderSyntax(Stream& interface, const char* header) {
  // Defensively strip a single pair of surrounding double quotes, since some SCPI clients quote
  // string parameters and the underlying parser doesn't strip them itself.
  String query(header);
  query.trim();
  if (query.length() >= 2 && query[0] == '"' && query[query.length() - 1] == '"') {
    query = query.substring(1, query.length() - 1);
  }

  for (int i = 0; i < kCommandCount; i++) {
    if (query.equalsIgnoreCase(kCommands[i].path)) {
      interface.print(kCommands[i].description);
      interface.write('\n');
      return;
    }
  }

  // A query must always produce a response — never leave the client waiting.
  logError(224, "Illegal parameter value");
  interface.print(F("UNKNOWN"));
  interface.write('\n');
}