# -*- coding: utf-8 -*-
"""Objekt-Katalog des Moduls: alle Messwerte und Einstellungen eines GoodWe-Wechselrichters.

ACHTUNG - das ist eine Schnittstelle, keine Liste.
Die Reihenfolge bestimmt KO-Nummern und die Lage der Aktiv-Bits im Parameterspeicher.
Ein Eintrag darf hinten angehaengt oder umbenannt werden; ihn zu verschieben oder zu loeschen
verschiebt alle folgenden KOs und macht bestehende ETS-Projekte kaputt.

Quelle ist die Python-Bibliothek goodwe 0.4.10 (et.py / dt.py), die sowohl die offizielle als
auch die inoffizielle Home-Assistant-Integration verwenden. Dubletten der Bibliothek
(*_label, errors/error_codes, battery_error_l/_h) sind zusammengefasst. Gleichbedeutende Werte
von ET und DT teilen sich ein Objekt.

Messwert:  (Name, Anzeigetext, Einheit, DPT, Familie, Quelle ET, Quelle DT, Bedingung)
  Familie   "C" = beide, "E" = nur Hybrid (ET-Familie), "D" = nur netzgekoppelt (DT-Familie)
  Quelle    None, R(Register, Typ, Teiler[, Register2]) oder CALC(Kennung[, Reg, Reg2])
  Bedingung Einschraenkung fuer den Assistenten (siehe COND)
"""

# ------------------------------------------------------------------ Rohdatentypen
# Muss zu GoodWe::ValueType in SlotCatalog.h passen (wird dort erzeugt).
TYPES = ["U16", "S16", "U32", "S32", "F32", "U64", "BH", "BL", "TEMP", "TS", "BITS22", "CALC", "CALCUI"]
# Teiler, Index wird gespeichert
DIVS = [1, 10, 100, 1000, 10000]

# Berechnete Werte
CALCS = ["PV_TOTAL", "HOUSE", "GRID_INOUT", "IMPORT", "EXPORT"]

# Bedingungen fuer die Verfuegbarkeit (Assistent und Firmware), mit "|" kombinierbar.
# Bit n der Bedingungsmaske entspricht COND[n]. Regeln wie in goodwe/et.py und dt.py.
#   L2 / L3   nur bei dreiphasigen Geraeten
#   PV3 / PV4 nur bei Geraeten mit 3 bzw. 4 MPP-Trackern
#   BAT       nur mit Batterie (Batteriemodus 35184 != 0)
#   BAT2      nur mit zweiter Batterie (25/29.9 kW ET bzw. ab 25 kW)
#   METER     nur mit GoodWe-Zaehler (Kommunikationsstatus OK)
#   MPPT      MPPT-Block, nur 745-Plattform bzw. ab 15 kW
#   MEXT      erweiterter Zaehlerblock (36045-36057), nur 745-Plattform bzw. ab 15 kW
#   MEXT2     Zaehlerenergie je Phase (36092-36123), nur 745-Plattform bzw. ab 15 kW
COND = ["L2", "L3", "PV3", "PV4", "BAT", "BAT2", "METER", "MPPT", "MEXT", "MEXT2"]


def cond_mask(cond):
    mask = 0
    for c in filter(None, cond.split("|")):
        assert c in COND, c
        mask |= 1 << COND.index(c)
    return mask


def R(reg, typ="U16", div=1, reg2=0):
    assert typ in TYPES and div in DIVS
    return (reg, typ, div, reg2)


def CALC(cid, reg=0, reg2=0):
    assert cid in CALCS
    return (reg, "CALC", CALCS.index(cid), reg2)


def UI(ureg, ireg):
    """Leistung als Produkt aus Spannung und Strom (DT liefert keine Strangleistung)."""
    return (ureg, "CALCUI", 1, ireg)


# DPT-Kurzformen: (DPT, Objektgroesse)
P = ("DPST-14-56", "4 Bytes")    # Leistung W
U = ("DPST-14-27", "4 Bytes")    # Spannung V
I = ("DPST-14-19", "4 Bytes")    # Strom A
F = ("DPST-14-33", "4 Bytes")    # Frequenz Hz
PF = ("DPST-14-57", "4 Bytes")   # Leistungsfaktor
E = ("DPST-13-10", "4 Bytes")    # Energie Wh
T = ("DPST-9-1", "2 Bytes")      # Temperatur
PCT = ("DPST-5-1", "1 Byte")     # Prozent
MODE = ("DPST-5-10", "1 Byte")   # Zustand / Modus
CNT = ("DPST-7-1", "2 Bytes")    # Zaehler / Kennung
BITS = ("DPST-12-1", "4 Bytes")  # Bitfeld / 32-Bit-Wert
TS = ("DPST-19-1", "8 Bytes")    # Datum / Uhrzeit
SW = ("DPST-1-1", "1 Bit")
TRIG = ("DPST-1-17", "1 Bit")
STATE = ("DPST-1-11", "1 Bit")


