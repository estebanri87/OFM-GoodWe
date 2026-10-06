#include "GoodWeChannel.h"
#include "GoodWeModule.h"
#include "NetworkModule.h"
#include "knxprod.h"
#include <math.h>
#include <string.h>

using namespace GoodWe;

namespace
{
// Register der Einstellungen (goodwe et.py / dt.py)
constexpr uint16_t REG_WORK_MODE = 47000;
constexpr uint16_t REG_EMS_MODE = 47511;      // 2 Register: EMS-Modus, EMS-Leistung
constexpr uint16_t REG_CLEAR_BATTERY_MODE = 47533;
constexpr uint16_t REG_BACKUP_SUPPLY = 45252;
constexpr uint16_t REG_COLD_START = 45248;
constexpr uint16_t REG_ECO_V1 = 47515;        // 4 Register je Gruppe
constexpr uint16_t REG_ECO_V2 = 47547;        // 6 Register je Gruppe
constexpr uint16_t REG_TIME_ET = 45200;
constexpr uint16_t REG_TIME_DT = 40313;
constexpr uint16_t REG_DT_START = 40330;
constexpr uint16_t REG_DT_STOP = 40331;

// Schalter der Eco-Gruppen 2-4 (High-Byte)
constexpr uint16_t ECO_SWITCH_V1[3] = {47522, 47526, 47530};
constexpr uint16_t ECO_SWITCH_V2[3] = {47555, 47561, 47567};

// Zeitplantypen (goodwe sensor.py ScheduleType)
constexpr int8_t SCHEDULE_ECO = 0;
constexpr int8_t SCHEDULE_ECO_745 = 6;

int8_t scheduleType(int8_t onOff)
{
    // detect_schedule_type: an/aus kodiert als -1-typ bzw. typ
    if (onOff == 85)
        return 85;
    return onOff < 0 ? (int8_t)(-1 - onOff) : onOff;
}

uint8_t clampU8(double v, double max = 255)
{
    if (v < 0)
        return 0;
    if (v > max)
        return (uint8_t)max;
    return (uint8_t)lround(v);
}
} // namespace

GoodWeChannel::GoodWeChannel(uint8_t channelIndex)
{
    _channelIndex = channelIndex;
}

GoodWeChannel::~GoodWeChannel()
{
    delete[] _active;
    delete[] _lastSent;
    delete[] _sentOnce;
}

const std::string GoodWeChannel::name()
{
    return "GoodWe";
}

bool GoodWeChannel::bitEnabled(uint16_t bit)
{
    const uint8_t b = knx.paramByte(GDW_ParamCalcIndex(ENABLE_OFFSET + bit / 8));
    // ETS zaehlt BitOffset vom hoechstwertigen Bit an
    return (b & (0x80 >> (bit % 8))) != 0;
}

void GoodWeChannel::setup()
{
    _type = ParamGDW_CHType;
    _pollIntervalS = ParamGDW_CHPollInterval;
    if (_pollIntervalS < 10)
        _pollIntervalS = 10;

    char ip[17];
    memcpy(ip, ParamGDW_CHIp, 16);
    ip[16] = 0;

    const GoodWeClient::Protocol protocol = (ParamGDW_CHProtocol == 1) ? GoodWeClient::Tcp : GoodWeClient::Udp;
    _configuredAddress = ParamGDW_CHAddress;
    _client.configure(ip, ParamGDW_CHPort, protocol, _configuredAddress);

    // Erkennungsreihenfolge. Die Bibliothek nutzt 0xF7 fuer ET und 0x7F fuer DT; bei
    // abweichender Einstellung wird die Standardadresse zusaetzlich versucht.
    _infoCount = 0;
    auto add = [this](uint8_t family, uint8_t address) {
        for (uint8_t i = 0; i < _infoCount; i++)
            if (_infoFamily[i] == family && _infoAddress[i] == address)
                return;
        _infoFamily[_infoCount] = family;
        _infoAddress[_infoCount] = address;
        _infoCount++;
    };
    if (_type == 2 || _type == 1)
        add(FamilyEt, _configuredAddress);
    if (_type == 3 || _type == 1)
        add(FamilyDt, _configuredAddress);
    if (_type == 2 || _type == 1)
        add(FamilyEt, 0xF7);
    if (_type == 3 || _type == 1)
        add(FamilyDt, 0x7F);

    _reachableEnabled = bitEnabled(0);
    _anySetting = false;
    for (uint8_t i = 0; i < SETTING_COUNT; i++)
    {
        _settings[i] = SettingState();
        _settings[i].enabled = bitEnabled(SENSOR_CAPACITY + i);
        _anySetting |= _settings[i].enabled;
    }

    logDebugP("%s:%d %s, Adresse %d, Intervall %ds", ip[0] ? ip : "(keine IP)", (int)_client.port(),
              protocol == GoodWeClient::Tcp ? "Modbus TCP" : "UDP", (int)_configuredAddress, (int)_pollIntervalS);
}

