#pragma once
#include <stdint.h>

// ERZEUGT von tools/gen_templ.py aus tools/slots.py - nicht von Hand bearbeiten.
//
// Objektkatalog: Herkunft (Register, Datentyp, Teiler) je Geraetefamilie und KO-Kodierung.
// Die Reihenfolge ist die KO-Reihenfolge und die Lage der Aktiv-Bits.

namespace GoodWe
{
    enum ValueType : uint8_t
    {
        TypeNone = 0,
        TypeU16,
        TypeS16,
        TypeU32,
        TypeS32,
        TypeF32,
        TypeU64,
        TypeBH,
        TypeBL,
        TypeTEMP,
        TypeTS,
        TypeBITS22,
        TypeCALC,
        TypeCALCUI,
    };

    enum CalcId : uint8_t
    {
        CalcPvTotal,
        CalcHouse,
        CalcGridInout,
        CalcImport,
        CalcExport,
    };

    enum Encoding : uint8_t
    {
        EncFloat,     // DPT 14.x
        EncFloat2,    // DPT 9.x
        EncEnergyWh,  // DPT 13.010, Wert in kWh -> Wh ganzzahlig
        EncPercent,   // DPT 5.001
        EncU8,        // DPT 5.010
        EncU16,       // DPT 7.001
        EncU32,       // DPT 12.001
        EncDateTime,  // DPT 19.001
        EncBool,      // DPT 1.x
    };

    // Bedingungen fuer die Verfuegbarkeit, Bitmaske (siehe tools/slots.py)
    enum Cond : uint16_t
    {
        CondL2 = 1 << 0,
        CondL3 = 1 << 1,
        CondPV3 = 1 << 2,
        CondPV4 = 1 << 3,
        CondBAT = 1 << 4,
        CondBAT2 = 1 << 5,
        CondMETER = 1 << 6,
        CondMPPT = 1 << 7,
        CondMEXT = 1 << 8,
        CondMEXT2 = 1 << 9,
    };

    // Familienmaske
    static const uint8_t FAM_ET = 1;
    static const uint8_t FAM_DT = 2;

    // Teiler zum Feld "div" einer Quelle
    extern const float DIV[5];

    struct Source
    {
        uint16_t reg;   // Register (bei TypeCalcUI: Spannung)
        uint16_t reg2;  // zweites Register (TypeBITS22: Low-Word, TypeCalcUI: Strom)
        uint8_t type;   // ValueType
        uint8_t div;    // Index in DIV, bei TypeCalc die CalcId
    };

    struct Sensor
    {
        const char* name;
        Source et;
        Source dt;
        uint16_t cond;
        uint8_t encoding;
        uint8_t dptMain;
        uint8_t dptSub;
    };

    struct Setting
    {
        const char* name;
        uint8_t families;   // FAM_ET | FAM_DT
        uint8_t encIn;
        uint8_t dptInMain;
        uint8_t dptInSub;
        uint8_t encOut;     // nur gueltig, wenn hasStatus
        uint8_t dptOutMain;
        uint8_t dptOutSub;
        bool hasStatus;
    };

    // Parameterlayout je Kanal, muss zu tools/gen_templ.py passen
    static const uint16_t ENABLE_OFFSET = 32;
    static const uint16_t SENSOR_COUNT = 274;
    static const uint16_t SENSOR_CAPACITY = 300;
    static const uint16_t SETTING_COUNT = 21;
    static const uint16_t SETTING_KO_BASE = 300;
    static const uint16_t ENABLE_BITS = 350;

    enum SettingId : uint8_t
    {
        SetOperationMode = 0,
        SetEmsMode = 1,
        SetEmsPowerLimit = 2,
        SetExportLimitEnable = 3,
        SetExportLimit = 4,
        SetExportLimitPercent = 5,
        SetDodOnGrid = 6,
        SetDodOffGrid = 7,
        SetSocProtection = 8,
        SetSocUpperLimit = 9,
        SetEcoModePower = 10,
        SetEcoModeSoc = 11,
        SetFastCharging = 12,
        SetFastChargingSoc = 13,
        SetFastChargingPower = 14,
        SetBackupSupply = 15,
        SetDodHolding = 16,
        SetLoadControl = 17,
        SetSyncClock = 18,
        SetStartInverter = 19,
        SetStopInverter = 20,
    };

    extern const Sensor SENSORS[SENSOR_COUNT];
    extern const Setting SETTINGS[SETTING_COUNT];
} // namespace GoodWe
