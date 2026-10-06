// KNX-frei halten: hier darf kein OpenKNX-/knx-Header hinein.
#include "GoodWeAssistant.h"
#include <Arduino.h>
#include <string.h>
#include <time.h>

using namespace GoodWe;

namespace
{
constexpr uint32_t MIN_GAP_MS = 250;

// Suchreihenfolge wie Home Assistant (erst UDP 8899, dann TCP 502); je Protokoll die
// Standardadresse der Familie. Kit-20 mit DT verlangt laut Erfahrungsberichten 247.
struct ProbeDef
{
    GoodWeClient::Protocol protocol;
    uint16_t port;
    uint8_t address;
    uint8_t family;
};
const ProbeDef PROBES[] = {
    {GoodWeClient::Udp, 8899, 0xF7, FamilyEt},
    {GoodWeClient::Udp, 8899, 0x7F, FamilyDt},
    {GoodWeClient::Tcp, 502, 0xF7, FamilyEt},
    {GoodWeClient::Tcp, 502, 0x7F, FamilyDt},
    {GoodWeClient::Tcp, 502, 0xF7, FamilyDt},
};
constexpr uint8_t PROBE_COUNT = sizeof(PROBES) / sizeof(PROBES[0]);
} // namespace

void GoodWeAssistant::start(uint8_t channel, const char* host, bool networkUp)
{
    _client.clear();
    _image.clear();
    _info = DeviceInfo();
    _channel = channel;
    strncpy(_host, host != nullptr ? host : "", sizeof(_host) - 1);
    _host[sizeof(_host) - 1] = 0;
    memset(_bits, 0, sizeof(_bits));
    memset(_settingOk, 0, sizeof(_settingOk));
    _probe = 0;
    _answered = false;
    _block = 0;
    _ecoStep = 0;
    _ecoAvailable = false;
    _setting = 0;
    _usedFallback = false;
    _lastRequestMs = 0;

    if (!networkUp)
    {
        finish(ErrNoNetwork);
        return;
    }
    // Weckruf vorab, falls der Dongle nachts eingefroren ist
    GoodWeClient::sendWakeup(_host);
    _state = Probe;
}

void GoodWeAssistant::finish(uint8_t error)
{
    _client.clear();
    _error = error;
    _state = Done;
}

void GoodWeAssistant::loop()
{
    if (!running())
        return;

    _client.poll();
    if (_client.finished())
    {
        const bool ok = _client.result() == GoodWeClient::Ok;
        handle(ok);
        _client.clear();
        _lastRequestMs = millis();
        return;
    }
    if (_client.busy() || millis() - _lastRequestMs < MIN_GAP_MS)
        return;
    if (!startRequest())
        evaluate();
}

bool GoodWeAssistant::startRequest()
{
    switch (_state)
    {
        case Probe:
        {
            if (_probe >= PROBE_COUNT)
            {
                finish(_answered ? ErrUnknown : ErrNoAnswer);
                return true;
            }
            const ProbeDef& p = PROBES[_probe];
            _client.configure(_host, p.port, p.protocol, p.address);
            if (p.family == FamilyEt)
                _client.beginRead(ET_INFO_REG, ET_INFO_COUNT);
            else
                _client.beginRead(DT_INFO_REG, DT_INFO_COUNT);
            return true;
        }

        case Blocks:
        {
            uint8_t block;
            if (!nextBlock(_block, block))
            {
                _state = (_info.family == FamilyEt) ? EcoProbe : Settings;
                return startRequest();
            }
            _block = block;
            uint8_t count = BLOCKS[block].count;
            if (block == BlkEtMeter)
                count = _meterCount;
            else if (_usedFallback)
                count = BLOCKS[block].fallback;
            _blockCount = count;
            _client.beginRead(BLOCKS[block].start, count);
            return true;
        }

        case EcoProbe:
            if (_ecoStep > 1)
            {
                _state = Settings;
                return startRequest();
            }
            if (_ecoStep == 0)
                _client.beginRead(47547, 6);
            else
                _client.beginRead(47515, 4);
            return true;

        case Settings:
            while (_setting < SETTING_COUNT)
            {
                uint16_t reg;
                uint8_t count;
                if (settingProbe(_setting, reg, count))
                {
                    _client.beginRead(reg, count);
                    return true;
                }
                _setting++;
            }
            return false; // fertig -> auswerten

        default:
            return true;
    }
}