// Wird aufgerufen, sobald die Familie bekannt ist.
void GoodWeChannel::configureActive()
{
    const uint8_t famMask = familyMask(_family);

    delete[] _active;
    delete[] _lastSent;
    delete[] _sentOnce;
    _active = nullptr;
    _lastSent = nullptr;
    _sentOnce = nullptr;
    _activeCount = 0;
    _blockMask = 0;

    uint16_t count = 0;
    for (uint16_t k = 1; k < SENSOR_COUNT; k++)
        if (sourceOf(SENSORS[k], _family).type != TypeNone && bitEnabled(k))
            count++;

    if (count > 0)
    {
        _active = new uint16_t[count];
        _lastSent = new double[count];
        _sentOnce = new uint8_t[(count + 7) / 8]();
    }

    uint16_t meterEnd = 0;
    for (uint16_t k = 1; k < SENSOR_COUNT && _activeCount < count; k++)
    {
        const Sensor& s = SENSORS[k];
        const Source& src = sourceOf(s, _family);
        if (src.type == TypeNone || !bitEnabled(k))
            continue;
        _active[_activeCount] = k;
        _lastSent[_activeCount] = 0;
        _activeCount++;
        _blockMask |= blocksOf(src, _family, s.cond);

        if (_family == FamilyEt && blockOf(_family, src.reg) == BlkEtMeter && src.type != TypeCalc)
        {
            const uint16_t end = src.reg + sourceWords(src);
            if (end > meterEnd)
                meterEnd = end;
        }
    }

    // Zaehlerblock nur so lang wie noetig; die Erweiterungen kennen nicht alle Geraete.
    if (meterEnd > 36000 + METER_EXT)
        _meterCount = METER_EXT2;
    else if (meterEnd > 36000 + METER_BASIC)
        _meterCount = METER_EXT;
    else
        _meterCount = METER_BASIC;

    // Einstellungen anderer Familie abschalten
    for (uint8_t i = 0; i < SETTING_COUNT; i++)
        if ((SETTINGS[i].families & famMask) == 0)
            _settings[i].enabled = false;

    logInfoP("%s %s (%s), %d Messwerte, %d W", _family == FamilyEt ? "Hybrid" : "netzgekoppelt",
             _info.model, _info.serial, (int)_activeCount, (int)_info.ratedPower);
}

void GoodWeChannel::loop()
{
    if (!_client.configured())
        return;

    // Die Zustandsmaschine muss in jedem Durchlauf getaktet werden; sie kehrt sofort zurueck.
    _client.poll();

    if (_client.finished())
    {
        handleResult();
        _client.clear();
        _request = ReqNone;
        _lastRequestMs = millis();
    }
    else if (!_client.busy())
    {
        startNextRequest();
    }

    updateReachableKo();
}

void GoodWeChannel::startNextRequest()
{
    const uint32_t now = millis();
    if (now - _lastRequestMs < MIN_GAP_MS)
        return;
    if (!openknxNetwork.established())
        return;

    // Weckruf beim Start und alle 10 Minuten (mletenay/home-assistant-goodwe-inverter #340)
    if (!_wakeupSent || now - _lastWakeupMs >= WAKEUP_INTERVAL_MS)
    {
        GoodWeClient::sendWakeup(_client.host());
        _wakeupSent = true;
        _lastWakeupMs = now;
    }

    if (_retryAtMs != 0)
    {
        if ((int32_t)(now - _retryAtMs) < 0)
            return;
        _retryAtMs = 0;
    }

    if (_phase == PhaseInfo)
    {
        startInfo();
        return;
    }

    if (_phase == PhaseEcoProbe)
    {
        _request = ReqEcoProbe;
        _client.beginRead(REG_ECO_V2, 6);
        return;
    }

    if (_dumpPending)
    {
        _dumpPending = false;
        _request = ReqDump;
        if (!_client.beginRead(_dumpStart, _dumpCount))
            logInfoP("gdwread: abgelehnt (%s)", GoodWeClient::resultText(_client.result()));
        return;
    }

    // Schreibauftraege haben Vorrang
    if (_stepCount > 0)
    {
        startStep();
        return;
    }

    // laufenden Lesezyklus fortsetzen
    if (_cycleBlock != BLOCK_NONE)
    {
        _request = ReqBlock;
        _client.beginRead(BLOCKS[_cycleBlock].start, blockCount(_cycleBlock));
        return;
    }

    if (_settingsCycle)
    {
        if (startNextSetting())
            return;
        _settingsCycle = false;
    }

    // neuer Lesezyklus
    if (!_pollStarted || now - _lastPollMs >= (uint32_t)_pollIntervalS * 1000UL)
    {
        _pollStarted = true;
        _lastPollMs = now;
        uint8_t block;
        if (nextBlock(0, block))
        {
            _cycleBlock = block;
            _cycleOk = false;
            _request = ReqBlock;
            _client.beginRead(BLOCKS[block].start, blockCount(block));
            return;
        }
        if (!_anySetting)
        {
            // Nur "Erreichbar" aktiv: die Geraeteinformation dient als Lebenszeichen.
            _phase = PhaseInfo;
            _infoAttempt = 0;
            startInfo();
            return;
        }
    }

    // Einstellungen lesen
    if (_anySetting && (int32_t)(now - _settingsDueMs) >= 0)
    {
        startSettingsCycle();
        if (!startNextSetting())
            _settingsCycle = false;
    }
}

