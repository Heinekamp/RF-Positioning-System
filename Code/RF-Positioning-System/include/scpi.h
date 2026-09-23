/**
 * @file scpi.h
 * @author Max Heinekamp
 * @brief This file contains the functions and classes related to SCPI communication.
 * @version 0.1
 * @date 27-01-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SCPI_H
#define SCPI_H

#include <Ethernet.h>
#include "config.h"   // defines SCPI_HASH_TYPE — must precede the parser include below
#define VREKRER_SCPI_PARSER_NO_IMPL
#include "Vrekrer_scpi_parser.h"
#include "stepper.h"

/**
 * @brief This class handles the SCPI communication with the instrument. It is responsible for parsing SCPI commands, sending responses, and managing errors.
 * @details The SCPI class uses the Vrekrer SCPI parser library to handle SCPI commands. It also manages an error queue to store error messages and codes.
 * The class is designed to work with both Ethernet and Serial communication interfaces. It registers all available SCPI commands and processes incoming commands from the specified interface.
 */
class SCPI {
    public:

    /**
     * @brief Construct a new SCPI object
     * 
     */
    SCPI();

    /**
     * @brief Set up the SCPI server and register all available commands.
     * 
     */
    void setup();

    /**
     * @brief Process incoming SCPI commands over Ethernet.
     * 
     */
    void processEthernet();

    /**
     * @brief Process incoming SCPI commands over Serial.
     * 
     */
    void processSerial();

    /**
     * @brief Print debug information about the SCPI parser to the specified interface.
     * 
     * @param interface Stream interface to print debug information to.
     */
    void PrintDebugInfo(Stream& interface);

    /**
     * @brief Log an error message to the SCPI error queue, compliant with section 21.8 of the SCPI standard.
     * 
     * @param code Error code (see SCPI standard)
     * @param message Error message (see SCPI standard)
     */
    void logError(int code, const char* message);
    
    /**
     * @brief Get the oldest error message from the error queue, compliant with section 21.8 of the SCPI standard.
     * 
     * @return const char* String containing the oldest error message with the error code as a csv.
     */
    const char* getError();

    /**
     * @brief Get the port number of the SCPI server.
     *
     * @return int port number of the SCPI server.
     */
    int getPort();

    /**
     * @brief Re-initialize the Ethernet stack (re-reads IP/MAC from flash, re-begins Ethernet and the
     * SCPI server, and drops all tracked client sessions). Used to recover after a hardware reset.
     *
     */
    void reinitEthernet();

    /**
     * @brief Print the live SCPI session table (DIAGnostic:SESSion:LIST?).
     *
     * @param interface Stream interface to print to.
     */
    void printSessionList(Stream& interface);

    /**
     * @brief Print the session close-reason ring buffer (DIAGnostic:SESSion:LOG?).
     *
     * @param interface Stream interface to print to.
     */
    void printSessionLog(Stream& interface);

    /**
     * @brief Print the raw state of all 8 W5500 hardware sockets, not just the ones tracked as
     * SCPI sessions (DIAGnostic:SOCKet:LIST?).
     *
     * @param interface Stream interface to print to.
     */
    void printSocketList(Stream& interface);

    /**
     * @brief Print system/reset health diagnostics (DIAGnostic:SYStem?).
     *
     * @param interface Stream interface to print to.
     */
    void printSystemDiagnostics(Stream& interface);

    /**
     * @brief Print every registered SCPI command header, one per line (SYSTem:HELP:HEADers?).
     *
     * @param interface Stream interface to print to.
     */
    void printAllHeaders(Stream& interface);

    /**
     * @brief Print syntax help for a single command header (SYSTem:HELP:SYNTax?).
     *
     * @param interface Stream interface to print to.
     * @param header The header to look up (must match its full spelling as printed by
     * printAllHeaders() — short forms are not resolved here).
     */
    void printHeaderSyntax(Stream& interface, const char* header);

    private:
    /**
     * @brief Register SCPI commands from scpi_commands.h/cpp
     *
     */
    void registerCommands();

    EthernetServer* scpi_server = nullptr;
    SCPI_Parser my_instrument;

    // Hardware ceiling: the W5500 has 8 sockets, one of which is reserved for LISTEN.
    static const int MAX_SCPI_CLIENTS = 4;
    struct SCPIClient {
        EthernetClient client;
        bool active = false;
        unsigned long connectedAt = 0;   // millis() when this slot was last assigned, for oldest-session eviction
        unsigned long lastActivity = 0;  // millis() of last byte transferred either direction
        unsigned long rxBytes = 0;
        unsigned long txBytes = 0;
        IPAddress remoteIP;              // cached at accept time — client.remoteIP() is unusable once stopped
        uint16_t remotePort = 0;         // cached at accept time
    };
    SCPIClient scpi_clients[MAX_SCPI_CLIENTS];

    /**
     * @brief Stream wrapper around a tracked SCPIClient's EthernetClient that transparently counts
     * bytes transferred and timestamps last activity, without needing any changes to the vendored
     * SCPI parser library (which only ever needs a plain Stream&).
     */
    class SessionStream : public Stream {
        public:
        SessionStream(SCPIClient& session) : session_(session) {}
        int available() override { return session_.client.available(); }
        int read() override {
            int c = session_.client.read();
            if (c >= 0) {
                session_.rxBytes++;
                session_.lastActivity = millis();
            }
            return c;
        }
        int peek() override { return session_.client.peek(); }
        void flush() override { session_.client.flush(); }
        size_t write(uint8_t b) override {
            size_t n = session_.client.write(b);
            if (n) {
                session_.txBytes += n;
                session_.lastActivity = millis();
            }
            return n;
        }
        size_t write(const uint8_t* buf, size_t size) override {
            size_t n = session_.client.write(buf, size);
            if (n) {
                session_.txBytes += n;
                session_.lastActivity = millis();
            }
            return n;
        }
        private:
        SCPIClient& session_;
    };

    // Ring buffer of the last SESSION_LOG_SIZE session closures, for DIAGnostic:SESSion:LOG?.
    static const int SESSION_LOG_SIZE = 16;
    struct SessionCloseLog {
        unsigned long closedAt = 0;
        IPAddress remoteIP;
        uint16_t remotePort = 0;
        char reason[16] = "";
        unsigned long durationMs = 0;
        unsigned long idleMsAtClose = 0;
        unsigned long rxBytes = 0;
        unsigned long txBytes = 0;
    };
    SessionCloseLog sessionLog[SESSION_LOG_SIZE];
    int sessionLogHead = 0;
    int sessionLogCount = 0;
    void logSessionClose(const SCPIClient& slot, const char* reason);

    unsigned long linkFlapCount = 0;  // Ethernet PHY link drops observed (LinkON -> not LinkON)
    unsigned long reinitCount = 0;    // number of times reinitEthernet() has been called
    EthernetLinkStatus lastLinkStatus = Unknown;

    bool server_started = false;

    /**
     * @brief Struct to represent an error in the SCPI error queue.
     * 
     */
    struct SCPI_Error {
        int code;
        char message[100];
    };

    // Initialize error queue with a fixed size
    SCPI_Error errorQueue[ERROR_QUEUE_SIZE];
    int errorHead = 0;  ///< Points to the next free slot
    int errorTail = 0;  ///< Points to the oldest error
    int errorCount = 0; ///< Tracks number of stored errors
    bool queueOverflow = false; ///< Tracks if queue overflowed

    int port = 0;   ///<Store SCPI port number at time of initialization

};

#endif // SCPI_H