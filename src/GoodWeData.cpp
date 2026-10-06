// KNX-frei halten: hier darf kein OpenKNX-/knx-Header hinein.
#include "GoodWeData.h"
#include <math.h>
#include <string.h>

namespace GoodWe
{
    // Laengen wie in der Bibliothek, mit zwei Abweichungen: der MPPT-Block wird bis 35365
    // gelesen, damit Scheinleistung L2/L3 (35361/35363, je 2 Register) vollstaendig ist,
    // und der DT-Zaehlerblock bis 30210, damit der Ableitstrom enthalten ist. Lehnt das Geraet
    // die laengere Anfrage ab, gilt die kuerzere der Bibliothek.
    const BlockDef BLOCKS[BLOCK_COUNT] = {
        {35100, 125, 125, FAM_ET},       // BlkEtRuntime
        {36000, METER_EXT2, METER_BASIC, FAM_ET}, // BlkEtMeter (Laenge waehlt der Aufrufer)
        {37000, 24, 24, FAM_ET},         // BlkEtBattery
        {39000, 22, 22, FAM_ET},         // BlkEtBattery2
        {35262, 6, 6, FAM_ET},           // BlkEtBattery2Ext
        {35301, 65, 0x3D, FAM_ET},       // BlkEtMppt
        {47900, 36, 36, FAM_ET},         // BlkEtBms
        {45350, 1, 1, FAM_ET},           // BlkEtBatCapacity
        {30100, 73, 73, FAM_DT},         // BlkDtRuntime
        {30195, 16, 15, FAM_DT},         // BlkDtMeter
    };

    // ---------------------------------------------------------------- Modelltabellen
    // goodwe/model.py: Merkmale werden an Kennbuchstaben in der Seriennummer erkannt.
    namespace
    {
        const char* const SINGLE_PHASE[] = {"DSN", "DST", "NSU", "SSN", "SST", "SSX", "SSY", "MSU", "MST", "PSB", "PSC",
                                            "MSC", "EHB", "EHU", "EHR", "HSB", "ESN", "EMN", "ERN", "EBN", "HLB", "HMB",
                                            "HBB", "SPN", "NAH"};
        const char* const MPPT3[] = {"MSU", "MST", "PSC", "MSC", "25KET", "29K9ET", "25KMT", "GW10K-ET20", "GW12K-ET20", "GW15K-ET20"};
        const char* const MPPT4[] = {"DTB", "KMT", "HSB", "EHB", "GW8K-EHA-G20", "GW10K-EHA-G20"};
        const char* const BAT2[] = {"25KET", "29K9ET"};
        const char* const P745[] = {"ESN", "EBN", "EMN", "SPN", "ERN", "ESC", "HLB", "HMB", "HBB", "EOA",
                                    "ETT", "HTA", "HUB", "AEB", "SPB", "CUB", "EUB", "HEB", "ERB", "BTT",
                                    "ETF", "ARB", "URB", "EBR", "NAH"};

        template <size_t N>
        bool anyTag(const char* serial, const char* const (&tags)[N])
        {
            for (size_t i = 0; i < N; i++)
                if (strstr(serial, tags[i]) != nullptr)
                    return true;
            return false;
        }

        // ASCII-Text aus Registern; Steuerzeichen beenden, Leerzeichen am Ende entfernen.
        void ascii(const uint16_t* regs, uint8_t words, char* out, size_t cap)
        {
            size_t n = 0;
            for (uint8_t i = 0; i < words && n + 1 < cap; i++)
            {
                const uint8_t c[2] = {(uint8_t)(regs[i] >> 8), (uint8_t)regs[i]};
                for (uint8_t j = 0; j < 2 && n + 1 < cap; j++)
                    if (c[j] >= 32 && c[j] < 127)
                        out[n++] = (char)c[j];
            }
            out[n] = 0;
            while (n > 0 && out[n - 1] == ' ')
                out[--n] = 0;
        }
    } // namespace