def S(name, label, unit, dpt, fam, et, dt, cond=""):
    assert fam in ("C", "E", "D")
    cond_mask(cond)
    if fam == "E":
        assert dt is None and et is not None, name
    if fam == "D":
        assert et is None and dt is not None, name
    return (name, label, unit, dpt, fam, et, dt, cond)


# ================================================================== Messwerte
# Slot 0 ist das Statusobjekt "Erreichbar"; es hat keine Registerquelle.
STATUS = [
    S("Reachable",      "Erreichbar",                  "",    STATE, "C", None, None),
    S("WorkMode",       "Arbeitsmodus",                "",    MODE, "C", R(35187), R(30129)),
    S("ErrorCodes",     "Fehlercode",                  "",    BITS, "C", R(35189, "U32"), R(30130, "U32")),
    S("WarningCode",    "Warnungscode",                "",    CNT,  "C", R(35185), R(30132)),
    S("SafetyCountry",  "Ländereinstellung",           "",    MODE, "C", R(35186), R(30149)),
    S("FunctionBit",    "Funktionsbits",               "",    CNT,  "C", R(35177), R(30162)),
    S("HoursTotal",     "Betriebsstunden",             "h",   BITS, "C", R(35197, "U32"), R(30147, "U32")),
    S("Timestamp",      "Gerätezeit",                  "",    TS,   "C", R(35100, "TS"), R(30100, "TS")),
    S("Temperature",    "Temperatur Wechselrichter",   "°C",  T,    "C", R(35176, "TEMP", 10), R(30141, "TEMP", 10)),
    S("BusVoltage",     "Busspannung",                 "V",   U,    "C", R(35178, "U16", 10), R(30163, "U16", 10)),
    S("NBusVoltage",    "N-Busspannung",               "V",   U,    "C", R(35179, "U16", 10), R(30164, "U16", 10)),
    S("GridInOut",      "Netzrichtung",                "",    MODE, "C", CALC("GRID_INOUT"), CALC("GRID_INOUT"), "METER"),
    S("Rssi",           "Signalstärke",                "",    CNT,  "C", R(36001), R(30172)),
    S("MeterCommStatus", "Zähler Kommunikationsstatus", "",   MODE, "C", R(36004), R(30209)),
]

STATUS_ET = [
    S("GridMode",       "Netzstatus",                  "",    MODE, "E", R(35136), None),
    S("OperationCode",  "Betriebsart",                 "",    MODE, "E", R(35188), None),
    S("DiagStatus",     "Diagnosestatus",              "",    BITS, "E", R(35220, "U32"), None),
    S("TempAir",        "Temperatur Luft",             "°C",  T,    "E", R(35174, "TEMP", 10), None),
    S("TempModule",     "Temperatur Modul",            "°C",  T,    "E", R(35175, "TEMP", 10), None),
]

STATUS_DT = [
    S("TempHeatsink",   "Temperatur Kühlkörper",       "°C",  T,    "D", None, R(30142, "TEMP", 10)),
    S("DeratingMode",   "Leistungsreduzierung",        "",    BITS, "D", None, R(30165, "U32")),
    S("LeakageCurrent", "Ableitstrom",                 "A",   I,    "D", None, R(30210, "S16", 10000)),
]

PV = [S("PvPower", "PV-Leistung gesamt", "W", P, "C", CALC("PV_TOTAL"), CALC("PV_TOTAL"))]
_et_pv = {1: 35103, 2: 35107, 3: 35111, 4: 35115}
_dt_pv = {1: 30103, 2: 30105, 3: 30107, 4: 30109}
for _n in (1, 2, 3, 4):
    _c = "PV%d" % _n if _n >= 3 else ""
    PV += [
        S("Pv%dVoltage" % _n, "PV%d Spannung" % _n, "V", U, "C", R(_et_pv[_n], "U16", 10), R(_dt_pv[_n], "U16", 10), _c),
        S("Pv%dCurrent" % _n, "PV%d Strom" % _n,    "A", I, "C", R(_et_pv[_n] + 1, "U16", 10), R(_dt_pv[_n] + 1, "U16", 10), _c),
        S("Pv%dPower" % _n,   "PV%d Leistung" % _n, "W", P, "C", R(_et_pv[_n] + 2, "U32"), UI(_dt_pv[_n], _dt_pv[_n] + 1), _c),
    ]

PV_ET = [
    S("Pv1Mode", "PV1 Modus", "", MODE, "E", R(35120, "BL"), None),
    S("Pv2Mode", "PV2 Modus", "", MODE, "E", R(35120, "BH"), None),
    S("Pv3Mode", "PV3 Modus", "", MODE, "E", R(35119, "BL"), None, "PV3"),
    S("Pv4Mode", "PV4 Modus", "", MODE, "E", R(35119, "BH"), None, "PV4"),
]

PV_DT = [
    S("TotalInputPower", "Eingangsleistung gesamt", "W", P, "D", None, R(30137, "S16")),
]

