#pragma once
#include <stddef.h>
#include <stdint.h>

// GoodWe-Client: Modbus ueber UDP (WiFi/LAN-Kit, Port 8899) oder Modbus TCP (LAN-Kit V2.0,
// Kit-20, Port 502), als NICHT-BLOCKIERENDE Zustandsmaschine.
//
// Diese Uebersetzungseinheit ist bewusst KNX-FREI (keine OpenKNX-/knx-Header) - Muster aus
// OFM-SolarmanPV/SolarmanV5Client.
//
// Rahmen nach der Python-Bibliothek goodwe (modbus.py):
//
//   UDP-Anfrage   : Modbus-RTU-Rahmen  Adresse FC Register Wert/Anzahl CRC16 (LSB zuerst)
//   UDP-Antwort   : AA 55 Adresse FC ... CRC16 - die CRC deckt den Rahmen OHNE AA 55 ab
//                   Lesen  : AA 55 Adr 03 ByteCount Daten... CRC   (ByteCount + 7 Byte)
//                   Schreib: AA 55 Adr 06|10 Reg Wert|Anzahl CRC  (10 Byte)
//                   Fehler : AA 55 Adr FC|80 Code CRC              (7 Byte)
//   TCP           : MBAP-Kopf (TID, 0, Laenge, Unit), dann die PDU ohne CRC. Das
//                   Laengenfeld der Antwort wird wie in der Bibliothek ignoriert (GoodWe-Fehler).
//
// Referenz-Testvektoren (CRC von Hand nachgerechnet):
//   ET Laufzeitdaten 35100/125 : F7 03 89 1C 00 7D 7A E7
//   DT Laufzeitdaten 30100/73  : 7F 03 75 94 00 49 D5 C2
//
// poll() kehrt immer sofort zurueck. Der Socket ist auf O_NONBLOCK gesetzt, connect() laeuft
// ueber EINPROGRESS + select() mit Timeout 0.

class GoodWeClient
{
  public:
    enum Protocol : uint8_t
    {
        Udp = 0,
        Tcp = 1,
    };

    enum Result : uint8_t
    {
        Ok = 0,
        Pending,
        ErrNotConfigured,
        ErrBusy,
        ErrSocket,     // Socket konnte nicht angelegt/verbunden werden
        ErrConnect,    // Verbindungsaufbau abgelehnt
        ErrTimeout,
        ErrFrame,      // kein gueltiger Rahmen
        ErrCrc,        // Modbus-CRC falsch
        ErrException,  // Modbus-Exception vom Geraet, Code in exceptionCode()
        ErrMismatch,   // Antwort passt nicht zur Anfrage
        ErrArgs,       // ungueltige Anzahl
    };

    // Modbus-Exception-Codes
    static const uint8_t EX_ILLEGAL_DATA_ADDRESS = 2;

    static const uint16_t MAX_REGISTERS = 125;
    static const uint8_t MAX_WRITE_REGISTERS = 8;

    // Nur IPv4-Literale, keine Hostnamen: Namensaufloesung wuerde blockieren.
    void configure(const char* host, uint16_t port, Protocol protocol, uint8_t address);
    void setAddress(uint8_t address) { _address = address; }

    bool beginRead(uint16_t start, uint16_t count);
    bool beginWrite(uint16_t reg, uint16_t value);
    bool beginWriteMulti(uint16_t reg, const uint16_t* values, uint8_t count);

    // In jedem loop() aufrufen. Kehrt immer sofort zurueck.
    void poll();

    bool busy() const { return _state != Idle && _state != Complete; }
    bool finished() const { return _state == Complete; }
    Result result() const { return _result; }
    uint8_t exceptionCode() const { return _exception; }

    // Nach finished(): Ergebnis abholen und die Maschine wieder freigeben.
    void clear();

    const uint16_t* registers() const { return _regs; }
    uint16_t registerCount() const { return _regCount; }

    bool configured() const { return _host[0] != 0; }
    const char* host() const { return _host; }
    uint16_t port() const { return _port; }
    Protocol protocol() const { return _protocol; }
    uint8_t address() const { return _address; }

    // Weckruf an den Dongle (UDP 48899, wie die SolarGo-App). Manche Dongles antworten nach
    // dem naechtlichen Einfrieren erst wieder, nachdem sie diesen Rahmen gesehen haben.
    // Feuern und vergessen, keine Antwort.
    static bool sendWakeup(const char* host);

    static uint16_t crc16(const uint8_t* data, size_t len);
    static const char* resultText(Result result);

  private:
    enum State : uint8_t
    {
        Idle = 0,
        Connecting,
        Sending,
        Receiving,
        Complete,
    };

    static const size_t REQUEST_CAP = 13 + 2 * MAX_WRITE_REGISTERS + 2;
    static const size_t RESPONSE_CAP = 9 + 2 * MAX_REGISTERS + 2;

    char _host[16] = {0};
    uint16_t _port = 8899;
    Protocol _protocol = Udp;
    uint8_t _address = 0xF7;
    uint16_t _tid = 0;

    State _state = Idle;
    Result _result = Ok;
    uint8_t _exception = 0;
    int _sock = -1;
    uint32_t _deadline = 0;
    uint8_t _attemptsLeft = 0;

    uint8_t _request[REQUEST_CAP];
    size_t _requestLen = 0;
    size_t _sent = 0;

    uint8_t _response[RESPONSE_CAP];
    size_t _received = 0;

    uint8_t _fc = 3;
    uint16_t _reqReg = 0;
    uint16_t _reqValue = 0; // Wert (FC 06) bzw. Anzahl (FC 03 / 10)
    uint16_t _regs[MAX_REGISTERS];
    uint16_t _regCount = 0;

    bool begin(uint8_t fc, uint16_t reg, uint16_t value, const uint16_t* values, uint8_t count);
    bool startAttempt();
    bool openSocket();
    void closeSocket();
    void fail(Result reason);
    void retryOrFail(Result reason);
    size_t expectedLength() const; // 0 = noch unbekannt
    void parse();
};