    bool DeviceInfo::parse(uint8_t fam, const uint16_t* regs, uint16_t count)
    {
        *this = DeviceInfo();
        if (fam == FamilyEt)
        {
            // 35000 Modbus-Version, 35001 Nennleistung, 35002 AC-Ausgang, 35003-35010 SN,
            // 35011-35015 Modell, 35019 ARM-Version
            if (count < 20)
                return false;
            ratedPower = regs[1];
            acOutputType = regs[2];
            ascii(regs + 3, 8, serial, sizeof(serial));
            ascii(regs + 11, 5, model, sizeof(model));
            armVersion = regs[19];
        }
        else if (fam == FamilyDt)
        {
            // 30001.. : SN ab Byte 6 (30004), Modell ab Byte 22 (30012), ARM-Version 30036
            if (count < 36)
                return false;
            ascii(regs + 3, 8, serial, sizeof(serial));
            ascii(regs + 11, 5, model, sizeof(model));
            armVersion = regs[35];
        }
        else
        {
            return false;
        }

        if (serial[0] == 0)
            return false;

        family = fam;
        singlePhase = anyTag(serial, SINGLE_PHASE) || (fam == FamilyEt && acOutputType == 0);
        mppt3 = anyTag(serial, MPPT3);
        mppt4 = anyTag(serial, MPPT4);
        battery2Model = anyTag(serial, BAT2);
        platform745 = anyTag(serial, P745);
        return true;
    }

    bool DeviceInfo::hasPv3() const
    {
        if (family == FamilyEt)
            return mppt4 || ratedPower >= 15000;
        return mppt3 || mppt4;
    }

    bool DeviceInfo::hasPv4() const
    {
        if (family == FamilyEt)
            return mppt4 || ratedPower >= 15000;
        return mppt4;
    }

    // ---------------------------------------------------------------- Registerabbild
    RegisterImage::~RegisterImage()
    {
        for (uint8_t b = 0; b < BLOCK_COUNT; b++)
            delete[] _regs[b];
    }

    void RegisterImage::clear()
    {
        for (uint8_t b = 0; b < BLOCK_COUNT; b++)
            _count[b] = 0;
    }

    void RegisterImage::invalidate(uint8_t block)
    {
        if (block < BLOCK_COUNT)
            _count[block] = 0;
    }

    void RegisterImage::store(uint8_t block, const uint16_t* regs, uint16_t count)
    {
        if (block >= BLOCK_COUNT)
            return;
        if (count > BLOCKS[block].count)
            count = BLOCKS[block].count;
        if (_regs[block] == nullptr)
            _regs[block] = new uint16_t[BLOCKS[block].count];
        memcpy(_regs[block], regs, count * sizeof(uint16_t));
        _count[block] = (uint8_t)count;
    }

    bool RegisterImage::word(uint16_t reg, uint16_t& out) const
    {
        for (uint8_t b = 0; b < BLOCK_COUNT; b++)
        {
            if (_count[b] == 0)
                continue;
            const uint16_t start = BLOCKS[b].start;
            if (reg >= start && reg < start + _count[b])
            {
                out = _regs[b][reg - start];
                return true;
            }
        }
        return false;
    }

    bool RegisterImage::batteryPresent() const
    {
        uint16_t mode = 0;
        return word(35184, mode) && mode != 0 && mode != 0xFFFF;
    }

    bool RegisterImage::meterOk(uint8_t family) const
    {
        uint16_t status = 0;
        return word(family == FamilyDt ? 30209 : 36004, status) && status == 1;
    }

    bool RegisterImage::activePower(uint8_t family, double& out) const
    {
        if (family == FamilyDt)
        {
            uint16_t hi, lo;
            if (!word(30195, hi) || !word(30196, lo))
                return false;
            out = (double)(int32_t)(((uint32_t)hi << 16) | lo);
            return true;
        }
        uint16_t p;
        if (!word(35140, p))
            return false;
        out = (double)(int16_t)p;
        return true;
    }

    bool RegisterImage::pvPower(uint8_t family, uint8_t n, double& out) const
    {
        if (family == FamilyDt)
        {
            const uint16_t ureg = (uint16_t)(30103 + 2 * (n - 1));
            uint16_t u, i;
            if (!word(ureg, u) || !word(ureg + 1, i))
                return false;
            // read_voltage/read_current der Bibliothek: 0xFFFF zaehlt als 0
            const double v = (u == 0xFFFF) ? 0.0 : u / 10.0;
            const double a = (i == 0xFFFF) ? 0.0 : i / 10.0;
            out = round(v * a);
            return true;
        }
        const uint16_t reg = (uint16_t)(35105 + 4 * (n - 1));
        uint16_t hi, lo;
        if (!word(reg, hi) || !word(reg + 1, lo))
            return false;
        const uint32_t raw = ((uint32_t)hi << 16) | lo;
        out = (raw == 0xFFFFFFFFUL) ? 0.0 : (double)raw;
        return true;
    }

