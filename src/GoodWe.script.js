// ERZEUGT von tools/gen_templ.py - nicht von Hand bearbeiten. Logik: tools/script.template.js
// Assistent "Wechselrichter auslesen".
// Gegenstelle im Geraet: GoodWeModule::processFunctionProperty, objectIndex 0xA4, propertyId 1.
// Ablauf wie OFM-IPCameraModule: Kommando 1 stoesst an und kehrt sofort zurueck, Kommando 2
// wird gepollt, bis resp[1] == 1 das Ergebnis meldet. Die Antwort ist groesser als eine APDU,
// deshalb laeuft alles ueber BASE_invokeFunctionPropertyWrapper (stueckelt).

var GDW_FUNCTION_OBJECT = 0xA4;
var GDW_FUNCTION_PROPERTY = 1;
var GDW_CMD_START = 1;
var GDW_CMD_STATUS = 2;

// Name des Aktiv-Parameters je Bit, Reihenfolge wie tools/slots.py. Leere Eintraege sind
// freie Plaetze. Parameter heissen GDW_CH<Kanal>Use<Name>.
var gdwBitNames = [
    "Reachable", "WorkMode", "ErrorCodes", "WarningCode", "SafetyCountry", "FunctionBit", "HoursTotal", "Timestamp",
    "Temperature", "BusVoltage", "NBusVoltage", "GridInOut", "Rssi", "MeterCommStatus", "GridMode", "OperationCode",
    "DiagStatus", "TempAir", "TempModule", "TempHeatsink", "DeratingMode", "LeakageCurrent", "PvPower", "Pv1Voltage",
    "Pv1Current", "Pv1Power", "Pv2Voltage", "Pv2Current", "Pv2Power", "Pv3Voltage", "Pv3Current", "Pv3Power",
    "Pv4Voltage", "Pv4Current", "Pv4Power", "Pv1Mode", "Pv2Mode", "Pv3Mode", "Pv4Mode", "TotalInputPower",
    "PvPowerTotalExt", "PvChannel", "Pv5Voltage", "Pv5Current", "Pv6Voltage", "Pv6Current", "Pv7Voltage", "Pv7Current",
    "Pv8Voltage", "Pv8Current", "Pv9Voltage", "Pv9Current", "Pv10Voltage", "Pv10Current", "Pv11Voltage", "Pv11Current",
    "Pv12Voltage", "Pv12Current", "Pv13Voltage", "Pv13Current", "Pv14Voltage", "Pv14Current", "Pv15Voltage", "Pv15Current",
    "Pv16Voltage", "Pv16Current", "Mppt1Power", "Mppt2Power", "Mppt3Power", "Mppt4Power", "Mppt5Power", "Mppt6Power",
    "Mppt7Power", "Mppt8Power", "Mppt1Current", "Mppt2Current", "Mppt3Current", "Mppt4Current", "Mppt5Current", "Mppt6Current",
    "Mppt7Current", "Mppt8Current", "GridVoltageL1", "GridCurrentL1", "GridFrequencyL1", "GridPowerL1", "GridVoltageL2", "GridCurrentL2",
    "GridFrequencyL2", "GridPowerL2", "GridVoltageL3", "GridCurrentL3", "GridFrequencyL3", "GridPowerL3", "InverterPower", "ActivePower",
    "ImportPower", "ExportPower", "ReactivePower", "ApparentPower", "HouseConsumption", "ReactivePowerL1", "ReactivePowerL2", "ReactivePowerL3",
    "ApparentPowerL1", "ApparentPowerL2", "ApparentPowerL3", "LineVoltageL1L2", "LineVoltageL2L3", "LineVoltageL3L1", "PowerFactor", "BackupVoltageL1",
    "BackupCurrentL1", "BackupFrequencyL1", "LoadModeL1", "BackupPowerL1", "BackupVoltageL2", "BackupCurrentL2", "BackupFrequencyL2", "LoadModeL2",
    "BackupPowerL2", "BackupVoltageL3", "BackupCurrentL3", "BackupFrequencyL3", "LoadModeL3", "BackupPowerL3", "LoadPowerL1", "LoadPowerL2",
    "LoadPowerL3", "BackupPowerTotal", "LoadPowerTotal", "UpsLoad", "BatteryVoltage", "BatteryCurrent", "BatteryPower", "BatteryMode",
    "BatterySoc", "BatterySoh", "BatteryTemperature", "BatteryChargeLimit", "BatteryDischargeLimit", "BatteryBms", "BatteryIndex", "BatteryStatus",
    "BatteryModules", "BatteryProtocol", "BatteryError", "BatteryWarning", "BatterySwVersion", "BatteryHwVersion", "BatteryMaxCellTempId", "BatteryMinCellTempId",
    "BatteryMaxCellVoltId", "BatteryMinCellVoltId", "BatteryMaxCellTemp", "BatteryMinCellTemp", "BatteryMaxCellVoltage", "BatteryMinCellVoltage", "BatteryCapacity", "Battery2Voltage",
    "Battery2Current", "Battery2Power", "Battery2Mode", "Battery2Status", "Battery2Temperature", "Battery2ChargeLimit", "Battery2DischargeLimit", "Battery2Soc",
    "Battery2Soh", "Battery2Modules", "Battery2Protocol", "Battery2Error", "Battery2Warning", "Battery2SwVersion", "Battery2HwVersion", "Battery2MaxCellTempId",
    "Battery2MinCellTempId", "Battery2MaxCellVoltId", "Battery2MinCellVoltId", "Battery2MaxCellTemp", "Battery2MinCellTemp", "Battery2MaxCellVoltage", "Battery2MinCellVoltage", "EnergyTotal",
    "EnergyToday", "MeterExportTotal", "MeterImportTotal", "ExportTotal", "ExportToday", "ImportTotal", "ImportToday", "LoadTotal",
    "LoadToday", "BatteryChargeTotal", "BatteryChargeToday", "BatteryDischargeTotal", "BatteryDischargeToday", "MeterCommode", "MeterManufacturer", "MeterTestStatus",
    "MeterTypeCode", "MeterSwVersion", "MeterPowerL1", "MeterPowerL2", "MeterPowerL3", "MeterPowerTotal", "MeterPower16L1", "MeterPower16L2",
    "MeterPower16L3", "MeterPower16Total", "MeterReactiveL1", "MeterReactiveL2", "MeterReactiveL3", "MeterReactiveTotal", "MeterReactive16Total", "MeterApparentL1",
    "MeterApparentL2", "MeterApparentL3", "MeterApparentTotal", "MeterPowerFactorL1", "MeterPowerFactorL2", "MeterPowerFactorL3", "MeterPowerFactor", "MeterFrequency",
    "MeterVoltageL1", "MeterVoltageL2", "MeterVoltageL3", "MeterCurrentL1", "MeterCurrentL2", "MeterCurrentL3", "Meter2Power", "Meter2ExportTotal",
    "Meter2ImportTotal", "Meter2CommStatus", "MeterExportL1", "MeterExportL2", "MeterExportL3", "MeterExportTotal64", "MeterImportL1", "MeterImportL2",
    "MeterImportL3", "MeterImportTotal64", "Bms1Version", "Bms1Modules", "Bms1ChargeVoltageMax", "Bms1ChargeCurrentMax", "Bms1DischargeVoltageMin", "Bms1DischargeCurrentMax",
    "Bms1Voltage", "Bms1Current", "Bms1Soc", "Bms1Soh", "Bms1Temperature", "Bms1WarningCode", "Bms1AlarmCode", "Bms1Status",
    "Bms1CommLossDisable", "Bms1StringRateVoltage", "Bms2Version", "Bms2Modules", "Bms2ChargeVoltageMax", "Bms2ChargeCurrentMax", "Bms2DischargeVoltageMin", "Bms2DischargeCurrentMax",
    "Bms2Voltage", "Bms2Current", "Bms2Soc", "Bms2Soh", "Bms2Temperature", "Bms2WarningCode", "Bms2AlarmCode", "Bms2Status",
    "Bms2CommLossDisable", "Bms2StringRateVoltage", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "OperationMode", "EmsMode", "EmsPowerLimit", "ExportLimitEnable",
    "ExportLimit", "ExportLimitPercent", "DodOnGrid", "DodOffGrid", "SocProtection", "SocUpperLimit", "EcoModePower", "EcoModeSoc",
    "FastCharging", "FastChargingSoc", "FastChargingPower", "BackupSupply", "DodHolding", "LoadControl", "SyncClock", "StartInverter",
    "StopInverter", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", ""
];

