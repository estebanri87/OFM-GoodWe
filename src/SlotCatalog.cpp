// ERZEUGT von tools/gen_templ.py aus tools/slots.py - nicht von Hand bearbeiten.
#include "SlotCatalog.h"

namespace GoodWe
{
    const float DIV[5] = {1.0f, 10.0f, 100.0f, 1000.0f, 10000.0f};

    const Sensor SENSORS[SENSOR_COUNT] = {
        {"Reachable", {0, 0, TypeNone, 0}, {0, 0, TypeNone, 0}, 0x000, EncBool, 1, 11}, // 0
        {"WorkMode", {35187, 0, TypeU16, 0}, {30129, 0, TypeU16, 0}, 0x000, EncU8, 5, 10}, // 1
        {"ErrorCodes", {35189, 0, TypeU32, 0}, {30130, 0, TypeU32, 0}, 0x000, EncU32, 12, 1}, // 2
        {"WarningCode", {35185, 0, TypeU16, 0}, {30132, 0, TypeU16, 0}, 0x000, EncU16, 7, 1}, // 3
        {"SafetyCountry", {35186, 0, TypeU16, 0}, {30149, 0, TypeU16, 0}, 0x000, EncU8, 5, 10}, // 4
        {"FunctionBit", {35177, 0, TypeU16, 0}, {30162, 0, TypeU16, 0}, 0x000, EncU16, 7, 1}, // 5
        {"HoursTotal", {35197, 0, TypeU32, 0}, {30147, 0, TypeU32, 0}, 0x000, EncU32, 12, 1}, // 6
        {"Timestamp", {35100, 0, TypeTS, 0}, {30100, 0, TypeTS, 0}, 0x000, EncDateTime, 19, 1}, // 7
        {"Temperature", {35176, 0, TypeTEMP, 1}, {30141, 0, TypeTEMP, 1}, 0x000, EncFloat2, 9, 1}, // 8
        {"BusVoltage", {35178, 0, TypeU16, 1}, {30163, 0, TypeU16, 1}, 0x000, EncFloat, 14, 27}, // 9
        {"NBusVoltage", {35179, 0, TypeU16, 1}, {30164, 0, TypeU16, 1}, 0x000, EncFloat, 14, 27}, // 10
        {"GridInOut", {0, 0, TypeCalc, CalcGridInout}, {0, 0, TypeCalc, CalcGridInout}, 0x040, EncU8, 5, 10}, // 11
        {"Rssi", {36001, 0, TypeU16, 0}, {30172, 0, TypeU16, 0}, 0x000, EncU16, 7, 1}, // 12
        {"MeterCommStatus", {36004, 0, TypeU16, 0}, {30209, 0, TypeU16, 0}, 0x000, EncU8, 5, 10}, // 13
        {"GridMode", {35136, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x000, EncU8, 5, 10}, // 14
        {"OperationCode", {35188, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x000, EncU8, 5, 10}, // 15
        {"DiagStatus", {35220, 0, TypeU32, 0}, {0, 0, TypeNone, 0}, 0x000, EncU32, 12, 1}, // 16
        {"TempAir", {35174, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x000, EncFloat2, 9, 1}, // 17
        {"TempModule", {35175, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x000, EncFloat2, 9, 1}, // 18
        {"TempHeatsink", {0, 0, TypeNone, 0}, {30142, 0, TypeTEMP, 1}, 0x000, EncFloat2, 9, 1}, // 19
        {"DeratingMode", {0, 0, TypeNone, 0}, {30165, 0, TypeU32, 0}, 0x000, EncU32, 12, 1}, // 20
        {"LeakageCurrent", {0, 0, TypeNone, 0}, {30210, 0, TypeS16, 4}, 0x000, EncFloat, 14, 19}, // 21
        {"PvPower", {0, 0, TypeCalc, CalcPvTotal}, {0, 0, TypeCalc, CalcPvTotal}, 0x000, EncFloat, 14, 56}, // 22
        {"Pv1Voltage", {35103, 0, TypeU16, 1}, {30103, 0, TypeU16, 1}, 0x000, EncFloat, 14, 27}, // 23
        {"Pv1Current", {35104, 0, TypeU16, 1}, {30104, 0, TypeU16, 1}, 0x000, EncFloat, 14, 19}, // 24
        {"Pv1Power", {35105, 0, TypeU32, 0}, {30103, 30104, TypeCalcUI, 1}, 0x000, EncFloat, 14, 56}, // 25
        {"Pv2Voltage", {35107, 0, TypeU16, 1}, {30105, 0, TypeU16, 1}, 0x000, EncFloat, 14, 27}, // 26
        {"Pv2Current", {35108, 0, TypeU16, 1}, {30106, 0, TypeU16, 1}, 0x000, EncFloat, 14, 19}, // 27
        {"Pv2Power", {35109, 0, TypeU32, 0}, {30105, 30106, TypeCalcUI, 1}, 0x000, EncFloat, 14, 56}, // 28
        {"Pv3Voltage", {35111, 0, TypeU16, 1}, {30107, 0, TypeU16, 1}, 0x004, EncFloat, 14, 27}, // 29
        {"Pv3Current", {35112, 0, TypeU16, 1}, {30108, 0, TypeU16, 1}, 0x004, EncFloat, 14, 19}, // 30
        {"Pv3Power", {35113, 0, TypeU32, 0}, {30107, 30108, TypeCalcUI, 1}, 0x004, EncFloat, 14, 56}, // 31
        {"Pv4Voltage", {35115, 0, TypeU16, 1}, {30109, 0, TypeU16, 1}, 0x008, EncFloat, 14, 27}, // 32
        {"Pv4Current", {35116, 0, TypeU16, 1}, {30110, 0, TypeU16, 1}, 0x008, EncFloat, 14, 19}, // 33
        {"Pv4Power", {35117, 0, TypeU32, 0}, {30109, 30110, TypeCalcUI, 1}, 0x008, EncFloat, 14, 56}, // 34
        {"Pv1Mode", {35120, 0, TypeBL, 0}, {0, 0, TypeNone, 0}, 0x000, EncU8, 5, 10}, // 35
        {"Pv2Mode", {35120, 0, TypeBH, 0}, {0, 0, TypeNone, 0}, 0x000, EncU8, 5, 10}, // 36
        {"Pv3Mode", {35119, 0, TypeBL, 0}, {0, 0, TypeNone, 0}, 0x004, EncU8, 5, 10}, // 37
        {"Pv4Mode", {35119, 0, TypeBH, 0}, {0, 0, TypeNone, 0}, 0x008, EncU8, 5, 10}, // 38
        {"TotalInputPower", {0, 0, TypeNone, 0}, {30137, 0, TypeS16, 0}, 0x000, EncFloat, 14, 56}, // 39
        {"PvPowerTotalExt", {35301, 0, TypeU32, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 40
        {"PvChannel", {35303, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncU16, 7, 1}, // 41
        {"Pv5Voltage", {35304, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 42
        {"Pv5Current", {35305, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 43
        {"Pv6Voltage", {35306, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 44
        {"Pv6Current", {35307, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 45
        {"Pv7Voltage", {35308, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 46
        {"Pv7Current", {35309, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 47
        {"Pv8Voltage", {35310, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 48
        {"Pv8Current", {35311, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 49
        {"Pv9Voltage", {35312, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 50
        {"Pv9Current", {35313, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 51
        {"Pv10Voltage", {35314, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 52
        {"Pv10Current", {35315, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 53
        {"Pv11Voltage", {35316, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 54
        {"Pv11Current", {35317, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 55
        {"Pv12Voltage", {35318, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 56
        {"Pv12Current", {35319, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 57
        {"Pv13Voltage", {35320, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 58
        {"Pv13Current", {35321, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 59
        {"Pv14Voltage", {35322, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 60
        {"Pv14Current", {35323, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 61
        {"Pv15Voltage", {35324, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 62
        {"Pv15Current", {35325, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 63
        {"Pv16Voltage", {35326, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 27}, // 64
        {"Pv16Current", {35327, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 65
        {"Mppt1Power", {35337, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 66
        {"Mppt2Power", {35338, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 67
        {"Mppt3Power", {35339, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 68
        {"Mppt4Power", {35340, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 69
        {"Mppt5Power", {35341, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 70
        {"Mppt6Power", {35342, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 71
        {"Mppt7Power", {35343, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 72
        {"Mppt8Power", {35344, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 73
        {"Mppt1Current", {35345, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 74
        {"Mppt2Current", {35346, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 75
        {"Mppt3Current", {35347, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 76
        {"Mppt4Current", {35348, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 77
        {"Mppt5Current", {35349, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 78
        {"Mppt6Current", {35350, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 79
        {"Mppt7Current", {35351, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 80
        {"Mppt8Current", {35352, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 19}, // 81
        {"GridVoltageL1", {35121, 0, TypeU16, 1}, {30118, 0, TypeU16, 1}, 0x000, EncFloat, 14, 27}, // 82
        {"GridCurrentL1", {35122, 0, TypeU16, 1}, {30121, 0, TypeU16, 1}, 0x000, EncFloat, 14, 19}, // 83
        {"GridFrequencyL1", {35123, 0, TypeS16, 2}, {30124, 0, TypeS16, 2}, 0x000, EncFloat, 14, 33}, // 84
        {"GridPowerL1", {35125, 0, TypeS16, 0}, {30118, 30121, TypeCalcUI, 1}, 0x000, EncFloat, 14, 56}, // 85
        {"GridVoltageL2", {35126, 0, TypeU16, 1}, {30119, 0, TypeU16, 1}, 0x001, EncFloat, 14, 27}, // 86
        {"GridCurrentL2", {35127, 0, TypeU16, 1}, {30122, 0, TypeU16, 1}, 0x001, EncFloat, 14, 19}, // 87
        {"GridFrequencyL2", {35128, 0, TypeS16, 2}, {30125, 0, TypeS16, 2}, 0x001, EncFloat, 14, 33}, // 88
        {"GridPowerL2", {35130, 0, TypeS16, 0}, {30119, 30122, TypeCalcUI, 1}, 0x001, EncFloat, 14, 56}, // 89
        {"GridVoltageL3", {35131, 0, TypeU16, 1}, {30120, 0, TypeU16, 1}, 0x002, EncFloat, 14, 27}, // 90
        {"GridCurrentL3", {35132, 0, TypeU16, 1}, {30123, 0, TypeU16, 1}, 0x002, EncFloat, 14, 19}, // 91
        {"GridFrequencyL3", {35133, 0, TypeS16, 2}, {30126, 0, TypeS16, 2}, 0x002, EncFloat, 14, 33}, // 92
        {"GridPowerL3", {35135, 0, TypeS16, 0}, {30120, 30123, TypeCalcUI, 1}, 0x002, EncFloat, 14, 56}, // 93
        {"InverterPower", {35138, 0, TypeS16, 0}, {30127, 0, TypeU32, 0}, 0x000, EncFloat, 14, 56}, // 94
        {"ActivePower", {35140, 0, TypeS16, 0}, {30195, 0, TypeS32, 0}, 0x040, EncFloat, 14, 56}, // 95
        {"ImportPower", {0, 0, TypeCalc, CalcImport}, {0, 0, TypeCalc, CalcImport}, 0x040, EncFloat, 14, 56}, // 96
        {"ExportPower", {0, 0, TypeCalc, CalcExport}, {0, 0, TypeCalc, CalcExport}, 0x040, EncFloat, 14, 56}, // 97
        {"ReactivePower", {35142, 0, TypeS16, 0}, {30135, 0, TypeS32, 0}, 0x000, EncFloat, 14, 56}, // 98
        {"ApparentPower", {35144, 0, TypeS16, 0}, {30133, 0, TypeS32, 0}, 0x000, EncFloat, 14, 56}, // 99
        {"HouseConsumption", {0, 0, TypeCalc, CalcHouse}, {0, 0, TypeCalc, CalcHouse}, 0x040, EncFloat, 14, 56}, // 100
        {"ReactivePowerL1", {35353, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 101
        {"ReactivePowerL2", {35355, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x081, EncFloat, 14, 56}, // 102
        {"ReactivePowerL3", {35357, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x082, EncFloat, 14, 56}, // 103
        {"ApparentPowerL1", {35359, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x080, EncFloat, 14, 56}, // 104
        {"ApparentPowerL2", {35361, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x081, EncFloat, 14, 56}, // 105
        {"ApparentPowerL3", {35363, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x082, EncFloat, 14, 56}, // 106
        {"LineVoltageL1L2", {0, 0, TypeNone, 0}, {30115, 0, TypeU16, 1}, 0x001, EncFloat, 14, 27}, // 107
        {"LineVoltageL2L3", {0, 0, TypeNone, 0}, {30116, 0, TypeU16, 1}, 0x001, EncFloat, 14, 27}, // 108
        {"LineVoltageL3L1", {0, 0, TypeNone, 0}, {30117, 0, TypeU16, 1}, 0x002, EncFloat, 14, 27}, // 109
        {"PowerFactor", {0, 0, TypeNone, 0}, {30139, 0, TypeS16, 3}, 0x000, EncFloat, 14, 57}, // 110
        {"BackupVoltageL1", {35145, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x000, EncFloat, 14, 27}, // 111
        {"BackupCurrentL1", {35146, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x000, EncFloat, 14, 19}, // 112
        {"BackupFrequencyL1", {35147, 0, TypeS16, 2}, {0, 0, TypeNone, 0}, 0x000, EncFloat, 14, 33}, // 113
        {"LoadModeL1", {35148, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x000, EncU8, 5, 10}, // 114
        {"BackupPowerL1", {35150, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x000, EncFloat, 14, 56}, // 115
        {"BackupVoltageL2", {35151, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x001, EncFloat, 14, 27}, // 116
        {"BackupCurrentL2", {35152, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x001, EncFloat, 14, 19}, // 117
        {"BackupFrequencyL2", {35153, 0, TypeS16, 2}, {0, 0, TypeNone, 0}, 0x001, EncFloat, 14, 33}, // 118
        {"LoadModeL2", {35154, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x001, EncU8, 5, 10}, // 119
        {"BackupPowerL2", {35156, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x001, EncFloat, 14, 56}, // 120
        {"BackupVoltageL3", {35157, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x002, EncFloat, 14, 27}, // 121
        {"BackupCurrentL3", {35158, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x002, EncFloat, 14, 19}, // 122
        {"BackupFrequencyL3", {35159, 0, TypeS16, 2}, {0, 0, TypeNone, 0}, 0x002, EncFloat, 14, 33}, // 123
        {"LoadModeL3", {35160, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x002, EncU8, 5, 10}, // 124
        {"BackupPowerL3", {35162, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x002, EncFloat, 14, 56}, // 125
        {"LoadPowerL1", {35164, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x000, EncFloat, 14, 56}, // 126
        {"LoadPowerL2", {35166, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x001, EncFloat, 14, 56}, // 127
        {"LoadPowerL3", {35168, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x002, EncFloat, 14, 56}, // 128
        {"BackupPowerTotal", {35170, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x000, EncFloat, 14, 56}, // 129
        {"LoadPowerTotal", {35172, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x000, EncFloat, 14, 56}, // 130
        {"UpsLoad", {35173, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x000, EncPercent, 5, 1}, // 131
        {"BatteryVoltage", {35180, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 132
        {"BatteryCurrent", {35181, 0, TypeS16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 133
        {"BatteryPower", {35182, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 56}, // 134
        {"BatteryMode", {35184, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU8, 5, 10}, // 135
        {"BatterySoc", {37007, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncPercent, 5, 1}, // 136
        {"BatterySoh", {37008, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncPercent, 5, 1}, // 137
        {"BatteryTemperature", {37003, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat2, 9, 1}, // 138
        {"BatteryChargeLimit", {37004, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 139
        {"BatteryDischargeLimit", {37005, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 140
        {"BatteryBms", {37000, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 141
        {"BatteryIndex", {37001, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 142
        {"BatteryStatus", {37002, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 143
        {"BatteryModules", {37009, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 144
        {"BatteryProtocol", {37011, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 145
        {"BatteryError", {37012, 37006, TypeBITS22, 0}, {0, 0, TypeNone, 0}, 0x010, EncU32, 12, 1}, // 146
        {"BatteryWarning", {37013, 37010, TypeBITS22, 0}, {0, 0, TypeNone, 0}, 0x010, EncU32, 12, 1}, // 147
        {"BatterySwVersion", {37014, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 148
        {"BatteryHwVersion", {37015, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 149
        {"BatteryMaxCellTempId", {37016, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 150
        {"BatteryMinCellTempId", {37017, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 151
        {"BatteryMaxCellVoltId", {37018, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 152
        {"BatteryMinCellVoltId", {37019, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 153
        {"BatteryMaxCellTemp", {37020, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat2, 9, 1}, // 154
        {"BatteryMinCellTemp", {37021, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat2, 9, 1}, // 155
        {"BatteryMaxCellVoltage", {37022, 0, TypeU16, 3}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 156
        {"BatteryMinCellVoltage", {37023, 0, TypeU16, 3}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 157
        {"BatteryCapacity", {45350, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 158
        {"Battery2Voltage", {35262, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x020, EncFloat, 14, 27}, // 159
        {"Battery2Current", {35263, 0, TypeS16, 1}, {0, 0, TypeNone, 0}, 0x020, EncFloat, 14, 19}, // 160
        {"Battery2Power", {35264, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x020, EncFloat, 14, 56}, // 161
        {"Battery2Mode", {35266, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU8, 5, 10}, // 162
        {"Battery2Status", {39000, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 163
        {"Battery2Temperature", {39001, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x020, EncFloat2, 9, 1}, // 164
        {"Battery2ChargeLimit", {39002, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncFloat, 14, 19}, // 165
        {"Battery2DischargeLimit", {39003, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncFloat, 14, 19}, // 166
        {"Battery2Soc", {39005, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncPercent, 5, 1}, // 167
        {"Battery2Soh", {39006, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncPercent, 5, 1}, // 168
        {"Battery2Modules", {39007, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 169
        {"Battery2Protocol", {39009, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 170
        {"Battery2Error", {39010, 39004, TypeBITS22, 0}, {0, 0, TypeNone, 0}, 0x020, EncU32, 12, 1}, // 171
        {"Battery2Warning", {39011, 39008, TypeBITS22, 0}, {0, 0, TypeNone, 0}, 0x020, EncU32, 12, 1}, // 172
        {"Battery2SwVersion", {39012, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 173
        {"Battery2HwVersion", {39013, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 174
        {"Battery2MaxCellTempId", {39014, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 175
        {"Battery2MinCellTempId", {39015, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 176
        {"Battery2MaxCellVoltId", {39016, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 177
        {"Battery2MinCellVoltId", {39017, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x020, EncU16, 7, 1}, // 178
        {"Battery2MaxCellTemp", {39018, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x020, EncFloat2, 9, 1}, // 179
        {"Battery2MinCellTemp", {39019, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x020, EncFloat2, 9, 1}, // 180
        {"Battery2MaxCellVoltage", {39020, 0, TypeU16, 3}, {0, 0, TypeNone, 0}, 0x020, EncFloat, 14, 27}, // 181
        {"Battery2MinCellVoltage", {39021, 0, TypeU16, 3}, {0, 0, TypeNone, 0}, 0x020, EncFloat, 14, 27}, // 182
        {"EnergyTotal", {35191, 0, TypeU32, 1}, {30145, 0, TypeU32, 1}, 0x000, EncEnergyWh, 13, 10}, // 183
        {"EnergyToday", {35193, 0, TypeU32, 1}, {30144, 0, TypeU16, 1}, 0x000, EncEnergyWh, 13, 10}, // 184
        {"MeterExportTotal", {36015, 0, TypeF32, 3}, {30197, 0, TypeU32, 3}, 0x040, EncEnergyWh, 13, 10}, // 185
        {"MeterImportTotal", {36017, 0, TypeF32, 3}, {30199, 0, TypeU32, 3}, 0x040, EncEnergyWh, 13, 10}, // 186
        {"ExportTotal", {35195, 0, TypeU32, 1}, {0, 0, TypeNone, 0}, 0x000, EncEnergyWh, 13, 10}, // 187
        {"ExportToday", {35199, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x000, EncEnergyWh, 13, 10}, // 188
        {"ImportTotal", {35200, 0, TypeU32, 1}, {0, 0, TypeNone, 0}, 0x000, EncEnergyWh, 13, 10}, // 189
        {"ImportToday", {35202, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x000, EncEnergyWh, 13, 10}, // 190
        {"LoadTotal", {35203, 0, TypeU32, 1}, {0, 0, TypeNone, 0}, 0x000, EncEnergyWh, 13, 10}, // 191
        {"LoadToday", {35205, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x000, EncEnergyWh, 13, 10}, // 192
        {"BatteryChargeTotal", {35206, 0, TypeU32, 1}, {0, 0, TypeNone, 0}, 0x010, EncEnergyWh, 13, 10}, // 193
        {"BatteryChargeToday", {35208, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncEnergyWh, 13, 10}, // 194
        {"BatteryDischargeTotal", {35209, 0, TypeU32, 1}, {0, 0, TypeNone, 0}, 0x010, EncEnergyWh, 13, 10}, // 195
        {"BatteryDischargeToday", {35211, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncEnergyWh, 13, 10}, // 196
        {"MeterCommode", {36000, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x040, EncU16, 7, 1}, // 197
        {"MeterManufacturer", {36002, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x040, EncU16, 7, 1}, // 198
        {"MeterTestStatus", {36003, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x040, EncU8, 5, 10}, // 199
        {"MeterTypeCode", {36043, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x040, EncU8, 5, 10}, // 200
        {"MeterSwVersion", {36044, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x040, EncU16, 7, 1}, // 201
        {"MeterPowerL1", {36019, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 202
        {"MeterPowerL2", {36021, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x041, EncFloat, 14, 56}, // 203
        {"MeterPowerL3", {36023, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x042, EncFloat, 14, 56}, // 204
        {"MeterPowerTotal", {36025, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 205
        {"MeterPower16L1", {36005, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 206
        {"MeterPower16L2", {36006, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x041, EncFloat, 14, 56}, // 207
        {"MeterPower16L3", {36007, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x042, EncFloat, 14, 56}, // 208
        {"MeterPower16Total", {36008, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 209
        {"MeterReactiveL1", {36027, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 210
        {"MeterReactiveL2", {36029, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x041, EncFloat, 14, 56}, // 211
        {"MeterReactiveL3", {36031, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x042, EncFloat, 14, 56}, // 212
        {"MeterReactiveTotal", {36033, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 213
        {"MeterReactive16Total", {36009, 0, TypeS16, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 214
        {"MeterApparentL1", {36035, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 215
        {"MeterApparentL2", {36037, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x041, EncFloat, 14, 56}, // 216
        {"MeterApparentL3", {36039, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x042, EncFloat, 14, 56}, // 217
        {"MeterApparentTotal", {36041, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 56}, // 218
        {"MeterPowerFactorL1", {36010, 0, TypeS16, 3}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 57}, // 219
        {"MeterPowerFactorL2", {36011, 0, TypeS16, 3}, {0, 0, TypeNone, 0}, 0x041, EncFloat, 14, 57}, // 220
        {"MeterPowerFactorL3", {36012, 0, TypeS16, 3}, {0, 0, TypeNone, 0}, 0x042, EncFloat, 14, 57}, // 221
        {"MeterPowerFactor", {36013, 0, TypeS16, 3}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 57}, // 222
        {"MeterFrequency", {36014, 0, TypeS16, 2}, {0, 0, TypeNone, 0}, 0x040, EncFloat, 14, 33}, // 223
        {"MeterVoltageL1", {36052, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x140, EncFloat, 14, 27}, // 224
        {"MeterVoltageL2", {36053, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x141, EncFloat, 14, 27}, // 225
        {"MeterVoltageL3", {36054, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x142, EncFloat, 14, 27}, // 226
        {"MeterCurrentL1", {36055, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x140, EncFloat, 14, 19}, // 227
        {"MeterCurrentL2", {36056, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x141, EncFloat, 14, 19}, // 228
        {"MeterCurrentL3", {36057, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x142, EncFloat, 14, 19}, // 229
        {"Meter2Power", {36045, 0, TypeS32, 0}, {0, 0, TypeNone, 0}, 0x100, EncFloat, 14, 56}, // 230
        {"Meter2ExportTotal", {36047, 0, TypeF32, 3}, {0, 0, TypeNone, 0}, 0x100, EncEnergyWh, 13, 10}, // 231
        {"Meter2ImportTotal", {36049, 0, TypeF32, 3}, {0, 0, TypeNone, 0}, 0x100, EncEnergyWh, 13, 10}, // 232
        {"Meter2CommStatus", {36051, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x100, EncU8, 5, 10}, // 233
        {"MeterExportL1", {36092, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x240, EncEnergyWh, 13, 10}, // 234
        {"MeterExportL2", {36096, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x241, EncEnergyWh, 13, 10}, // 235
        {"MeterExportL3", {36100, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x242, EncEnergyWh, 13, 10}, // 236
        {"MeterExportTotal64", {36104, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x240, EncEnergyWh, 13, 10}, // 237
        {"MeterImportL1", {36108, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x240, EncEnergyWh, 13, 10}, // 238
        {"MeterImportL2", {36112, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x241, EncEnergyWh, 13, 10}, // 239
        {"MeterImportL3", {36116, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x242, EncEnergyWh, 13, 10}, // 240
        {"MeterImportTotal64", {36120, 0, TypeU64, 2}, {0, 0, TypeNone, 0}, 0x240, EncEnergyWh, 13, 10}, // 241
        {"Bms1Version", {47900, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 242
        {"Bms1Modules", {47901, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 243
        {"Bms1ChargeVoltageMax", {47902, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 244
        {"Bms1ChargeCurrentMax", {47903, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 245
        {"Bms1DischargeVoltageMin", {47904, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 246
        {"Bms1DischargeCurrentMax", {47905, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 247
        {"Bms1Voltage", {47906, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 248
        {"Bms1Current", {47907, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 249
        {"Bms1Soc", {47908, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncPercent, 5, 1}, // 250
        {"Bms1Soh", {47909, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncPercent, 5, 1}, // 251
        {"Bms1Temperature", {47910, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat2, 9, 1}, // 252
        {"Bms1WarningCode", {47911, 0, TypeU32, 0}, {0, 0, TypeNone, 0}, 0x010, EncU32, 12, 1}, // 253
        {"Bms1AlarmCode", {47913, 0, TypeU32, 0}, {0, 0, TypeNone, 0}, 0x010, EncU32, 12, 1}, // 254
        {"Bms1Status", {47915, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 255
        {"Bms1CommLossDisable", {47916, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 256
        {"Bms1StringRateVoltage", {47917, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 257
        {"Bms2Version", {47918, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 258
        {"Bms2Modules", {47919, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 259
        {"Bms2ChargeVoltageMax", {47920, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 260
        {"Bms2ChargeCurrentMax", {47921, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 261
        {"Bms2DischargeVoltageMin", {47922, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 262
        {"Bms2DischargeCurrentMax", {47923, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 263
        {"Bms2Voltage", {47924, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 27}, // 264
        {"Bms2Current", {47925, 0, TypeU16, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat, 14, 19}, // 265
        {"Bms2Soc", {47926, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncPercent, 5, 1}, // 266
        {"Bms2Soh", {47927, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncPercent, 5, 1}, // 267
        {"Bms2Temperature", {47928, 0, TypeTEMP, 1}, {0, 0, TypeNone, 0}, 0x010, EncFloat2, 9, 1}, // 268
        {"Bms2WarningCode", {47929, 0, TypeU32, 0}, {0, 0, TypeNone, 0}, 0x010, EncU32, 12, 1}, // 269
        {"Bms2AlarmCode", {47931, 0, TypeU32, 0}, {0, 0, TypeNone, 0}, 0x010, EncU32, 12, 1}, // 270
        {"Bms2Status", {47933, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 271
        {"Bms2CommLossDisable", {47934, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 272
        {"Bms2StringRateVoltage", {47935, 0, TypeU16, 0}, {0, 0, TypeNone, 0}, 0x010, EncU16, 7, 1}, // 273
    };

    const Setting SETTINGS[SETTING_COUNT] = {
        {"OperationMode", 1, EncU8, 5, 10, EncU8, 5, 10, true},
        {"EmsMode", 1, EncU8, 5, 10, EncU8, 5, 10, true},
        {"EmsPowerLimit", 1, EncFloat, 14, 56, EncFloat, 14, 56, true},
        {"ExportLimitEnable", 3, EncBool, 1, 1, EncBool, 1, 1, true},
        {"ExportLimit", 3, EncFloat, 14, 56, EncFloat, 14, 56, true},
        {"ExportLimitPercent", 2, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"DodOnGrid", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"DodOffGrid", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"SocProtection", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"SocUpperLimit", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"EcoModePower", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"EcoModeSoc", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"FastCharging", 1, EncBool, 1, 1, EncBool, 1, 1, true},
        {"FastChargingSoc", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"FastChargingPower", 1, EncPercent, 5, 1, EncPercent, 5, 1, true},
        {"BackupSupply", 1, EncBool, 1, 1, EncBool, 1, 1, true},
        {"DodHolding", 1, EncBool, 1, 1, EncBool, 1, 1, true},
        {"LoadControl", 1, EncBool, 1, 1, EncBool, 1, 1, true},
        {"SyncClock", 3, EncBool, 1, 17, EncBool, 0, 0, false},
        {"StartInverter", 2, EncBool, 1, 17, EncBool, 0, 0, false},
        {"StopInverter", 2, EncBool, 1, 17, EncBool, 0, 0, false},
    };
} // namespace GoodWe