    bool RegisterImage::calc(uint8_t id, uint8_t family, double& out) const
    {
        switch (id)
        {
            case CalcPvTotal:
            {
                double sum = 0;
                for (uint8_t n = 1; n <= 4; n++)
                {
                    double p = 0;
                    if (pvPower(family, n, p) && p > 0)
                        sum += p;
                }
                out = sum;
                return valid(family == FamilyDt ? BlkDtRuntime : BlkEtRuntime);
            }

            case CalcHouse:
            {
                if (family == FamilyDt)
                {
                    double pv = 0, meter = 0;
                    if (!calc(CalcPvTotal, family, pv) || !activePower(family, meter))
                        return false;
                    out = fabs(pv - meter);
                    return true;
                }
                // ppv1..4 + pbattery1 - active_power
                double sum = 0;
                for (uint8_t n = 1; n <= 4; n++)
                {
                    double p = 0;
                    if (pvPower(family, n, p))
                        sum += p;
                }
                uint16_t hi, lo;
                if (!word(35182, hi) || !word(35183, lo))
                    return false;
                sum += (double)(int32_t)(((uint32_t)hi << 16) | lo);
                double grid = 0;
                if (!activePower(family, grid))
                    return false;
                out = sum - grid;
                return true;
            }

            case CalcGridInout:
            {
                double p = 0;
                if (!activePower(family, p))
                    return false;
                out = (p < -90) ? 2 : ((p >= 90) ? 1 : 0); // 0 Ruhe, 1 Einspeisung, 2 Bezug
                return true;
            }

            case CalcImport:
            case CalcExport:
            {
                double p = 0;
                if (!activePower(family, p))
                    return false;
                out = (id == CalcImport) ? (p < 0 ? -p : 0) : (p > 0 ? p : 0);
                return true;
            }
        }
        return false;
    }

    bool RegisterImage::decode(const Source& src, uint8_t family, double& out) const
    {
        const float div = (src.type == TypeCalc || src.type == TypeCalcUI) ? 1.0f : DIV[src.div];
        uint16_t w0 = 0, w1 = 0, w2 = 0, w3 = 0;

        switch (src.type)
        {
            case TypeNone:
            case TypeTS:
                return false;

            case TypeCalc:
                return calc(src.div, family, out);

            case TypeCalcUI:
            {
                if (!word(src.reg, w0) || !word(src.reg2, w1))
                    return false;
                const double v = (w0 == 0xFFFF) ? 0.0 : w0 / 10.0;
                const double a = (w1 == 0xFFFF) ? 0.0 : w1 / 10.0;
                out = round(v * a);
                return true;
            }

            case TypeU16:
                if (!word(src.reg, w0) || w0 == 0xFFFF)
                    return false;
                out = w0 / (double)div;
                return true;

            case TypeS16:
                if (!word(src.reg, w0))
                    return false;
                out = (int16_t)w0 / (double)div;
                return true;

            case TypeTEMP:
            {
                if (!word(src.reg, w0))
                    return false;
                const int16_t t = (int16_t)w0;
                if (t == -1 || t == 32767)
                    return false;
                out = t / (double)div;
                return true;
            }

            case TypeBH:
            case TypeBL:
                if (!word(src.reg, w0))
                    return false;
                out = (int8_t)((src.type == TypeBH) ? (w0 >> 8) : (w0 & 0xFF));
                return true;

            case TypeU32:
            case TypeS32:
            case TypeF32:
            {
                if (!word(src.reg, w0) || !word(src.reg + 1, w1))
                    return false;
                const uint32_t raw = ((uint32_t)w0 << 16) | w1; // High-Word zuerst
                if (src.type == TypeU32)
                {
                    if (raw == 0xFFFFFFFFUL)
                        return false;
                    out = raw / (double)div;
                }
                else if (src.type == TypeS32)
                {
                    out = (int32_t)raw / (double)div;
                }
                else
                {
                    float f;
                    memcpy(&f, &raw, sizeof(f));
                    if (isnan(f) || isinf(f))
                        return false;
                    out = f / (double)div;
                }
                return true;
            }

            case TypeU64:
            {
                if (!word(src.reg, w0) || !word(src.reg + 1, w1) || !word(src.reg + 2, w2) || !word(src.reg + 3, w3))
                    return false;
                const uint64_t raw = ((uint64_t)w0 << 48) | ((uint64_t)w1 << 32) | ((uint64_t)w2 << 16) | w3;
                if (raw == 0xFFFFFFFFFFFFFFFFULL)
                    return false;
                out = (double)raw / div;
                return true;
            }

            case TypeBITS22:
                // Bibliothek: High-Word | Low-Word aus zwei Registern
                if (!word(src.reg, w0) || !word(src.reg2, w1))
                    return false;
                out = (double)(((uint32_t)w0 << 16) | w1);
                return true;
        }
        return false;
    }