void GoodWeChannel::handleResult()
{
    const bool ok = _client.result() == GoodWeClient::Ok;

    switch (_request)
    {
        case ReqInfo:
            handleInfo(ok);
            break;

        case ReqEcoProbe:
            if (ok)
            {
                _ecoV2 = true;
                _phase = PhaseReady;
            }
            else if (_client.result() == GoodWeClient::ErrException)
            {
                _ecoV2 = false; // EcoModeV1, aeltere ARM-Firmware
                _phase = PhaseReady;
            }
            else
            {
                _retryAtMs = millis() + (uint32_t)_pollIntervalS * 1000UL;
            }
            if (_phase == PhaseReady)
                logDebugP("Eco-Modus %s", _ecoV2 ? "V2" : "V1");
            break;

        case ReqBlock:
            handleBlock(ok);
            break;

        case ReqSetting:
            handleSetting(ok);
            break;

        case ReqEcoGroup:
            _ecoRead = true;
            if (ok)
            {
                memcpy(_eco, _client.registers(), _client.registerCount() * sizeof(uint16_t));
                _ecoKnown = true;
                updateSettingFromEco();
            }
            break;

        case ReqStep:
            handleStep(ok);
            break;

        case ReqDump:
            if (ok)
            {
                logInfoP("%d Register ab %u", (int)_client.registerCount(), (unsigned)_dumpStart);
                logIndentUp();
                for (uint16_t i = 0; i < _client.registerCount(); i++)
                {
                    const uint16_t raw = _client.registers()[i];
                    logInfoP("%u = %5u (0x%04X, signed %6d)", (unsigned)(_dumpStart + i), (unsigned)raw,
                             (unsigned)raw, (int)(int16_t)raw);
                }
                logIndentDown();
            }
            else
            {
                logInfoP("Dump fehlgeschlagen (%s, Code %d)", GoodWeClient::resultText(_client.result()),
                         (int)_client.exceptionCode());
            }
            break;

        default:
            break;
    }
}

// ---------------------------------------------------------------- Geraeteinformation
bool GoodWeChannel::startInfo()
{
    if (_infoCount == 0)
        return false;
    if (_infoAttempt >= _infoCount)
        _infoAttempt = 0;
    const uint8_t family = _infoFamily[_infoAttempt];
    _client.setAddress(_infoAddress[_infoAttempt]);
    _request = ReqInfo;
    if (family == FamilyEt)
        return _client.beginRead(ET_INFO_REG, ET_INFO_COUNT);
    return _client.beginRead(DT_INFO_REG, DT_INFO_COUNT);
}

void GoodWeChannel::handleInfo(bool ok)
{
    const uint8_t family = _infoFamily[_infoAttempt];
    DeviceInfo info;
    if (ok && info.parse(family, _client.registers(), _client.registerCount()))
    {
        const bool first = (_family == FamilyUnknown);
        _info = info;
        if (first)
        {
            _family = family;
            configureActive();
            const bool needEco = _family == FamilyEt &&
                                 (_settings[SetOperationMode].enabled || _settings[SetEcoModePower].enabled ||
                                  _settings[SetEcoModeSoc].enabled);
            _phase = needEco ? PhaseEcoProbe : PhaseReady;
            _settingsDueMs = millis();
        }
        else
        {
            _phase = PhaseReady; // nur Lebenszeichen
        }
        setReachable(true);
        return;
    }

    _infoAttempt++;
    if (_infoAttempt >= _infoCount)
    {
        // Alle Varianten erfolglos: im Abfrageintervall erneut versuchen. Netzgekoppelte
        // Geraete schlafen nachts - das ist normal.
        _infoAttempt = 0;
        setReachable(false);
        _retryAtMs = millis() + (uint32_t)_pollIntervalS * 1000UL;
    }
}

// ---------------------------------------------------------------- Lesezyklus
bool GoodWeChannel::nextBlock(uint8_t from, uint8_t& block) const
{
    const bool battery = _image.batteryPresent();
    for (uint8_t b = from; b < BLOCK_COUNT; b++)
    {
        if ((_blockMask & (1 << b)) == 0 || _blockUnsupported[b])
            continue;
        // Batteriebloecke nur mit Batterie (et.py liest sie nur bei battery_mode != 0)
        if ((b == BlkEtBattery || b == BlkEtBms || b == BlkEtBatCapacity) && !battery)
            continue;
        block = b;
        return true;
    }
    return false;
}

uint8_t GoodWeChannel::blockCount(uint8_t block) const
{
    if (block == BlkEtMeter)
        return _meterCount;
    return (_blockFallback & (1 << block)) ? BLOCKS[block].fallback : BLOCKS[block].count;
}