var gdwErrors = [
    "",
    "Der Wechselrichter antwortet weder über UDP 8899 noch über Modbus TCP 502. Adresse prüfen; netzgekoppelte Geräte (DT/MS/XS) schlafen nachts - dann bitte tagsüber ausführen.",
    "Das Gerät antwortet, ist aber kein unterstützter GoodWe-Wechselrichter (ET- oder DT-Familie).",
    "Das KNX-Gerät hat keine Netzwerkverbindung.",
    "Es läuft bereits eine Abfrage."
];

function GDW_sleep(milliseconds) {
    var start = new Date().getTime();
    while (new Date().getTime() - start < milliseconds) { /* warten */ }
}

// Zerlegt eine IPv4-Adresse in vier Bytes. Gibt null zurueck, wenn sie nicht taugt.
function GDW_ip4(text) {
    var parts = ("" + text).split(".");
    if (parts.length != 4) return null;
    var out = [];
    for (var i = 0; i < 4; i++) {
        var n = parseInt(parts[i], 10);
        if (isNaN(n) || n < 0 || n > 255) return null;
        out.push(n);
    }
    return out;
}

function GDW_text(resp, from, length) {
    var s = "";
    for (var i = from; i < from + length && i < resp.length; i++) {
        if (resp[i] === 0) break;
        if (resp[i] >= 32 && resp[i] < 127) s += String.fromCharCode(resp[i]);
    }
    return s;
}