void GoodWeAssistant::handle(bool ok)
{
    switch (_state)
    {
        case Probe:
        {
            const ProbeDef& p = PROBES[_probe];
            if (ok && _info.parse(p.family, _client.registers(), _client.registerCount()))
            {
                _protocol = p.protocol;
                _port = p.port;
                _address = p.address;
                // Zaehlerblock: erweitert nur auf 745-Plattform bzw. ab 15 kW (et.py)
                _meterCount = _info.extendedBlocks() ? METER_EXT2 : METER_BASIC;
                _block = 0;
                _state = Blocks;
                return;
            }
            // Antwort (auch eine Modbus-Exception) heisst: unter dieser Adresse lebt etwas
            if (ok || _client.result() == GoodWeClient::ErrException)
                _answered = true;
            _info = DeviceInfo();
            _probe++;
            return;
        }

        case Blocks:
        {
            const uint8_t block = _block;
            if (ok)
            {
                _image.store(block, _client.registers(), _client.registerCount());
            }
            else if (_client.result() == GoodWeClient::ErrException &&
                     _client.exceptionCode() == GoodWeClient::EX_ILLEGAL_DATA_ADDRESS)
            {
                if (block == BlkEtMeter && _meterCount > METER_BASIC)
                {
                    _meterCount = (_meterCount == METER_EXT2) ? METER_EXT : METER_BASIC;
                    return; // kuerzer erneut
                }
                if (block != BlkEtMeter && !_usedFallback && BLOCKS[block].fallback != BLOCKS[block].count)
                {
                    _usedFallback = true;
                    return;
                }
            }
            _usedFallback = false;
            _block = block + 1;
            return;
        }

        case EcoProbe:
            if (ok)
            {
                _ecoAvailable = true;
                _ecoStep = 2;
            }
            else
            {
                _ecoStep++;
            }
            return;

        case Settings:
            _settingOk[_setting] = ok;
            _setting++;
            return;

        default:
            return;
    }
}

bool GoodWeAssistant::nextBlock(uint8_t from, uint8_t& block) const
{
    const uint8_t mask = familyMask(_info.family);
    const bool battery = _image.batteryPresent();
    for (uint8_t b = from; b < BLOCK_COUNT; b++)
    {
        if ((BLOCKS[b].family & mask) == 0)
            continue;
        // Bedingungen wie et.py: Batteriebloecke nur mit Batterie, zweite Batterie nur bei
        // passenden Modellen, MPPT-Block nur 745-Plattform bzw. ab 15 kW
        if ((b == BlkEtBattery || b == BlkEtBms || b == BlkEtBatCapacity) && !battery)
            continue;
        if ((b == BlkEtBattery2 || b == BlkEtBattery2Ext) && !_info.hasBattery2())
            continue;
        if (b == BlkEtMppt && !_info.extendedBlocks())
            continue;
        block = b;
        return true;
    }
    return false;
}

bool GoodWeAssistant::settingProbe(uint8_t id, uint16_t& reg, uint8_t& count) const
{
    const bool dt = (_info.family == FamilyDt);
    if ((SETTINGS[id].families & familyMask(_info.family)) == 0)
        return false;
    count = 1;
    switch (id)
    {
        case SetOperationMode: reg = 47000; return true;
        case SetEmsMode: reg = 47511; return true;
        case SetEmsPowerLimit: reg = 47512; return true;
        case SetExportLimitEnable: reg = dt ? 40327 : 47509; return true;
        case SetExportLimit:
            if (dt && !_info.singlePhase)
                return false; // dreiphasig nur in Prozent
            reg = dt ? 40328 : 47510;
            count = dt ? 2 : 1;
            return true;
        case SetExportLimitPercent:
            if (_info.singlePhase)
                return false;
            reg = 40336;
            return true;
        case SetDodOnGrid: reg = 45356; return true;
        case SetDodOffGrid: reg = 45358; return true;
        case SetSocProtection: reg = 47500; return true;
        case SetSocUpperLimit: reg = 47760; return true;
        case SetEcoModePower:
        case SetEcoModeSoc:
            return false; // aus der Eco-Probe
        case SetFastCharging: reg = 47545; return true;
        case SetFastChargingSoc: reg = 47546; return true;
        case SetFastChargingPower: reg = 47603; return true;
        case SetBackupSupply: reg = 45252; return true;
        case SetDodHolding: reg = 47602; return true;
        case SetLoadControl: reg = 47596; return true;
        case SetSyncClock:
            reg = dt ? 40313 : 45200;
            count = 3;
            return true;
        case SetStartInverter: reg = 40330; return true;
        case SetStopInverter: reg = 40331; return true;
    }
    return false;
}