void GoodWeChannel::handleBlock(bool ok)
{
    const uint8_t block = _cycleBlock;
    if (ok)
    {
        _image.store(block, _client.registers(), _client.registerCount());
        _cycleOk = true;
    }
    else
    {
        _image.invalidate(block);
        const GoodWeClient::Result result = _client.result();

        if (result == GoodWeClient::ErrException && _client.exceptionCode() == GoodWeClient::EX_ILLEGAL_DATA_ADDRESS)
        {
            // Block (in dieser Laenge) nicht vorhanden: kuerzer versuchen oder abschalten.
            if (block == BlkEtMeter && _meterCount > METER_BASIC)
            {
                _meterCount = (_meterCount == METER_EXT2) ? METER_EXT : METER_BASIC;
                logInfoP("Zaehlerblock verkuerzt auf %d Register", (int)_meterCount);
                return; // denselben Block erneut lesen
            }
            if (block != BlkEtMeter && BLOCKS[block].fallback != BLOCKS[block].count &&
                (_blockFallback & (1 << block)) == 0)
            {
                _blockFallback |= (uint8_t)(1 << block);
                return;
            }
            _blockUnsupported[block] = true;
            logInfoP("Block %u wird vom Geraet nicht unterstuetzt", (unsigned)BLOCKS[block].start);
        }
        else if (result == GoodWeClient::ErrTimeout || result == GoodWeClient::ErrSocket ||
                 result == GoodWeClient::ErrConnect)
        {
            // Geraet antwortet nicht: Zyklus abbrechen, statt jeden Block in den Timeout
            // laufen zu lassen.
            logDebugP("Block %u: %s", (unsigned)BLOCKS[block].start, GoodWeClient::resultText(result));
            _image.clear();
            _cycleBlock = BLOCK_NONE;
            setReachable(false);
            return;
        }
        else
        {
            logDebugP("Block %u: %s", (unsigned)BLOCKS[block].start, GoodWeClient::resultText(result));
        }
    }

    uint8_t next;
    if (nextBlock(block + 1, next))
    {
        _cycleBlock = next;
        return;
    }
    _cycleBlock = BLOCK_NONE;
    setReachable(_cycleOk);
    if (_cycleOk)
        publishValues();
}

// ---------------------------------------------------------------- Einstellungen lesen
bool GoodWeChannel::settingUsesEco(uint8_t id) const
{
    return id == SetOperationMode || id == SetEcoModePower || id == SetEcoModeSoc;
}

bool GoodWeChannel::settingRegister(uint8_t id, uint16_t& reg, uint8_t& count) const
{
    const bool dt = (_family == FamilyDt);
    count = 1;
    switch (id)
    {
        case SetOperationMode: reg = REG_WORK_MODE; return !dt;
        case SetEmsMode: reg = REG_EMS_MODE; return !dt;
        case SetEmsPowerLimit: reg = REG_EMS_MODE + 1; return !dt;
        case SetExportLimitEnable: reg = dt ? 40327 : 47509; return true;
        case SetExportLimit:
            reg = dt ? 40328 : 47510;
            count = dt ? 2 : 1; // DT einphasig: 32 Bit in W
            return true;
        case SetExportLimitPercent: reg = 40336; return dt;
        case SetDodOnGrid: reg = 45356; return !dt;
        case SetDodOffGrid: reg = 45358; return !dt;
        case SetSocProtection: reg = 47500; return !dt;
        case SetSocUpperLimit: reg = 47760; return !dt;
        case SetFastCharging: reg = 47545; return !dt;
        case SetFastChargingSoc: reg = 47546; return !dt;
        case SetFastChargingPower: reg = 47603; return !dt;
        case SetBackupSupply: reg = REG_BACKUP_SUPPLY; return !dt;
        case SetDodHolding: reg = 47602; return !dt;
        case SetLoadControl: reg = 47596; return !dt;
        default: return false; // aus der Eco-Gruppe abgeleitet oder reiner Ausloeser
    }
}

void GoodWeChannel::startSettingsCycle()
{
    _settingsCycle = true;
    _settingIndex = 0;
    _ecoRead = false;
    _settingsDueMs = millis() + SETTINGS_INTERVAL_MS;
}

bool GoodWeChannel::startNextSetting()
{
    if (!_ecoRead && _family == FamilyEt)
    {
        bool needEco = false;
        for (uint8_t i = 0; i < SETTING_COUNT; i++)
            needEco |= settingUsesEco(i) && _settings[i].enabled && !_settings[i].unsupported;
        if (needEco)
        {
            _request = ReqEcoGroup;
            _client.beginRead(_ecoV2 ? REG_ECO_V2 : REG_ECO_V1, _ecoV2 ? 6 : 4);
            return true;
        }
        _ecoRead = true;
    }

    while (_settingIndex < SETTING_COUNT)
    {
        const uint8_t id = _settingIndex++;
        const SettingState& st = _settings[id];
        if (!st.enabled || st.unsupported || !SETTINGS[id].hasStatus)
            continue;
        uint16_t reg;
        uint8_t count;
        if (!settingRegister(id, reg, count))
            continue;
        _request = ReqSetting;
        _requestIndex = id;
        _client.beginRead(reg, count);
        return true;
    }
    return false;
}

void GoodWeChannel::handleSetting(bool ok)
{
    const uint8_t id = _requestIndex;
    SettingState& st = _settings[id];
    // Ohne Messwerte sind die Einstellungen das einzige Lebenszeichen.
    if (_blockMask == 0)
        setReachable(ok || _client.result() == GoodWeClient::ErrException);
    if (!ok)
    {
        if (_client.result() == GoodWeClient::ErrException &&
            _client.exceptionCode() == GoodWeClient::EX_ILLEGAL_DATA_ADDRESS)
        {
            st.unsupported = true;
            logInfoP("Einstellung %s wird vom Geraet nicht unterstuetzt", SETTINGS[id].name);
            openknxGoodWeModule.diagnose(_channelIndex, "Funktion fehlt");
        }
        return;
    }

    const uint16_t* r = _client.registers();
    int32_t value = r[0];
    switch (id)
    {
        case SetDodOnGrid:
        case SetDodOffGrid:
            value = 100 - (int32_t)r[0]; // gespeichert wird 100 - Entladetiefe
            break;
        case SetExportLimit:
            if (_client.registerCount() >= 2)
                value = (int32_t)(((uint32_t)r[0] << 16) | r[1]);
            break;
        case SetOperationMode:
            if (value == 3 && _ecoKnown)
                value = ecoIsCharge() ? 6 : (ecoIsDischarge() ? 7 : 3);
            break;
        default:
            break;
    }
    st.value = value;
    st.known = true;
    publishSetting(id);
}