function GDW_setParam(device, name, value) {
    var par = device.getParameterByName(name);
    if (!par) return false;
    par.value = value;
    return true;
}

function GDW_readInverter(device, online, progress, context) {
    var channel = Number(context.channel);
    var prefix = "GDW_CH" + channel;
    var info = device.getParameterByName(prefix + "Detected");

    var ip = GDW_ip4(device.getParameterByName(prefix + "Ip").value);
    if (!ip) {
        progress.setText("GoodWe " + channel + ": bitte zuerst eine IPv4-Adresse eintragen, z.B. 192.168.1.50.");
        return;
    }

    progress.setProgress(2);
    progress.setText("GoodWe " + channel + ": verbinde mit dem KNX-Gerät ...");
    online.connect();

    try {
        var resp = BASE_invokeFunctionPropertyWrapper(GDW_FUNCTION_OBJECT, GDW_FUNCTION_PROPERTY,
            [GDW_CMD_START, channel, ip[0], ip[1], ip[2], ip[3]], device, online, progress);
        if (!resp || resp.length < 1 || resp[0] != 0) {
            progress.setText("GoodWe " + channel + ": das KNX-Gerät hat die Abfrage abgelehnt. Ist die Firmware mit GoodWe-Modul programmiert?");
            return;
        }

        // Erkennung, Blockproben und Einstellungen dauern je nach Geraet 5-40 s.
        var done = null;
        for (var attempt = 0; attempt < 90; attempt++) {
            if (progress.isCanceled && progress.isCanceled()) return;
            GDW_sleep(1000);
            progress.setProgress(5 + attempt);
            var status = BASE_invokeFunctionPropertyWrapper(GDW_FUNCTION_OBJECT, GDW_FUNCTION_PROPERTY,
                [GDW_CMD_STATUS, channel], device, online, progress);
            if (!status || status.length < 2 || status[0] != 0) {
                progress.setText("GoodWe " + channel + ": Statusabfrage fehlgeschlagen.");
                return;
            }
            if (status[1] == 1) { done = status; break; }
            progress.setText("GoodWe " + channel + ": lese Wechselrichter " + ip.join(".") + " aus ...");
        }
        if (!done) {
            progress.setText("GoodWe " + channel + ": Zeitüberschreitung.");
            return;
        }

        // [2] Fehler [3] Familie (2 Hybrid, 3 netzgekoppelt) [4] Protokoll [5..6] Port
        // [7] Modbus-Adresse [8] Merkmale [9..10] Nennleistung [11..12] ARM-Version
        // [13] Anzahl Bitbytes N, [14..] Bits (LSB zuerst), danach Modell (10) und Seriennummer (16)
        if (done[2] != 0) {
            var err = (done[2] < gdwErrors.length) ? gdwErrors[done[2]] : ("Fehler " + done[2]);
            if (info) info.value = "Nicht erkannt.";
            progress.setText("GoodWe " + channel + ": " + err);
            return;
        }
        if (done.length < 14) {
            progress.setText("GoodWe " + channel + ": unerwartete Antwortlänge " + done.length + ".");
            return;
        }

        var family = done[3];
        var protocol = done[4];
        var port = done[5] * 256 + done[6];
        var address = done[7];
        var flags = done[8];
        var rated = done[9] * 256 + done[10];
        var nBytes = done[13];
        var model = GDW_text(done, 14 + nBytes, 10);
        var serial = GDW_text(done, 24 + nBytes, 16);

        GDW_setParam(device, prefix + "Type", family);
        GDW_setParam(device, prefix + "TypeSelect", family);
        GDW_setParam(device, prefix + "Protocol", protocol);
        GDW_setParam(device, prefix + "Port", port);
        GDW_setParam(device, prefix + "Address", address);

        var active = 0;
        for (var k = 0; k < gdwBitNames.length; k++) {
            if (gdwBitNames[k] === "") continue;
            var bit = 0;
            if ((k >> 3) < nBytes) bit = (done[14 + (k >> 3)] >> (k & 7)) & 1;
            if (GDW_setParam(device, prefix + "Use" + gdwBitNames[k], bit) && bit) active++;
        }

        var parts = [];
        parts.push(family == 2 ? "Hybrid" : "netzgekoppelt");
        parts.push((flags & 1) ? "1-phasig" : "3-phasig");
        if (flags & 2) parts.push("Batterie");
        if (flags & 4) parts.push("Batterie 2");
        if (flags & 8) parts.push("Zähler");
        var text = (model ? model : "GoodWe") + (serial ? " (" + serial + ")" : "") + ": " + parts.join(", ") + ", " + active + " Objekte";
        if (rated > 0) text += ", " + rated + " W";
        if (info) info.value = text.substring(0, 70);

        progress.setProgress(100);
        progress.setText("GoodWe " + channel + ": " + text + ". Bitte erneut programmieren.");
    } finally {
        online.disconnect();
    }
}