# Weitere MPP-Tracker (Geraete der 745-Plattform bzw. ab 15 kW)
PV_EXT = [
    S("PvPowerTotalExt", "PV-Leistung gesamt (MPPT-Block)", "W", P, "E", R(35301, "U32"), None, "MPPT"),
    S("PvChannel",       "Anzahl PV-Kanäle",               "",  CNT, "E", R(35303), None, "MPPT"),
]
for _n in range(5, 17):
    _r = 35304 + (_n - 5) * 2
    PV_EXT += [
        S("Pv%dVoltage" % _n, "PV%d Spannung" % _n, "V", U, "E", R(_r, "U16", 10), None, "MPPT"),
        S("Pv%dCurrent" % _n, "PV%d Strom" % _n,    "A", I, "E", R(_r + 1, "U16", 10), None, "MPPT"),
    ]
for _n in range(1, 9):
    PV_EXT.append(S("Mppt%dPower" % _n, "MPPT%d Leistung" % _n, "W", P, "E", R(35336 + _n), None, "MPPT"))
for _n in range(1, 9):
    PV_EXT.append(S("Mppt%dCurrent" % _n, "MPPT%d Strom" % _n, "A", I, "E", R(35344 + _n, "U16", 10), None, "MPPT"))

GRID = []
_et_grid = {1: 35121, 2: 35126, 3: 35131}
_dt_u = {1: 30118, 2: 30119, 3: 30120}
_dt_i = {1: 30121, 2: 30122, 3: 30123}
_dt_f = {1: 30124, 2: 30125, 3: 30126}
for _n in (1, 2, 3):
    _c = "L%d" % _n if _n > 1 else ""
    GRID += [
        S("GridVoltageL%d" % _n,   "Netzspannung L%d" % _n, "V",  U, "C", R(_et_grid[_n], "U16", 10), R(_dt_u[_n], "U16", 10), _c),
        S("GridCurrentL%d" % _n,   "Netzstrom L%d" % _n,    "A",  I, "C", R(_et_grid[_n] + 1, "U16", 10), R(_dt_i[_n], "U16", 10), _c),
        S("GridFrequencyL%d" % _n, "Netzfrequenz L%d" % _n, "Hz", F, "C", R(_et_grid[_n] + 2, "S16", 100), R(_dt_f[_n], "S16", 100), _c),
        S("GridPowerL%d" % _n,     "Leistung L%d" % _n,     "W",  P, "C", R(_et_grid[_n] + 4, "S16"), UI(_dt_u[_n], _dt_i[_n]), _c),
    ]
GRID += [
    S("InverterPower",    "Wechselrichterleistung",          "W",   P, "C", R(35138, "S16"), R(30127, "U32")),
    S("ActivePower",      "Netzleistung (+ Einspeisung)",    "W",   P, "C", R(35140, "S16"), R(30195, "S32"), "METER"),
    S("ImportPower",      "Netzbezug Leistung",              "W",   P, "C", CALC("IMPORT"), CALC("IMPORT"), "METER"),
    S("ExportPower",      "Einspeisung Leistung",            "W",   P, "C", CALC("EXPORT"), CALC("EXPORT"), "METER"),
    S("ReactivePower",    "Blindleistung (var)",             "var", P, "C", R(35142, "S16"), R(30135, "S32")),
    S("ApparentPower",    "Scheinleistung (VA)",             "VA",  P, "C", R(35144, "S16"), R(30133, "S32")),
    S("HouseConsumption", "Hausverbrauch",                   "W",   P, "C", CALC("HOUSE"), CALC("HOUSE"), "METER"),
]

GRID_ET = []
for _n in (1, 2, 3):
    GRID_ET.append(S("ReactivePowerL%d" % _n, "Blindleistung L%d (var)" % _n, "var", P, "E", R(35351 + 2 * _n, "S32"), None, "MPPT|L%d" % _n if _n > 1 else "MPPT"))
for _n in (1, 2, 3):
    GRID_ET.append(S("ApparentPowerL%d" % _n, "Scheinleistung L%d (VA)" % _n, "VA", P, "E", R(35357 + 2 * _n, "S32"), None, "MPPT|L%d" % _n if _n > 1 else "MPPT"))

GRID_DT = [
    S("LineVoltageL1L2", "Außenleiterspannung L1-L2", "V", U, "D", None, R(30115, "U16", 10), "L2"),
    S("LineVoltageL2L3", "Außenleiterspannung L2-L3", "V", U, "D", None, R(30116, "U16", 10), "L2"),
    S("LineVoltageL3L1", "Außenleiterspannung L3-L1", "V", U, "D", None, R(30117, "U16", 10), "L3"),
    S("PowerFactor",     "Leistungsfaktor",           "",  PF, "D", None, R(30139, "S16", 1000)),
]

BACKUP = []
for _n in (1, 2, 3):
    _r = 35145 + (_n - 1) * 6
    _c = "L%d" % _n if _n > 1 else ""
    BACKUP += [
        S("BackupVoltageL%d" % _n,   "Backup L%d Spannung" % _n,  "V",  U,    "E", R(_r, "U16", 10), None, _c),
        S("BackupCurrentL%d" % _n,   "Backup L%d Strom" % _n,     "A",  I,    "E", R(_r + 1, "U16", 10), None, _c),
        S("BackupFrequencyL%d" % _n, "Backup L%d Frequenz" % _n,  "Hz", F,    "E", R(_r + 2, "S16", 100), None, _c),
        S("LoadModeL%d" % _n,        "Lastmodus L%d" % _n,        "",   MODE, "E", R(_r + 3), None, _c),
        S("BackupPowerL%d" % _n,     "Backup L%d Leistung" % _n,  "W",  P,    "E", R(_r + 5, "S16"), None, _c),
    ]