// ---------------------------------------------------------------- Eco-Gruppe 1
bool GoodWeChannel::ecoIsCharge() const
{
    if (!_ecoKnown)
        return false;
    const bool allDay = _eco[0] == 0x0000 && _eco[1] == 0x173B;
    if (!_ecoV2)
    {
        const int8_t onOff = (int8_t)(_eco[3] >> 8);
        const uint8_t days = _eco[3] & 0xFF;
        return allDay && onOff != 0 && days == 127 && (int16_t)_eco[2] < 0;
    }
    const int8_t onOff = (int8_t)(_eco[2] >> 8);
    const uint8_t days = _eco[2] & 0xFF;
    const int8_t type = scheduleType(onOff);
    return allDay && onOff == (int8_t)(-1 - type) && days == 127 && (int16_t)_eco[3] < 0 &&
           (_eco[5] == 0 || _eco[5] == 0x0FFF);
}

bool GoodWeChannel::ecoIsDischarge() const
{
    if (!_ecoKnown)
        return false;
    const bool allDay = _eco[0] == 0x0000 && _eco[1] == 0x173B;
    if (!_ecoV2)
    {
        const int8_t onOff = (int8_t)(_eco[3] >> 8);
        const uint8_t days = _eco[3] & 0xFF;
        return allDay && onOff != 0 && days == 127 && (int16_t)_eco[2] > 0;
    }
    const int8_t onOff = (int8_t)(_eco[2] >> 8);
    const uint8_t days = _eco[2] & 0xFF;
    const int8_t type = scheduleType(onOff);
    return allDay && onOff == (int8_t)(-1 - type) && days == 127 && (int16_t)_eco[3] > 0 &&
           (_eco[5] == 0 || _eco[5] == 0x0FFF);
}

int16_t GoodWeChannel::ecoPowerPercent() const
{
    if (!_ecoV2)
        return (int16_t)abs((int16_t)_eco[2]);
    const int16_t raw = (int16_t)_eco[3];
    const int8_t type = scheduleType((int8_t)(_eco[2] >> 8));
    int16_t p = raw;
    if (type == SCHEDULE_ECO_745)
        p = raw / 10; // 745-Plattform: Promille
    else if (type == 85 && (raw < -100 || raw > 100))
        p = raw / 10;
    return (int16_t)abs(p);
}

uint16_t GoodWeChannel::ecoSocValue() const
{
    return _ecoV2 ? _eco[4] : 100; // V1 kennt keinen Ziel-SoC
}

void GoodWeChannel::updateSettingFromEco()
{
    if (_settings[SetEcoModePower].enabled)
    {
        _settings[SetEcoModePower].value = ecoPowerPercent();
        _settings[SetEcoModePower].known = true;
        publishSetting(SetEcoModePower);
    }
    if (_settings[SetEcoModeSoc].enabled)
    {
        _settings[SetEcoModeSoc].value = ecoSocValue();
        _settings[SetEcoModeSoc].known = true;
        publishSetting(SetEcoModeSoc);
    }
}

void GoodWeChannel::buildEcoGroup(uint8_t mode, uint16_t* words, uint8_t& count) const
{
    const bool charge = (mode == 6);
    if (!_ecoV2)
    {
        // EcoModeV1.encode_charge/encode_discharge: 0000 173B pwr FF7F
        const int16_t p = charge ? (int16_t)-_ecoPower : (int16_t)_ecoPower;
        words[0] = 0x0000;
        words[1] = 0x173B;
        words[2] = (uint16_t)p;
        words[3] = 0xFF7F;
        count = 4;
        return;
    }

    // Schedule.encode_*: Zeitplantyp beibehalten, sonst je nach Plattform
    int8_t type = _info.platform745 ? SCHEDULE_ECO_745 : SCHEDULE_ECO;
    if (_ecoKnown)
    {
        const int8_t current = scheduleType((int8_t)(_eco[2] >> 8));
        if (current == SCHEDULE_ECO || current == SCHEDULE_ECO_745)
            type = current;
    }
    const int16_t enc = (type == SCHEDULE_ECO_745) ? (int16_t)(_ecoPower * 10) : (int16_t)_ecoPower;
    words[0] = 0x0000;
    words[1] = 0x173B;
    words[2] = (uint16_t)(((uint8_t)(255 - type) << 8) | 0x7F);
    words[3] = charge ? (uint16_t)(int16_t)-enc : (uint16_t)enc;
    words[4] = charge ? _ecoSoc : 100;
    words[5] = (type == SCHEDULE_ECO_745) ? 0x0FFF : 0x0000;
    count = 6;
}

// ---------------------------------------------------------------- Schreiben
bool GoodWeChannel::pushStep(uint8_t kind, uint16_t reg, uint8_t count, const uint16_t* data)
{
    if (_stepCount >= MAX_STEPS)
    {
        logInfoP("Schreibpuffer voll, Auftrag verworfen");
        return false;
    }
    Step& s = _steps[(_stepHead + _stepCount) % MAX_STEPS];
    s.kind = kind;
    s.reg = reg;
    s.count = count;
    memset(s.data, 0, sizeof(s.data));
    if (data != nullptr)
        memcpy(s.data, data, count * sizeof(uint16_t));
    _stepCount++;
    return true;
}