    bool RegisterImage::decodeTime(const Source& src, struct tm& out) const
    {
        uint16_t w0, w1, w2;
        if (src.type != TypeTS || !word(src.reg, w0) || !word(src.reg + 1, w1) || !word(src.reg + 2, w2))
            return false;
        memset(&out, 0, sizeof(out));
        out.tm_year = 100 + (w0 >> 8); // Jahr ab 2000, tm zaehlt ab 1900
        out.tm_mon = (w0 & 0xFF) - 1;
        out.tm_mday = w1 >> 8;
        out.tm_hour = w1 & 0xFF;
        out.tm_min = w2 >> 8;
        out.tm_sec = w2 & 0xFF;
        if (out.tm_mon < 0 || out.tm_mon > 11 || out.tm_mday < 1 || out.tm_mday > 31 || out.tm_hour > 23 ||
            out.tm_min > 59 || out.tm_sec > 59)
            return false;
        return true;
    }

    // ---------------------------------------------------------------- Zuordnung
    const Source& sourceOf(const Sensor& s, uint8_t family)
    {
        return family == FamilyDt ? s.dt : s.et;
    }

    uint8_t sourceWords(const Source& src)
    {
        switch (src.type)
        {
            case TypeU32:
            case TypeS32:
            case TypeF32:
                return 2;
            case TypeU64:
                return 4;
            case TypeTS:
                return 3;
            default:
                return 1;
        }
    }

    uint8_t blockOf(uint8_t family, uint16_t reg)
    {
        const uint8_t mask = familyMask(family);
        for (uint8_t b = 0; b < BLOCK_COUNT; b++)
        {
            if ((BLOCKS[b].family & mask) == 0)
                continue;
            if (reg >= BLOCKS[b].start && reg < BLOCKS[b].start + BLOCKS[b].count)
                return b;
        }
        return BLOCK_NONE;
    }

    uint16_t blocksOf(const Source& src, uint8_t family, uint16_t cond)
    {
        uint16_t mask = 0;
        const uint8_t runtime = (family == FamilyDt) ? BlkDtRuntime : BlkEtRuntime;
        const uint8_t meter = (family == FamilyDt) ? BlkDtMeter : BlkEtMeter;

        switch (src.type)
        {
            case TypeNone:
                break;
            case TypeCalc:
                mask |= 1 << runtime;
                if (src.div != CalcPvTotal)
                    mask |= 1 << meter; // Netzleistung bzw. Zaehlerstatus
                break;
            case TypeCalcUI:
                mask |= 1 << blockOf(family, src.reg);
                break;
            default:
            {
                const uint8_t b = blockOf(family, src.reg);
                if (b != BLOCK_NONE)
                    mask |= 1 << b;
                if (src.type == TypeBITS22)
                {
                    const uint8_t b2 = blockOf(family, src.reg2);
                    if (b2 != BLOCK_NONE)
                        mask |= 1 << b2;
                }
                break;
            }
        }
        if (cond & CondMETER)
            mask |= 1 << meter;
        if ((cond & CondBAT) && family == FamilyEt)
            mask |= 1 << BlkEtRuntime; // Batteriemodus
        return mask;
    }
} // namespace GoodWe