BACKUP += [
    S("LoadPowerL1",      "Last L1",                "W", P,   "E", R(35164, "S16"), None),
    S("LoadPowerL2",      "Last L2",                "W", P,   "E", R(35166, "S16"), None, "L2"),
    S("LoadPowerL3",      "Last L3",                "W", P,   "E", R(35168, "S16"), None, "L3"),
    S("BackupPowerTotal", "Backup-Last gesamt",     "W", P,   "E", R(35170, "S16"), None),
    S("LoadPowerTotal",   "Last gesamt",            "W", P,   "E", R(35172, "S16"), None),
    S("UpsLoad",          "USV-Auslastung",         "%", PCT, "E", R(35173), None),
]

BATTERY = [
    S("BatteryVoltage",      "Batterie Spannung",                "V",  U,    "E", R(35180, "U16", 10), None, "BAT"),
    S("BatteryCurrent",      "Batterie Strom",                   "A",  I,    "E", R(35181, "S16", 10), None, "BAT"),
    S("BatteryPower",        "Batterie Leistung (+ Entladen)",   "W",  P,    "E", R(35182, "S32"), None, "BAT"),
    S("BatteryMode",         "Batterie Modus",                   "",   MODE, "E", R(35184), None, "BAT"),
    S("BatterySoc",          "Batterie Ladezustand",             "%",  PCT,  "E", R(37007), None, "BAT"),
    S("BatterySoh",          "Batterie Gesundheitszustand",      "%",  PCT,  "E", R(37008), None, "BAT"),
    S("BatteryTemperature",  "Batterie Temperatur",              "°C", T,    "E", R(37003, "TEMP", 10), None, "BAT"),
    S("BatteryChargeLimit",  "Batterie Ladestromgrenze",         "A",  I,    "E", R(37004), None, "BAT"),
    S("BatteryDischargeLimit", "Batterie Entladestromgrenze",    "A",  I,    "E", R(37005), None, "BAT"),
    S("BatteryBms",          "Batterie BMS",                     "",   CNT,  "E", R(37000), None, "BAT"),
    S("BatteryIndex",        "Batterie Index",                   "",   CNT,  "E", R(37001), None, "BAT"),
    S("BatteryStatus",       "Batterie Status",                  "",   CNT,  "E", R(37002), None, "BAT"),
    S("BatteryModules",      "Batterie Modulanzahl",             "",   CNT,  "E", R(37009), None, "BAT"),
    S("BatteryProtocol",     "Batterie Protokoll",               "",   CNT,  "E", R(37011), None, "BAT"),
    S("BatteryError",        "Batterie Fehler",                  "",   BITS, "E", R(37012, "BITS22", 1, 37006), None, "BAT"),
    S("BatteryWarning",      "Batterie Warnung",                 "",   BITS, "E", R(37013, "BITS22", 1, 37010), None, "BAT"),
    S("BatterySwVersion",    "Batterie Softwareversion",         "",   CNT,  "E", R(37014), None, "BAT"),
    S("BatteryHwVersion",    "Batterie Hardwareversion",         "",   CNT,  "E", R(37015), None, "BAT"),
    S("BatteryMaxCellTempId", "Batterie Zelle max. Temperatur (Nr.)", "", CNT, "E", R(37016), None, "BAT"),
    S("BatteryMinCellTempId", "Batterie Zelle min. Temperatur (Nr.)", "", CNT, "E", R(37017), None, "BAT"),
    S("BatteryMaxCellVoltId", "Batterie Zelle max. Spannung (Nr.)",   "", CNT, "E", R(37018), None, "BAT"),
    S("BatteryMinCellVoltId", "Batterie Zelle min. Spannung (Nr.)",   "", CNT, "E", R(37019), None, "BAT"),
    S("BatteryMaxCellTemp",  "Batterie Zelltemperatur max.",     "°C", T,    "E", R(37020, "TEMP", 10), None, "BAT"),
    S("BatteryMinCellTemp",  "Batterie Zelltemperatur min.",     "°C", T,    "E", R(37021, "TEMP", 10), None, "BAT"),
    S("BatteryMaxCellVoltage", "Batterie Zellspannung max.",     "V",  U,    "E", R(37022, "U16", 1000), None, "BAT"),
    S("BatteryMinCellVoltage", "Batterie Zellspannung min.",     "V",  U,    "E", R(37023, "U16", 1000), None, "BAT"),
    S("BatteryCapacity",     "Batterie Kapazität (Ah)",          "Ah", CNT,  "E", R(45350), None, "BAT"),
]