bool GoodWeChannel::pushWrite(uint16_t reg, uint16_t value)
{
    return pushStep(StepWrite, reg, 1, &value);
}

void GoodWeChannel::clearSteps()
{
    _stepHead = 0;
    _stepCount = 0;
    _stepPhase = 0;
}

void GoodWeChannel::queueOperationMode(uint8_t mode)
{
    // set_operation_mode aus goodwe/et.py
    const uint16_t emsAuto[2] = {0x0001, 0x0000};
    const uint16_t emsOffGrid[2] = {0x0007, 0x0000};

    switch (mode)
    {
        case 0: // Allgemein
        case 2: // Backup
        case 4: // Peak Shaving
        case 5: // Eigenverbrauch (745-Plattform)
            pushWrite(REG_WORK_MODE, mode);
            pushStep(StepWriteMulti, REG_EMS_MODE, 2, emsAuto);
            pushWrite(REG_CLEAR_BATTERY_MODE, 1);
            break;
        case 1: // Inselbetrieb
            pushWrite(REG_WORK_MODE, 1);
            pushStep(StepWriteMulti, REG_EMS_MODE, 2, emsOffGrid);
            pushWrite(REG_BACKUP_SUPPLY, 1);
            pushWrite(REG_COLD_START, 4);
            pushWrite(REG_CLEAR_BATTERY_MODE, 1);
            break;
        case 3: // Eco
            pushWrite(REG_WORK_MODE, 3);
            pushStep(StepWriteMulti, REG_EMS_MODE, 2, emsAuto);
            break;
        case 6: // Eco Laden
        case 7: // Eco Entladen
        {
            // Eco-Gruppe 1 als Dauerauftrag, Gruppen 2-4 aus, dann Eco-Modus
            const uint16_t m = mode;
            pushStep(StepEcoGroup, _ecoV2 ? REG_ECO_V2 : REG_ECO_V1, 1, &m);
            const uint16_t* sw = _ecoV2 ? ECO_SWITCH_V2 : ECO_SWITCH_V1;
            const uint16_t off = 0;
            for (uint8_t i = 0; i < 3; i++)
                pushStep(StepRmwHigh, sw[i], 1, &off);
            pushWrite(REG_WORK_MODE, 3);
            pushStep(StepWriteMulti, REG_EMS_MODE, 2, emsAuto);
            break;
        }
        default:
            logInfoP("Betriebsmodus %d unbekannt", (int)mode);
            return;
    }
    logInfoP("Betriebsmodus %d wird gesetzt", (int)mode);
}

void GoodWeChannel::queueSetting(uint8_t id, GroupObject& ko)
{
    const bool dt = (_family == FamilyDt);
    const Setting& def = SETTINGS[id];

    // Eingangswert in eine ganze Zahl wandeln
    int32_t v = 0;
    switch (def.encIn)
    {
        case EncBool:
            v = (bool)ko.value(Dpt(def.dptInMain, def.dptInSub)) ? 1 : 0;
            break;
        case EncPercent:
            v = clampU8((float)ko.value(DPT_Scaling), 100);
            break;
        case EncU8:
            v = (uint8_t)ko.value(DPT_Value_1_Ucount);
            break;
        case EncFloat:
        {
            const float f = (float)ko.value(Dpt(def.dptInMain, def.dptInSub));
            v = (f < 0) ? 0 : (int32_t)lroundf(f);
            break;
        }
        default:
            break;
    }

    switch (id)
    {
        case SetOperationMode:
            if (v > 7)
            {
                logInfoP("Betriebsmodus %d ungueltig", (int)v);
                return;
            }
            queueOperationMode((uint8_t)v);
            return;

        case SetEmsMode:
            if (v < 1 || v > 12)
            {
                logInfoP("EMS-Modus %d ungueltig", (int)v);
                return;
            }
            pushWrite(REG_EMS_MODE, (uint16_t)v);
            break;

        case SetEmsPowerLimit:
            pushWrite(REG_EMS_MODE + 1, (uint16_t)(v > 65535 ? 65535 : v));
            break;

        case SetExportLimitEnable:
            pushWrite(dt ? 40327 : 47509, (uint16_t)v);
            break;

        case SetExportLimit:
            if (dt)
            {
                const uint16_t words[2] = {(uint16_t)((uint32_t)v >> 16), (uint16_t)v};
                pushStep(StepWriteMulti, 40328, 2, words);
            }
            else
            {
                pushWrite(47510, (uint16_t)(v > 65535 ? 65535 : v));
            }
            break;

        case SetExportLimitPercent:
            pushWrite(40336, (uint16_t)v);
            break;

        case SetDodOnGrid:
        case SetDodOffGrid:
            if (v > 99)
                v = 99;
            pushWrite(id == SetDodOnGrid ? 45356 : 45358, (uint16_t)(100 - v));
            break;

        case SetSocProtection: pushWrite(47500, (uint16_t)v); break;
        case SetSocUpperLimit: pushWrite(47760, (uint16_t)v); break;
        case SetFastCharging: pushWrite(47545, (uint16_t)v); break;
        case SetFastChargingSoc: pushWrite(47546, (uint16_t)v); break;
        case SetFastChargingPower: pushWrite(47603, (uint16_t)v); break;
        case SetBackupSupply: pushWrite(REG_BACKUP_SUPPLY, (uint16_t)v); break;
        case SetDodHolding: pushWrite(47602, (uint16_t)v); break;
        case SetLoadControl: pushWrite(47596, (uint16_t)v); break;

        case SetEcoModePower:
        case SetEcoModeSoc:
        {
            // Wie die inoffizielle HA-Integration: Leistung und Ziel-SoC gehoeren zu den
            // Modi "Eco Laden/Entladen". Ist einer davon aktiv, wird er neu geschrieben.
            if (id == SetEcoModePower)
                _ecoPower = (uint8_t)v;
            else
                _ecoSoc = (uint8_t)v;
            const SettingState& mode = _settings[SetOperationMode];
            if (mode.known && (mode.value == 6 || mode.value == 7))
                queueOperationMode((uint8_t)mode.value);
            else if (ecoIsCharge() || ecoIsDischarge())
                queueOperationMode(ecoIsCharge() ? 6 : 7);
            return;
        }

        case SetSyncClock:
        {
            if (!openknx.time.isValid())
            {
                logInfoP("Uhr synchronisieren: keine gueltige Uhrzeit");
                return;
            }
            struct tm t;
            openknx.time.getLocalTime().toTm(t);
            const uint16_t words[3] = {
                (uint16_t)(((t.tm_year - 100) << 8) | (t.tm_mon + 1)),
                (uint16_t)((t.tm_mday << 8) | t.tm_hour),
                (uint16_t)((t.tm_min << 8) | t.tm_sec)};
            pushStep(StepWriteMulti, dt ? REG_TIME_DT : REG_TIME_ET, 3, words);
            break;
        }

        case SetStartInverter:
            pushWrite(REG_DT_START, 0);
            break;

        case SetStopInverter:
            pushWrite(REG_DT_STOP, 0);
            break;

        default:
            return;
    }
    logDebugP("%s = %d", def.name, (int)v);
}

