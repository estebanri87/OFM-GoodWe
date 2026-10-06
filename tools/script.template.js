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
/*BITNAMES*/
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