BATTERY2 = [
    S("Battery2Voltage",     "Batterie 2 Spannung",              "V",  U,    "E", R(35262, "U16", 10), None, "BAT2"),
    S("Battery2Current",     "Batterie 2 Strom",                 "A",  I,    "E", R(35263, "S16", 10), None, "BAT2"),
    S("Battery2Power",       "Batterie 2 Leistung (+ Entladen)", "W",  P,    "E", R(35264, "S32"), None, "BAT2"),
    S("Battery2Mode",        "Batterie 2 Modus",                 "",   MODE, "E", R(35266), None, "BAT2"),
    S("Battery2Status",      "Batterie 2 Status",                "",   CNT,  "E", R(39000), None, "BAT2"),
    S("Battery2Temperature", "Batterie 2 Temperatur",            "°C", T,    "E", R(39001, "TEMP", 10), None, "BAT2"),
    S("Battery2ChargeLimit", "Batterie 2 Ladestromgrenze",       "A",  I,    "E", R(39002), None, "BAT2"),
    S("Battery2DischargeLimit", "Batterie 2 Entladestromgrenze", "A",  I,    "E", R(39003), None, "BAT2"),
    S("Battery2Soc",         "Batterie 2 Ladezustand",           "%",  PCT,  "E", R(39005), None, "BAT2"),
    S("Battery2Soh",         "Batterie 2 Gesundheitszustand",    "%",  PCT,  "E", R(39006), None, "BAT2"),
    S("Battery2Modules",     "Batterie 2 Modulanzahl",           "",   CNT,  "E", R(39007), None, "BAT2"),
    S("Battery2Protocol",    "Batterie 2 Protokoll",             "",   CNT,  "E", R(39009), None, "BAT2"),
    S("Battery2Error",       "Batterie 2 Fehler",                "",   BITS, "E", R(39010, "BITS22", 1, 39004), None, "BAT2"),
    S("Battery2Warning",     "Batterie 2 Warnung",               "",   BITS, "E", R(39011, "BITS22", 1, 39008), None, "BAT2"),
    S("Battery2SwVersion",   "Batterie 2 Softwareversion",       "",   CNT,  "E", R(39012), None, "BAT2"),
    S("Battery2HwVersion",   "Batterie 2 Hardwareversion",       "",   CNT,  "E", R(39013), None, "BAT2"),
    S("Battery2MaxCellTempId", "Batterie 2 Zelle max. Temperatur (Nr.)", "", CNT, "E", R(39014), None, "BAT2"),
    S("Battery2MinCellTempId", "Batterie 2 Zelle min. Temperatur (Nr.)", "", CNT, "E", R(39015), None, "BAT2"),
    S("Battery2MaxCellVoltId", "Batterie 2 Zelle max. Spannung (Nr.)",   "", CNT, "E", R(39016), None, "BAT2"),
    S("Battery2MinCellVoltId", "Batterie 2 Zelle min. Spannung (Nr.)",   "", CNT, "E", R(39017), None, "BAT2"),
    S("Battery2MaxCellTemp", "Batterie 2 Zelltemperatur max.",   "°C", T,    "E", R(39018, "TEMP", 10), None, "BAT2"),
    S("Battery2MinCellTemp", "Batterie 2 Zelltemperatur min.",   "°C", T,    "E", R(39019, "TEMP", 10), None, "BAT2"),
    S("Battery2MaxCellVoltage", "Batterie 2 Zellspannung max.",  "V",  U,    "E", R(39020, "U16", 1000), None, "BAT2"),
    S("Battery2MinCellVoltage", "Batterie 2 Zellspannung min.",  "V",  U,    "E", R(39021, "U16", 1000), None, "BAT2"),
]

ENERGY = [
    S("EnergyTotal",       "PV-Ertrag gesamt",           "kWh", E, "C", R(35191, "U32", 10), R(30145, "U32", 10)),
    S("EnergyToday",       "PV-Ertrag heute",            "kWh", E, "C", R(35193, "U32", 10), R(30144, "U16", 10)),
    S("MeterExportTotal",  "Zähler Einspeisung gesamt",  "kWh", E, "C", R(36015, "F32", 1000), R(30197, "U32", 1000), "METER"),
    S("MeterImportTotal",  "Zähler Netzbezug gesamt",    "kWh", E, "C", R(36017, "F32", 1000), R(30199, "U32", 1000), "METER"),
]

ENERGY_ET = [
    S("ExportTotal",          "Einspeisung gesamt",            "kWh", E, "E", R(35195, "U32", 10), None),
    S("ExportToday",          "Einspeisung heute",             "kWh", E, "E", R(35199, "U16", 10), None),
    S("ImportTotal",          "Netzbezug gesamt",              "kWh", E, "E", R(35200, "U32", 10), None),
    S("ImportToday",          "Netzbezug heute",               "kWh", E, "E", R(35202, "U16", 10), None),
    S("LoadTotal",            "Verbrauch gesamt",              "kWh", E, "E", R(35203, "U32", 10), None),
    S("LoadToday",            "Verbrauch heute",               "kWh", E, "E", R(35205, "U16", 10), None),
    S("BatteryChargeTotal",   "Batterie geladen gesamt",       "kWh", E, "E", R(35206, "U32", 10), None, "BAT"),
    S("BatteryChargeToday",   "Batterie geladen heute",        "kWh", E, "E", R(35208, "U16", 10), None, "BAT"),
    S("BatteryDischargeTotal", "Batterie entladen gesamt",     "kWh", E, "E", R(35209, "U32", 10), None, "BAT"),
    S("BatteryDischargeToday", "Batterie entladen heute",      "kWh", E, "E", R(35211, "U16", 10), None, "BAT"),
]