bool GoodWeChannel::startStep()
{
    Step& s = _steps[_stepHead];
    _request = ReqStep;
    switch (s.kind)
    {
        case StepWrite:
            return _client.beginWrite(s.reg, s.data[0]);
        case StepWriteMulti:
            return _client.beginWriteMulti(s.reg, s.data, s.count);
        case StepRmwHigh:
            if (_stepPhase == 0)
                return _client.beginRead(s.reg, 1);
            return _client.beginWrite(s.reg, (uint16_t)((s.data[0] << 8) | (s.data[1] & 0xFF)));
        case StepEcoGroup:
        {
            if (_stepPhase == 0)
                return _client.beginRead(s.reg, _ecoV2 ? 6 : 4);
            uint16_t words[6];
            uint8_t count = 0;
            buildEcoGroup((uint8_t)s.data[0], words, count);
            return _client.beginWriteMulti(s.reg, words, count);
        }
    }
    return false;
}

void GoodWeChannel::handleStep(bool ok)
{
    if (_stepCount == 0)
        return;
    Step& s = _steps[_stepHead];
    if (!ok)
    {
        logInfoP("Schreiben auf %u fehlgeschlagen (%s, Code %d)", (unsigned)s.reg,
                 GoodWeClient::resultText(_client.result()), (int)_client.exceptionCode());
        openknxGoodWeModule.diagnose(_channelIndex, "Schreibfehler");
        clearSteps();
        _settingsDueMs = millis() + 2000;
        return;
    }

    if (_stepPhase == 0 && s.kind == StepRmwHigh)
    {
        s.data[1] = _client.registers()[0];
        _stepPhase = 1;
        return;
    }
    if (_stepPhase == 0 && s.kind == StepEcoGroup)
    {
        memcpy(_eco, _client.registers(), _client.registerCount() * sizeof(uint16_t));
        _ecoKnown = true;
        _stepPhase = 1;
        return;
    }

    _stepHead = (uint8_t)((_stepHead + 1) % MAX_STEPS);
    _stepCount--;
    _stepPhase = 0;
    if (_stepCount == 0)
        _settingsDueMs = millis() + 2000; // Status zuruecklesen
}

void GoodWeChannel::processInputKo(GroupObject& ko)
{
    const int index = GDW_KoCalcIndex(ko.asap());
    if (index < (int)SETTING_KO_BASE)
        return;
    const int rel = index - SETTING_KO_BASE;
    if (rel % 2 != 0)
        return; // Statusobjekt
    const uint8_t id = (uint8_t)(rel / 2);
    if (id >= SETTING_COUNT || !_settings[id].enabled)
        return;

    if (_phase != PhaseReady)
    {
        logInfoP("%s: Wechselrichter noch nicht erkannt, Telegramm verworfen", SETTINGS[id].name);
        return;
    }
    if (_settings[id].unsupported)
    {
        logInfoP("%s wird vom Geraet nicht unterstuetzt", SETTINGS[id].name);
        return;
    }
    queueSetting(id, ko);
}

