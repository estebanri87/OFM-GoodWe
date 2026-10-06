#pragma once
#include "SlotCatalog.h"
#include <stdint.h>
#include <time.h>

// Registerabbild eines Wechselrichters und Dekodierung der Katalogwerte. Gemeinsam genutzt
// von Kanal und Assistent; KNX-frei.
//
// Alle Regeln (Bloecke, Skalierung, Modellmerkmale) stammen aus der Python-Bibliothek
// goodwe 0.4.10 (et.py, dt.py, model.py, sensor.py).
namespace GoodWe
{
    enum Family : uint8_t
    {
        FamilyUnknown = 0,
        FamilyEt = 2, // Werte wie der ETS-Typ: 2 = Hybrid
        FamilyDt = 3, //                         3 = netzgekoppelt
    };

    inline uint8_t familyMask(uint8_t family)
    {
        return family == FamilyEt ? FAM_ET : (family == FamilyDt ? FAM_DT : 0);
    }

    enum BlockId : uint8_t
    {
        BlkEtRuntime = 0,
        BlkEtMeter,
        BlkEtBattery,
        BlkEtBattery2,
        BlkEtBattery2Ext,
        BlkEtMppt,
        BlkEtBms,
        BlkEtBatCapacity,
        BlkDtRuntime,
        BlkDtMeter,
        BLOCK_COUNT,
        BLOCK_NONE = 0xFF,
    };

    struct BlockDef
    {
        uint16_t start;
        uint8_t count;    // regulaere Laenge
        uint8_t fallback; // kuerzere Laenge, falls das Geraet die regulaere ablehnt
        uint8_t family;   // FAM_ET / FAM_DT
    };

    extern const BlockDef BLOCKS[BLOCK_COUNT];

    // Laengen des Zaehlerblocks 36000 (ET): Basis, erweitert (Zaehler 2, U/I je Phase),
    // erweitert 2 (Energie je Phase).
    static const uint8_t METER_BASIC = 0x2D;
    static const uint8_t METER_EXT = 0x3A;
    static const uint8_t METER_EXT2 = 0x7D;

    // Geraeteinformation (ET: 35000/0x21, DT: 30001/0x28)
    static const uint16_t ET_INFO_REG = 35000;
    static const uint8_t ET_INFO_COUNT = 0x21;
    static const uint16_t DT_INFO_REG = 30001;
    static const uint8_t DT_INFO_COUNT = 0x28;

    struct DeviceInfo
    {
        uint8_t family = FamilyUnknown;
        char serial[17] = {0};
        char model[11] = {0};
        uint16_t ratedPower = 0;
        uint16_t acOutputType = 0;
        uint16_t armVersion = 0;
        bool singlePhase = false;
        bool mppt3 = false;
        bool mppt4 = false;
        bool battery2Model = false;
        bool platform745 = false;

        bool parse(uint8_t family, const uint16_t* regs, uint16_t count);

        // Modellregeln wie et.py / dt.py
        bool hasPv3() const;
        bool hasPv4() const;
        bool extendedBlocks() const { return platform745 || ratedPower >= 15000; } // MPPT, Zaehler erweitert
        bool hasBattery2() const { return battery2Model || ratedPower >= 25000; }
    };

    class RegisterImage
    {
      public:
        ~RegisterImage();

        void clear();
        void invalidate(uint8_t block);
        void store(uint8_t block, const uint16_t* regs, uint16_t count);
        bool valid(uint8_t block) const { return block < BLOCK_COUNT && _count[block] != 0; }

        bool word(uint16_t reg, uint16_t& out) const;

        // Dekodiert eine Quelle. false, wenn Register fehlen oder der Wert "undefiniert" ist.
        bool decode(const Source& src, uint8_t family, double& out) const;
        bool decodeTime(const Source& src, struct tm& out) const;

        // Hilfswerte fuer Bedingungen
        bool batteryPresent() const;  // ET: Batteriemodus != 0
        bool meterOk(uint8_t family) const;
        bool activePower(uint8_t family, double& out) const;

      private:
        uint16_t* _regs[BLOCK_COUNT] = {};
        uint8_t _count[BLOCK_COUNT] = {};

        bool calc(uint8_t id, uint8_t family, double& out) const;
        bool pvPower(uint8_t family, uint8_t n, double& out) const;
    };

    // Block, in dem ein Register einer Familie liegt (ohne Laengenpruefung des Zaehlerblocks)
    uint8_t blockOf(uint8_t family, uint16_t reg);
    // Anzahl Register, die ein Wert ab src.reg belegt
    uint8_t sourceWords(const Source& src);
    // Bloecke, die ein Katalogwert benoetigt (Bitmaske ueber BlockId)
    uint16_t blocksOf(const Source& src, uint8_t family, uint16_t cond);

    const Source& sourceOf(const Sensor& s, uint8_t family);
} // namespace GoodWe