METER = [
    S("MeterCommode",        "Zähler Kommunikationsart",    "",    CNT, "E", R(36000), None, "METER"),
    S("MeterManufacturer",   "Zähler Herstellercode",       "",    CNT, "E", R(36002), None, "METER"),
    S("MeterTestStatus",     "Zähler Prüfstatus",           "",    MODE, "E", R(36003), None, "METER"),
    S("MeterTypeCode",       "Zähler Typ",                  "",    MODE, "E", R(36043), None, "METER"),
    S("MeterSwVersion",      "Zähler Softwareversion",      "",    CNT, "E", R(36044), None, "METER"),
    S("MeterPowerL1",        "Zähler Wirkleistung L1",      "W",   P,   "E", R(36019, "S32"), None, "METER"),
    S("MeterPowerL2",        "Zähler Wirkleistung L2",      "W",   P,   "E", R(36021, "S32"), None, "METER|L2"),
    S("MeterPowerL3",        "Zähler Wirkleistung L3",      "W",   P,   "E", R(36023, "S32"), None, "METER|L3"),
    S("MeterPowerTotal",     "Zähler Wirkleistung gesamt",  "W",   P,   "E", R(36025, "S32"), None, "METER"),
    S("MeterPower16L1",      "Zähler Wirkleistung L1 (16 Bit)", "W", P, "E", R(36005, "S16"), None, "METER"),
    S("MeterPower16L2",      "Zähler Wirkleistung L2 (16 Bit)", "W", P, "E", R(36006, "S16"), None, "METER|L2"),
    S("MeterPower16L3",      "Zähler Wirkleistung L3 (16 Bit)", "W", P, "E", R(36007, "S16"), None, "METER|L3"),
    S("MeterPower16Total",   "Zähler Wirkleistung gesamt (16 Bit)", "W", P, "E", R(36008, "S16"), None, "METER"),
    S("MeterReactiveL1",     "Zähler Blindleistung L1 (var)", "var", P, "E", R(36027, "S32"), None, "METER"),
    S("MeterReactiveL2",     "Zähler Blindleistung L2 (var)", "var", P, "E", R(36029, "S32"), None, "METER|L2"),
    S("MeterReactiveL3",     "Zähler Blindleistung L3 (var)", "var", P, "E", R(36031, "S32"), None, "METER|L3"),
    S("MeterReactiveTotal",  "Zähler Blindleistung gesamt (var)", "var", P, "E", R(36033, "S32"), None, "METER"),
    S("MeterReactive16Total", "Zähler Blindleistung gesamt (16 Bit, var)", "var", P, "E", R(36009, "S16"), None, "METER"),
    S("MeterApparentL1",     "Zähler Scheinleistung L1 (VA)", "VA", P,  "E", R(36035, "S32"), None, "METER"),
    S("MeterApparentL2",     "Zähler Scheinleistung L2 (VA)", "VA", P,  "E", R(36037, "S32"), None, "METER|L2"),
    S("MeterApparentL3",     "Zähler Scheinleistung L3 (VA)", "VA", P,  "E", R(36039, "S32"), None, "METER|L3"),
    S("MeterApparentTotal",  "Zähler Scheinleistung gesamt (VA)", "VA", P, "E", R(36041, "S32"), None, "METER"),
    S("MeterPowerFactorL1",  "Zähler Leistungsfaktor L1",   "",    PF,  "E", R(36010, "S16", 1000), None, "METER"),
    S("MeterPowerFactorL2",  "Zähler Leistungsfaktor L2",   "",    PF,  "E", R(36011, "S16", 1000), None, "METER|L2"),
    S("MeterPowerFactorL3",  "Zähler Leistungsfaktor L3",   "",    PF,  "E", R(36012, "S16", 1000), None, "METER|L3"),
    S("MeterPowerFactor",    "Zähler Leistungsfaktor",      "",    PF,  "E", R(36013, "S16", 1000), None, "METER"),
    S("MeterFrequency",      "Zähler Frequenz",             "Hz",  F,   "E", R(36014, "S16", 100), None, "METER"),
    S("MeterVoltageL1",      "Zähler Spannung L1",          "V",   U,   "E", R(36052, "U16", 10), None, "METER|MEXT"),
    S("MeterVoltageL2",      "Zähler Spannung L2",          "V",   U,   "E", R(36053, "U16", 10), None, "METER|MEXT|L2"),
    S("MeterVoltageL3",      "Zähler Spannung L3",          "V",   U,   "E", R(36054, "U16", 10), None, "METER|MEXT|L3"),
    S("MeterCurrentL1",      "Zähler Strom L1",             "A",   I,   "E", R(36055, "U16", 10), None, "METER|MEXT"),
    S("MeterCurrentL2",      "Zähler Strom L2",             "A",   I,   "E", R(36056, "U16", 10), None, "METER|MEXT|L2"),
    S("MeterCurrentL3",      "Zähler Strom L3",             "A",   I,   "E", R(36057, "U16", 10), None, "METER|MEXT|L3"),
    S("Meter2Power",         "Zähler 2 Wirkleistung",       "W",   P,   "E", R(36045, "S32"), None, "MEXT"),
    S("Meter2ExportTotal",   "Zähler 2 Einspeisung gesamt", "kWh", E,   "E", R(36047, "F32", 1000), None, "MEXT"),
    S("Meter2ImportTotal",   "Zähler 2 Netzbezug gesamt",   "kWh", E,   "E", R(36049, "F32", 1000), None, "MEXT"),
    S("Meter2CommStatus",    "Zähler 2 Kommunikationsstatus", "",  MODE, "E", R(36051), None, "MEXT"),
]