// ---------------------------------------------------------------- Senden
void GoodWeChannel::sendSensor(uint16_t slot, double value)
{
    const Sensor& s = SENSORS[slot];
    GroupObject& ko = knx.getGroupObject(GDW_KoCalcNumber(slot));
    const Dpt dpt(s.dptMain, s.dptSub);

    switch (s.encoding)
    {
        case EncFloat:
        case EncFloat2:
            ko.value((float)value, dpt);
            break;
        case EncEnergyWh:
            ko.value((int32_t)llround(value * 1000.0), dpt);
            break;
        case EncPercent:
            ko.value((float)(value < 0 ? 0 : (value > 100 ? 100 : value)), dpt);
            break;
        case EncU8:
            ko.value(clampU8(value), dpt);
            break;
        case EncU16:
            ko.value((uint16_t)(value < 0 ? 0 : (value > 65535 ? 65535 : lround(value))), dpt);
            break;
        case EncU32:
            ko.value((uint32_t)(value < 0 ? 0 : (value > 4294967295.0 ? 4294967295UL : (uint32_t)llround(value))), dpt);
            break;
        case EncBool:
            ko.value(value != 0, dpt);
            break;
        default:
            break;
    }
}

void GoodWeChannel::publishValues()
{
    const uint32_t cyclicMs = ParamGDW_CHSendDelayTimeMS;
    const uint8_t changePercent = ParamGDW_CHSendChangePercent;
    const bool cyclicDue = (cyclicMs != 0) && delayCheck(_lastCyclicMs, cyclicMs);
    const bool battery = _image.batteryPresent();
    const bool meter = _image.meterOk(_family);

    for (uint16_t i = 0; i < _activeCount; i++)
    {
        const uint16_t slot = _active[i];
        const Sensor& s = SENSORS[slot];
        if ((s.cond & CondBAT) && _family == FamilyEt && !battery)
            continue;
        if ((s.cond & CondMETER) && !meter)
            continue;

        const Source& src = sourceOf(s, _family);
        const bool first = (_sentOnce[i / 8] & (1 << (i % 8))) == 0;

        if (src.type == TypeTS)
        {
            struct tm t;
            if (!_image.decodeTime(src, t))
                continue;
            const double packed = ((((t.tm_year * 12.0 + t.tm_mon) * 31 + t.tm_mday) * 24 + t.tm_hour) * 60 + t.tm_min) * 60 + t.tm_sec;
            if (first || cyclicDue || packed != _lastSent[i])
            {
                knx.getGroupObject(GDW_KoCalcNumber(slot)).value(t, DPT_DateTime);
                _lastSent[i] = packed;
                _sentOnce[i / 8] |= (uint8_t)(1 << (i % 8));
            }
            continue;
        }

        double value = 0;
        if (!_image.decode(src, _family, value))
            continue;

        bool send = first || cyclicDue;
        if (!send)
        {
            const double last = _lastSent[i];
            const bool analog = s.encoding == EncFloat || s.encoding == EncFloat2 || s.encoding == EncEnergyWh;
            if (!analog || changePercent == 0)
            {
                send = (value != last);
            }
            else
            {
                const double reference = fabs(last);
                const double delta = fabs(value - last);
                // Bezugsgroesse 0 -> jede Aenderung zaehlt, sonst gaebe es keine Prozentbasis.
                send = (reference < 0.001) ? (delta > 0.0) : (delta / reference * 100.0 >= changePercent);
            }
        }

        if (send)
        {
            sendSensor(slot, value);
            _lastSent[i] = value;
            _sentOnce[i / 8] |= (uint8_t)(1 << (i % 8));
        }
    }

    if (cyclicDue)
        _lastCyclicMs = millis();
}

void GoodWeChannel::publishSetting(uint8_t id)
{
    SettingState& st = _settings[id];
    const Setting& def = SETTINGS[id];
    if (!def.hasStatus || !st.enabled || !st.known)
        return;
    if (st.sent && st.sentValue == st.value)
        return;

    GroupObject& ko = knx.getGroupObject(GDW_KoCalcNumber(SETTING_KO_BASE + 2 * id + 1));
    const Dpt dpt(def.dptOutMain, def.dptOutSub);
    switch (def.encOut)
    {
        case EncBool:
            ko.value(st.value != 0, dpt);
            break;
        case EncPercent:
            ko.value((float)(st.value < 0 ? 0 : (st.value > 100 ? 100 : st.value)), dpt);
            break;
        case EncU8:
            ko.value(clampU8(st.value), dpt);
            break;
        case EncFloat:
            ko.value((float)st.value, dpt);
            break;
        default:
            return;
    }
    st.sent = true;
    st.sentValue = st.value;
}

void GoodWeChannel::setReachable(bool reachable)
{
    if (reachable != _reachable)
    {
        logInfoP("%s", reachable ? "erreichbar" : "nicht erreichbar");
        openknxGoodWeModule.diagnose(_channelIndex, reachable ? "erreichbar" : "nicht erreichbar");
    }
    _reachable = reachable;
}

void GoodWeChannel::updateReachableKo()
{
    if (!_reachableEnabled)
        return;
    if (!_reachableSent || _reachable != _lastReachable)
    {
        _lastReachable = _reachable;
        _reachableSent = true;
        knx.getGroupObject(GDW_KoCalcNumber(0)).value(_reachable, DPT_Switch);
    }
}

bool GoodWeChannel::requestDump(uint16_t start, uint16_t count)
{
    if (_dumpPending)
        return false;
    _dumpStart = start;
    _dumpCount = count;
    _dumpPending = true;
    return true;
}
