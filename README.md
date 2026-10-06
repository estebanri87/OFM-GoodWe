# OFM-GoodWe

OpenKNX-Modul für **GoodWe-Wechselrichter**: liest sie lokal über den WiFi/LAN-Dongle aus und
stellt Messwerte und Einstellungen als KNX-Gruppenobjekte bereit. Keine Cloud.

**Status: Beta, ohne Testgerät entwickelt.** Die Registerbelegung ist aus der Python-Bibliothek
[goodwe](https://github.com/marcelblijleven/goodwe) 0.4.10 übernommen, die auch die
[offizielle](https://www.home-assistant.io/integrations/goodwe) und die
[inoffizielle](https://github.com/mletenay/home-assistant-goodwe-inverter) Home-Assistant-
Integration verwenden. Am Gerät ist noch nichts bestätigt.

## Umfang

* Familien **Hybrid** (ET, EH, BT, BH, ES-G2, Plattform 205/745/753) und **netzgekoppelt**
  (DT, MS, D-NS, XS, SDT). Nicht: alte ES/EM/BP mit AA55-Protokoll.
* Transport **UDP 8899** (Modbus-RTU-Rahmen, Antwort mit `AA 55`-Präfix) oder
  **Modbus TCP 502** (LAN-Kit V2.0, Kit-20).
* 274 Messwerte - alle Sensoren der Bibliothek, Dubletten zusammengefasst - und 21
  Einstellungen, also alle Number-, Switch-, Select- und Button-Entitäten der inoffiziellen
  Integration (Betriebsmodus inkl. Eco Laden/Entladen, EMS-Modus und -Leistung,
  Einspeisebegrenzung, Entladetiefen, SoC-Grenzen, Schnellladen, Backup-Versorgung,
  DoD halten, Lastregelung, Uhr synchronisieren, Start/Stopp).
* Jedes Objekt einzeln per Häkchen wählbar. Der ETS-Assistent **„Wechselrichter auslesen"**
  erkennt Protokoll, Typ und Modbus-Adresse und hakt alle Objekte an, die das Gerät liefert.
* 2 Kanäle, je bis zu 341 KOs.

Details: [Applikationsbeschreibung](doc/Applikationsbeschreibung-GoodWe.md).

## Aufbau

* `tools/slots.py` - **die eine Quelle** für alle Objekte (Register, Typ, Skalierung, DPT,
  Bedingungen). Die Reihenfolge bestimmt KO-Nummern und Parameterlage: nur hinten anhängen.
* `python tools/generate.py` erzeugt daraus `src/GoodWe.templ.xml`, `src/SlotCatalog.h/.cpp`
  und `src/GoodWe.script.js` (Logik in `tools/script.template.js`).
* `GoodWeClient` - KNX-frei, nicht blockierend (O_NONBLOCK, `select()` mit Timeout 0), UDP und
  TCP, je Anfrage höchstens ein zweiter Versuch, mindestens 250 ms Abstand zwischen Anfragen.
* `GoodWeData` - Registerabbild, Dekodierung, Modellregeln (`model.py`).
* `GoodWeChannel` - Lesezyklus nur über die Blöcke angehakter Objekte, Einstellungen alle
  5 Minuten, Schreibaufträge als Schrittfolge (wie `set_operation_mode` der Bibliothek).
* `GoodWeAssistant` - Erkennung für die ETS (Function Property 0xA4).

Hilfetexte: `openknxproducer baggages` (VS-Code-Task „OpenKNXproducer Documentation").

## Bekannte Einschränkungen

* Der Dongle verträgt nur einen Client. Parallel laufendes Home Assistant stört.
* Schnelles Abfragen kann SEMS-Uploads stoppen; einige Dongles (Kit-20) stürzen bei vielen
  Wiederholungen ab.
* Netzgekoppelte Geräte schlafen nachts; der Assistent muss tagsüber laufen.
* Eco Laden/Entladen überschreibt Eco-Gruppe 1 des Wechselrichters.
* Funktionen ab ARM-Firmware 19/22 fehlen auf älteren Geräten.

## Lizenz

GPL-3.0, siehe [LICENSE](LICENSE).