METER_ENERGY = [
    S("MeterExportL1",       "Zähler Einspeisung L1",       "kWh", E, "E", R(36092, "U64", 100), None, "METER|MEXT2"),
    S("MeterExportL2",       "Zähler Einspeisung L2",       "kWh", E, "E", R(36096, "U64", 100), None, "METER|MEXT2|L2"),
    S("MeterExportL3",       "Zähler Einspeisung L3",       "kWh", E, "E", R(36100, "U64", 100), None, "METER|MEXT2|L3"),
    S("MeterExportTotal64",  "Zähler Einspeisung gesamt (64 Bit)", "kWh", E, "E", R(36104, "U64", 100), None, "METER|MEXT2"),
    S("MeterImportL1",       "Zähler Netzbezug L1",         "kWh", E, "E", R(36108, "U64", 100), None, "METER|MEXT2"),
    S("MeterImportL2",       "Zähler Netzbezug L2",         "kWh", E, "E", R(36112, "U64", 100), None, "METER|MEXT2|L2"),
    S("MeterImportL3",       "Zähler Netzbezug L3",         "kWh", E, "E", R(36116, "U64", 100), None, "METER|MEXT2|L3"),
    S("MeterImportTotal64",  "Zähler Netzbezug gesamt (64 Bit)", "kWh", E, "E", R(36120, "U64", 100), None, "METER|MEXT2"),
]

BMS = []
for _b, _base in ((1, 47900), (2, 47918)):
    _p = "Bms%d" % _b
    _l = "BMS %d " % _b
    _c = "BAT"
    BMS += [
        S(_p + "Version",          _l + "Version",                   "",   CNT,  "E", R(_base), None, _c),
        S(_p + "Modules",          _l + "Modulanzahl",               "",   CNT,  "E", R(_base + 1), None, _c),
        S(_p + "ChargeVoltageMax", _l + "Ladespannung max.",         "V",  U,    "E", R(_base + 2, "U16", 10), None, _c),
        S(_p + "ChargeCurrentMax", _l + "Ladestrom max.",            "A",  I,    "E", R(_base + 3, "U16", 10), None, _c),
        S(_p + "DischargeVoltageMin", _l + "Entladespannung min.",   "V",  U,    "E", R(_base + 4, "U16", 10), None, _c),
        S(_p + "DischargeCurrentMax", _l + "Entladestrom max.",      "A",  I,    "E", R(_base + 5, "U16", 10), None, _c),
        S(_p + "Voltage",          _l + "Spannung",                  "V",  U,    "E", R(_base + 6, "U16", 10), None, _c),
        S(_p + "Current",          _l + "Strom",                     "A",  I,    "E", R(_base + 7, "U16", 10), None, _c),
        S(_p + "Soc",              _l + "Ladezustand",               "%",  PCT,  "E", R(_base + 8), None, _c),
        S(_p + "Soh",              _l + "Gesundheitszustand",        "%",  PCT,  "E", R(_base + 9), None, _c),
        S(_p + "Temperature",      _l + "Temperatur",                "°C", T,    "E", R(_base + 10, "TEMP", 10), None, _c),
        S(_p + "WarningCode",      _l + "Warnungscode",              "",   BITS, "E", R(_base + 11, "U32"), None, _c),
        S(_p + "AlarmCode",        _l + "Alarmcode",                 "",   BITS, "E", R(_base + 13, "U32"), None, _c),
        S(_p + "Status",           _l + "Status",                    "",   CNT,  "E", R(_base + 15), None, _c),
        S(_p + "CommLossDisable",  _l + "Kommunikationsverlust ignorieren", "", CNT, "E", R(_base + 16), None, _c),
        S(_p + "StringRateVoltage", _l + "Strang-Nennspannung",      "",   CNT,  "E", R(_base + 17), None, _c),
    ]

