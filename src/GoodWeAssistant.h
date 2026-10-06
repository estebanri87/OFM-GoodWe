#pragma once
#include "GoodWeClient.h"
#include "GoodWeData.h"
#include <stdint.h>

// Inbetriebnahme-Assistent "Wechselrichter auslesen": ermittelt Protokoll, Familie und
// Modbus-Adresse eines Wechselrichters und prueft fuer jedes Objekt des Katalogs, ob das
// Geraet es liefert. Arbeitet mit einem EIGENEN Client, unabhaengig von den Kanaelen - die
// Adresse kommt aus dem ETS-Aufruf, ist also eine gerade eingetippte, noch nicht
// heruntergeladene.
//
// Aufruf ueber processFunctionProperty (objectIndex 0xA4). invokeFunctionProperty ist in der
// ETS synchron, die Erkennung dauert aber Sekunden: deshalb starten und pollen (Muster
// OFM-IPCameraModule / OFM-SolarmanPV). KNX-frei.
class GoodWeAssistant
{
  public:
    enum Error : uint8_t
    {
        ErrNone = 0,
        ErrNoAnswer = 1,   // weder UDP noch TCP
        ErrUnknown = 2,    // antwortet, aber keine verwertbare Geraeteinformation
        ErrNoNetwork = 3,
        ErrBusy = 4,
        ErrNotStarted = 5,
    };

    static const uint8_t BIT_BYTES = (GoodWe::ENABLE_BITS + 7) / 8;
    // Laenge des Ergebnisses ab Byte [2] der Antwort
    static const uint8_t RESULT_BYTES = 12 + BIT_BYTES + 10 + 16;

    void start(uint8_t channel, const char* host, bool networkUp);
    void loop();

    bool running() const { return _state != Idle && _state != Done; }
    bool finished() const { return _state == Done; }
    uint8_t channel() const { return _channel; }
    uint8_t error() const { return _error; }

    // Schreibt das Ergebnis (ab Antwortbyte [2]) und liefert die Laenge.
    uint8_t buildResult(uint8_t* out) const;
    // Zusammenfassung fuer das Log
    uint16_t availableCount() const;
    const GoodWe::DeviceInfo& info() const { return _info; }
    GoodWeClient::Protocol protocol() const { return _protocol; }
    uint8_t address() const { return _address; }

  private:
    enum State : uint8_t
    {
        Idle = 0,
        Probe,     // Protokoll, Adresse und Familie suchen
        Blocks,    // Registerbloecke lesen
        EcoProbe,  // Eco-Gruppe V2 oder V1
        Settings,  // Einstellungen einzeln lesen
        Done,
    };

    GoodWeClient _client;
    GoodWe::RegisterImage _image;
    GoodWe::DeviceInfo _info;

    State _state = Idle;
    uint8_t _error = ErrNotStarted;
    uint8_t _channel = 0;
    char _host[16] = {0};
    uint32_t _lastRequestMs = 0;

    uint8_t _probe = 0;
    bool _answered = false;
    GoodWeClient::Protocol _protocol = GoodWeClient::Udp;
    uint16_t _port = 8899;
    uint8_t _address = 0xF7;

    uint8_t _block = 0;
    uint8_t _blockCount = 0;
    uint8_t _meterCount = GoodWe::METER_BASIC;
    bool _usedFallback = false;

    uint8_t _ecoStep = 0; // 0 = V2 versuchen, 1 = V1 versuchen
    bool _ecoAvailable = false;
    uint8_t _setting = 0;
    bool _settingOk[GoodWe::SETTING_COUNT] = {};

    uint8_t _bits[BIT_BYTES] = {};

    bool startRequest();
    void handle(bool ok);
    bool nextBlock(uint8_t from, uint8_t& block) const;
    bool settingProbe(uint8_t id, uint16_t& reg, uint8_t& count) const;
    void evaluate();
    void setBit(uint16_t bit);
    void finish(uint8_t error);
};