void GoodWeAssistant::setBit(uint16_t bit)
{
    if (bit < ENABLE_BITS)
        _bits[bit / 8] |= (uint8_t)(1 << (bit % 8)); // LSB zuerst, so liest es das Skript
}

void GoodWeAssistant::evaluate()
{
    const uint8_t family = _info.family;
    const bool battery = (family == FamilyEt) && _image.batteryPresent();
    const bool meter = _image.meterOk(family);

    setBit(0); // Erreichbar
    for (uint16_t k = 1; k < SENSOR_COUNT; k++)
    {
        const Sensor& s = SENSORS[k];
        const Source& src = sourceOf(s, family);
        if (src.type == TypeNone)
            continue;

        const uint16_t c = s.cond;
        if ((c & (CondL2 | CondL3)) && _info.singlePhase)
            continue;
        if ((c & CondPV3) && !_info.hasPv3())
            continue;
        if ((c & CondPV4) && !_info.hasPv4())
            continue;
        if ((c & CondBAT) && family == FamilyEt && !battery)
            continue;
        if ((c & CondBAT2) && !_info.hasBattery2())
            continue;
        if ((c & CondMETER) && !meter)
            continue;
        if ((c & (CondMPPT | CondMEXT | CondMEXT2)) && !_info.extendedBlocks())
            continue;

        bool available;
        if (src.type == TypeTS)
        {
            struct tm t;
            available = _image.decodeTime(src, t);
        }
        else
        {
            double v;
            available = _image.decode(src, family, v);
        }
        if (available)
            setBit(k);
    }

    for (uint8_t i = 0; i < SETTING_COUNT; i++)
    {
        bool ok = _settingOk[i];
        if (i == SetEcoModePower || i == SetEcoModeSoc)
            ok = (family == FamilyEt) && _ecoAvailable;
        if (i == SetOperationMode)
            ok = ok && _ecoAvailable; // Eco-Laden/-Entladen brauchen die Eco-Gruppe
        if (ok)
            setBit(SENSOR_CAPACITY + i);
    }
    finish(ErrNone);
}

uint16_t GoodWeAssistant::availableCount() const
{
    uint16_t n = 0;
    for (uint16_t k = 0; k < ENABLE_BITS; k++)
        if (_bits[k / 8] & (1 << (k % 8)))
            n++;
    return n;
}

uint8_t GoodWeAssistant::buildResult(uint8_t* out) const
{
    uint8_t i = 0;
    out[i++] = _error;
    out[i++] = _info.family;
    out[i++] = (uint8_t)_protocol;
    out[i++] = (uint8_t)(_port >> 8);
    out[i++] = (uint8_t)_port;
    out[i++] = _address;
    uint8_t flags = 0;
    if (_info.singlePhase)
        flags |= 0x01;
    if (_info.family == FamilyEt && _image.batteryPresent())
        flags |= 0x02;
    if (_image.valid(BlkEtBattery2))
        flags |= 0x04;
    if (_image.meterOk(_info.family))
        flags |= 0x08;
    out[i++] = flags;
    out[i++] = (uint8_t)(_info.ratedPower >> 8);
    out[i++] = (uint8_t)_info.ratedPower;
    out[i++] = (uint8_t)(_info.armVersion >> 8);
    out[i++] = (uint8_t)_info.armVersion;
    out[i++] = BIT_BYTES;
    memcpy(out + i, _bits, BIT_BYTES);
    i += BIT_BYTES;
    memset(out + i, 0, 26);
    memcpy(out + i, _info.model, strnlen(_info.model, 10));
    i += 10;
    memcpy(out + i, _info.serial, strnlen(_info.serial, 16));
    i += 16;
    return i;
}