# Gruppen erscheinen als Tabellen auf der Kanalseite. Reihenfolge = KO-Reihenfolge.
SENSOR_GROUPS = [
    ("Status",                         STATUS),
    ("Status Hybrid",                  STATUS_ET),
    ("Status netzgekoppelt",           STATUS_DT),
    ("PV",                             PV),
    ("PV Hybrid",                      PV_ET),
    ("PV netzgekoppelt",               PV_DT),
    ("PV erweitert (MPPT-Block)",      PV_EXT),
    ("Netz",                           GRID),
    ("Netz Hybrid",                    GRID_ET),
    ("Netz netzgekoppelt",             GRID_DT),
    ("Backup und Last",                BACKUP),
    ("Batterie",                       BATTERY),
    ("Batterie 2",                     BATTERY2),
    ("Energie",                        ENERGY),
    ("Energie Hybrid",                 ENERGY_ET),
    ("Zähler",                         METER),
    ("Zähler Energie je Phase",        METER_ENERGY),
    ("BMS-Daten",                      BMS),
]

SENSORS = [s for _, g in SENSOR_GROUPS for s in g]

# Platz fuer spaeter angehaengte Messwerte; die Einstellungen beginnen dahinter. So bleiben
# deren KO-Nummern stabil, wenn Messwerte dazukommen.
SENSOR_CAPACITY = 300

# ================================================================== Einstellungen
# (Name, Anzeigetext, Familie, DPT Eingang, DPT Status oder None (nur Ausloeser), Beschreibung)
SETTINGS = [
    ("OperationMode",    "Betriebsmodus",               "E", MODE, MODE,
     "0=Allgemein 1=Inselbetrieb 2=Backup 3=Eco 4=Peak Shaving 5=Eigenverbrauch 6=Eco Laden 7=Eco Entladen"),
    ("EmsMode",          "EMS-Modus",                   "E", MODE, MODE,
     "1=Auto 2=Laden PV 3=Entladen PV 4=Import AC 5=Export AC 6=Erhalten 7=Inselbetrieb 8=Batterie Standby 9=Bezug 10=Verkauf 11=Batterie laden 12=Batterie entladen"),
    ("EmsPowerLimit",    "EMS-Leistung",                "E", P, P, "W"),
    ("ExportLimitEnable", "Einspeisebegrenzung aktiv",  "C", SW, SW, ""),
    ("ExportLimit",      "Einspeisebegrenzung",         "C", P, P, "W"),
    ("ExportLimitPercent", "Einspeisebegrenzung (%)",   "D", PCT, PCT, "%"),
    ("DodOnGrid",        "Entladetiefe Netzbetrieb",    "E", PCT, PCT, "%"),
    ("DodOffGrid",       "Entladetiefe Inselbetrieb",   "E", PCT, PCT, "%"),
    ("SocProtection",    "SoC-Schutz",                  "E", PCT, PCT, "%"),
    ("SocUpperLimit",    "SoC-Obergrenze",              "E", PCT, PCT, "%"),
    ("EcoModePower",     "Eco-Leistung",                "E", PCT, PCT, "%"),
    ("EcoModeSoc",       "Eco-Ziel-SoC",                "E", PCT, PCT, "%"),
    ("FastCharging",     "Schnellladen",                "E", SW, SW, ""),
    ("FastChargingSoc",  "Schnellladen Ziel-SoC",       "E", PCT, PCT, "%"),
    ("FastChargingPower", "Schnellladen Leistung",      "E", PCT, PCT, "%"),
    ("BackupSupply",     "Backup-Versorgung",           "E", SW, SW, ""),
    ("DodHolding",       "Entladetiefe halten",         "E", SW, SW, ""),
    ("LoadControl",      "Lastregelung",                "E", SW, SW, ""),
    ("SyncClock",        "Uhr synchronisieren",         "C", TRIG, None, ""),
    ("StartInverter",    "Wechselrichter starten",      "D", TRIG, None, ""),
    ("StopInverter",     "Wechselrichter stoppen",      "D", TRIG, None, ""),
]

SETTING_CAPACITY = 50   # Aktiv-Bits fuer Einstellungen
SETTING_KO_BASE = SENSOR_CAPACITY   # je Einstellung zwei KOs: Eingang, Status

ENABLE_BITS = SENSOR_CAPACITY + SETTING_CAPACITY

assert len(SENSORS) <= SENSOR_CAPACITY, "zu viele Messwerte: %d" % len(SENSORS)
assert len(SETTINGS) <= SETTING_CAPACITY
assert len(set(s[0] for s in SENSORS) | set(s[0] for s in SETTINGS)) == len(SENSORS) + len(SETTINGS), \
    "Namen muessen eindeutig sein"
assert SENSORS[0][0] == "Reachable"


def setting_ko(index):
    """KO-Nummern (Eingang, Status) einer Einstellung relativ zum Kanal."""
    base = SETTING_KO_BASE + 2 * index
    return base, base + 1


if __name__ == "__main__":
    i = 0
    for title, group in SENSOR_GROUPS:
        print("%s (%d)" % (title, len(group)))
        for s in group:
            print("  %3d  %-24s %-40s %s" % (i, s[0], s[1], s[4]))
            i += 1
    print("Einstellungen (%d)" % len(SETTINGS))
    for n, s in enumerate(SETTINGS):
        print("  %3d/%3d  %-20s %s" % (setting_ko(n) + (s[0], s[1])))
    print("Messwerte: %d, Einstellungen: %d" % (len(SENSORS), len(SETTINGS)))
