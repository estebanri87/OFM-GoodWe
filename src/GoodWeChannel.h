#pragma once
#include "GoodWeClient.h"
#include "GoodWeData.h"
#include "OpenKNX.h"

// Ein Kanal = ein Wechselrichter mit eigener Verbindung. Welche Objekte angelegt und gesendet
// werden, steht in den Aktiv-Bits der ETS; gelesen werden nur die Registerbloecke, in denen
// mindestens ein aktives Objekt liegt.
class GoodWeChannel : public OpenKNX::Channel
{
  public:
    explicit GoodWeChannel(uint8_t channelIndex);
    ~GoodWeChannel();

    const std::string name() override;
    void setup() override;
    void loop() override;
    void processInputKo(GroupObject& ko) override;

    // Diagnose fuer die Konsole
    bool configured() const { return _client.configured(); }
    bool reachable() const { return _reachable; }
    bool busy() const { return _client.busy(); }
    const GoodWe::DeviceInfo& info() const { return _info; }
    uint16_t activeSensors() const { return _activeCount; }
    uint16_t blockMask() const { return _blockMask; }
    const char* host() const { return _client.host(); }

    // Registerdump anfordern (Konsole). false, wenn schon einer laeuft.
    bool requestDump(uint16_t start, uint16_t count);

  private:
    // Abstand zwischen zwei Anfragen: schont den Dongle
    static const uint32_t MIN_GAP_MS = 250;
    // Einstellungen lesen (aendern sich selten)
    static const uint32_t SETTINGS_INTERVAL_MS = 300000;
    // Weckruf an den Dongle
    static const uint32_t WAKEUP_INTERVAL_MS = 600000;
    static const uint8_t MAX_STEPS = 16;

    enum Phase : uint8_t
    {
        PhaseInfo = 0,  // Geraeteinformation lesen, Familie bestimmen
        PhaseEcoProbe,  // Eco-Modus V1 oder V2
        PhaseReady,
    };

    // Was die laufende Anfrage ist
    enum Request : uint8_t
    {
        ReqNone = 0,
        ReqInfo,
        ReqEcoProbe,
        ReqBlock,
        ReqSetting,
        ReqEcoGroup,
        ReqStep,
        ReqDump,
    };

    // Schreibschritte. Ein Auftrag (z.B. Betriebsmodus) besteht aus mehreren Schritten.
    enum StepKind : uint8_t
    {
        StepWrite = 0,   // FC 06, data[0]
        StepWriteMulti,  // FC 10, count Woerter
        StepRmwHigh,     // High-Byte eines Registers setzen (lesen, dann schreiben)
        StepEcoGroup,    // Eco-Gruppe 1 als Dauer-Laden/-Entladen (lesen, berechnen, schreiben)
    };

    struct Step
    {
        uint8_t kind;
        uint8_t count;
        uint16_t reg;
        uint16_t data[6];
    };

    struct SettingState
    {
        bool enabled = false;
        bool unsupported = false;
        bool known = false;
        bool sent = false;
        int32_t value = 0;
        int32_t sentValue = 0;
    };

    GoodWeClient _client;
    GoodWe::RegisterImage _image;
    GoodWe::DeviceInfo _info;

    uint8_t _type = 0;        // ETS-Typ: 1 automatisch, 2 Hybrid, 3 netzgekoppelt
    uint8_t _family = GoodWe::FamilyUnknown;
    uint8_t _configuredAddress = 0xF7;
    Phase _phase = PhaseInfo;
    // Erkennungsversuche: Familie und Modbus-Adresse
    uint8_t _infoFamily[4] = {};
    uint8_t _infoAddress[4] = {};
    uint8_t _infoCount = 0;
    uint8_t _infoAttempt = 0;
    bool _ecoV2 = false;

    Request _request = ReqNone;
    uint8_t _requestIndex = 0;
    uint32_t _lastRequestMs = 0;
    uint32_t _retryAtMs = 0;

    uint16_t _pollIntervalS = 30;
    uint32_t _lastPollMs = 0;
    bool _pollStarted = false;
    uint32_t _lastWakeupMs = 0;
    bool _wakeupSent = false;

    // aktive Messwerte
    uint16_t* _active = nullptr;
    uint16_t _activeCount = 0;
    double* _lastSent = nullptr;
    uint8_t* _sentOnce = nullptr; // Bitfeld
    bool _reachableEnabled = false;

    // Bloecke
    uint16_t _blockMask = 0;
    uint8_t _meterCount = GoodWe::METER_BASIC;
    bool _blockUnsupported[GoodWe::BLOCK_COUNT] = {};
    uint8_t _blockFallback = 0; // Bitmaske: Block mit kuerzerer Laenge lesen
    uint8_t _cycleBlock = GoodWe::BLOCK_NONE;
    bool _cycleOk = false;

    // Einstellungen
    SettingState _settings[GoodWe::SETTING_COUNT];
    bool _anySetting = false;
    bool _settingsCycle = false;
    uint8_t _settingIndex = 0;
    bool _ecoRead = false;          // Eco-Gruppe im laufenden Zyklus gelesen
    uint16_t _eco[6] = {};
    bool _ecoKnown = false;
    uint32_t _settingsDueMs = 0;
    uint8_t _ecoPower = 100;
    uint8_t _ecoSoc = 100;

    // Schreibauftraege
    Step _steps[MAX_STEPS];
    uint8_t _stepHead = 0;
    uint8_t _stepCount = 0;
    uint8_t _stepPhase = 0;

    bool _reachable = false;
    bool _lastReachable = false;
    bool _reachableSent = false;
    uint32_t _lastCyclicMs = 0;

    bool _dumpPending = false;
    uint16_t _dumpStart = 0;
    uint16_t _dumpCount = 0;

    bool bitEnabled(uint16_t bit);
    void configureActive();
    void startNextRequest();
    void handleResult();

    bool startInfo();
    void handleInfo(bool ok);
    bool nextBlock(uint8_t from, uint8_t& block) const;
    uint8_t blockCount(uint8_t block) const;
    void handleBlock(bool ok);
    void startSettingsCycle();
    bool startNextSetting();
    void handleSetting(bool ok);
    bool settingRegister(uint8_t id, uint16_t& reg, uint8_t& count) const;
    bool settingUsesEco(uint8_t id) const;
    void updateSettingFromEco();
    bool startStep();
    void handleStep(bool ok);
    void clearSteps();

    bool pushStep(uint8_t kind, uint16_t reg, uint8_t count, const uint16_t* data);
    bool pushWrite(uint16_t reg, uint16_t value);
    void queueOperationMode(uint8_t mode);
    void queueSetting(uint8_t id, GroupObject& ko);
    void buildEcoGroup(uint8_t mode, uint16_t* words, uint8_t& count) const;
    bool ecoIsCharge() const;
    bool ecoIsDischarge() const;
    int16_t ecoPowerPercent() const;
    uint16_t ecoSocValue() const;

    void publishValues();
    void sendSensor(uint16_t slot, double value);
    void publishSetting(uint8_t id);
    void updateReachableKo();
    void setReachable(bool reachable);
};
