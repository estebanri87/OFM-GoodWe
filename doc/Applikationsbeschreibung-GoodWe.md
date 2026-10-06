<!-- SPDX-License-Identifier: GPL-3.0-only -->
<!-- Copyright (C) 2026 OpenKNX -->

<!-- KEINE MARKDOWN-TABELLEN in den DOC-Bloecken: die ETS zeigt sie als rohe Pipe-Zeichen an.
     Aufzaehlungen verwenden. Die Bloecke zwischen DOC und DOCEND werden von
     "openknxproducer baggages" zu den Hilfetexten in src/Baggages/Help_de/ verarbeitet. -->

# Applikationsbeschreibung GoodWe

Das Modul liest GoodWe-Wechselrichter **lokal über das Netzwerk** aus und stellt Messwerte
und Einstellungen als KNX-Gruppenobjekte bereit. Keine Cloud, kein SEMS-Konto.

Unterstützt werden die beiden Familien, die auch die Home-Assistant-Integrationen
(offiziell und mletenay) abdecken. Grundlage ist die Python-Bibliothek goodwe 0.4.10:

* **Hybrid**: ET, EH, BT, BH, ES-G2 und weitere Geräte der Plattformen 205/745/753
* **Netzgekoppelt**: DT, MS, D-NS, XS, SDT

Nicht unterstützt werden die alten ES/EM/BP-Geräte mit dem AA55-Protokoll.

## Wichtige Hinweise

* Diese KNXprod wird nicht von der KNX Association offiziell unterstützt!
* Die Erzeugung der KNXprod geschieht auf eure eigene Verantwortung!
* Die Registerbelegung stammt aus der goodwe-Bibliothek und ist ohne Testgerät
  **ungeprüft**. Rückmeldungen mit `gdwread`-Ausgaben sind willkommen.

# Allgemein

<!-- DOC -->
## Allgemein

Die Seite zeigt die Modulversion. Welche Wechselrichter angebunden werden, wird auf der Seite
**Kanalauswahl** festgelegt: je Kanal der Wechselrichtertyp (oder „Deaktiviert") und eine
Beschreibung. Jeder aktivierte Kanal bekommt eine eigene Seite mit eigener Verbindung und
eigenem Satz Kommunikationsobjekte.

Der WiFi/LAN-Dongle verträgt nur **einen** Client zuverlässig. Läuft parallel eine andere
Abfrage (etwa Home Assistant oder SolarGo im lokalen Netz), kommt es zu Aussetzern.

<!-- DOCEND -->

<!-- DOC -->
## Diagnose

Aktiviert ein Textobjekt (DPT 16.001), auf das das Modul kurze Meldungen schreibt, zum
Beispiel „GW1 erreichbar", „GW1 nicht erreichbar", „GW1 Schreibfehler" oder „GW1 Funktion
fehlt".

<!-- DOCEND -->

# Kanal

<!-- DOC -->
## Wechselrichtertyp

* **Automatisch erkennen**: Die Firmware fragt beim Start zuerst die Geräteinformation der
  Hybrid-Familie (Register 35000), dann die der netzgekoppelten Familie (30001) ab.
* **Hybrid (ET, EH, BT, BH, ES-G2)**: Wechselrichter mit Batterieanschluss.
* **Netzgekoppelt (DT, MS, D-NS, XS)**: reine PV-Wechselrichter. Sie schalten nachts ab und
  sind dann nicht erreichbar - das ist normal.

„Wechselrichter auslesen" setzt den Typ automatisch.

<!-- DOCEND -->

<!-- DOC -->
## Verbindung

* **IP-Adresse**: Adresse des WiFi/LAN-Dongles. Nur IPv4-Adressen, **keine Hostnamen** -
  eine Namensauflösung würde den KNX-Stack blockieren. Am besten eine feste Adresse im Router
  vergeben.

<!-- DOCEND -->

<!-- DOC -->
## Protokoll

* **UDP (Port 8899)**: ältere WiFi-Kits und LAN-Kits. Das ist der Weg, den auch die
  SolarGo-App nutzt.
* **Modbus TCP (Port 502)**: LAN-Kit V2.0 (WLA0000-01-00P) und Kit-20 (WLA0000-03). Das
  Kit-20 kann **nur** TCP. Modbus TCP muss in SolarGo eingeschaltet sein.

Die Suche von „Wechselrichter auslesen" probiert erst UDP, dann TCP - wie Home Assistant.

<!-- DOCEND -->

<!-- DOC -->
## Modbus-Adresse

Standard ist **247** für die Hybrid-Familie und **127** für die netzgekoppelte Familie. Bei
Modbus TCP verlangt der Dongle in der Regel 247; die Werkseinstellung 245 mancher Geräte
führt zu Fehlern. Die Firmware versucht bei einer abweichenden Einstellung zusätzlich die
Standardadresse der Familie.

<!-- DOCEND -->

<!-- DOC -->
## Assistent

„Wechselrichter auslesen" ermittelt über das programmierte KNX-Gerät:

* Protokoll, Port und Modbus-Adresse
* Familie, Modell, Seriennummer, Phasenzahl und Leistungsklasse
* ob Batterie, zweite Batterie und GoodWe-Zähler vorhanden sind
* welche Messwerte das Gerät liefert und welche Einstellungen es unterstützt

Anschließend sind genau diese Objekte angehakt; alles andere ist abgewählt. Die Auswahl lässt
sich danach von Hand anpassen. Danach das Gerät **erneut programmieren**.

Voraussetzungen: Das KNX-Gerät ist mit einer Firmware mit GoodWe-Modul programmiert und im
Netz, und der Wechselrichter ist eingeschaltet. Netzgekoppelte Geräte schlafen nachts - dann
bitte tagsüber ausführen. Während der Suche ruhen alle GoodWe-Kanäle.

<!-- DOCEND -->

<!-- DOC -->
## Abfrage

**Abfrageintervall** (Sekunden, mindestens 10): Abstand zwischen zwei Lesezyklen. Gelesen
werden nur die Registerblöcke, in denen ein angehaktes Objekt liegt.

Zu schnelles Abfragen kann die Übertragung an die SEMS-Cloud stoppen und manche Dongles zum
Absturz bringen. Wer SEMS weiter nutzt, sollte 60 Sekunden oder mehr wählen. Einstellungen
werden alle 5 Minuten und 2 Sekunden nach jedem Schreiben gelesen. Alle 10 Minuten geht ein
Weckruf an den Dongle (UDP 48899), weil manche Dongles nachts einfrieren.

<!-- DOCEND -->

<!-- DOC -->
## Sendeverhalten

* **zyklisch senden alle**: sendet alle Messwerte in diesem Abstand erneut (0 = nicht).
* **bei Änderung um**: sendet Leistungen, Spannungen, Ströme, Temperaturen und Energien,
  sobald sie sich um mindestens diesen Prozentsatz zum zuletzt gesendeten Wert ändern
  (0 = jede Änderung). Zustände, Codes und Prozentwerte werden bei jeder Änderung gesendet.

Nach dem Start wird jeder Wert einmal gesendet, sobald er gelesen wurde.

<!-- DOCEND -->

<!-- DOC -->
## Objekte

Jedes Objekt hat ein eigenes Häkchen. Nur angehakte Objekte werden als KO angelegt und
gesendet. Die Gruppen entsprechen den Daten der goodwe-Bibliothek:

* **Status**: Arbeitsmodus, Fehler, Warnungen, Temperaturen, Betriebsstunden, Gerätezeit
* **PV**: Spannung, Strom und Leistung je String, PV-Leistung gesamt
* **PV erweitert**: PV5-PV16 und MPPT1-8 (Geräte der 745-Plattform bzw. ab 15 kW)
* **Netz**: Spannung, Strom, Frequenz und Leistung je Phase, Netzleistung, Bezug,
  Einspeisung, Hausverbrauch
* **Backup und Last** (Hybrid): Backup-Ausgang je Phase, Last je Phase, USV-Auslastung
* **Batterie, Batterie 2, BMS-Daten** (Hybrid)
* **Energie**: Erträge, Bezug, Einspeisung, Verbrauch, Batterie geladen/entladen
* **Zähler**: Werte des GoodWe-Zählers (GM1000/GM3000) je Phase, Zähler 2
* **Einstellungen**: schreibbare Funktionen, je Funktion ein Eingang und ein Statusobjekt

Netzleistung, Bezug, Einspeisung und Hausverbrauch sind **nur mit angeschlossenem
GoodWe-Zähler** gültig. Ohne Zähler kennt der Wechselrichter den Netzanschlusspunkt nicht.

Vorzeichen: Netzleistung positiv = Einspeisung, Batterieleistung positiv = Entladen.

Einstellungen wirken erst, wenn ein Telegramm eintrifft; beim Start wird nichts geschrieben.
Der Betriebsmodus kennt 0 = Allgemein, 1 = Inselbetrieb, 2 = Backup, 3 = Eco, 4 = Peak
Shaving, 5 = Eigenverbrauch (745-Plattform), 6 = Eco Laden, 7 = Eco Entladen. „Eco Laden"
und „Eco Entladen" sind keine Modi des Wechselrichters: wie in Home Assistant wird dafür Eco-
Gruppe 1 als Dauerauftrag mit Eco-Leistung und Eco-Ziel-SoC überschrieben und die Gruppen 2-4
werden abgeschaltet. Eine in SolarGo angelegte Gruppe 1 geht dabei verloren.

Manche Funktionen gibt es erst ab ARM-Firmware 19 (Schnellladen, Lastregelung) bzw. 22
(Entladetiefe halten, Peak Shaving). Der Assistent hakt nur an, was das Gerät kennt.

<!-- DOCEND -->

# Kommunikationsobjekte

Die KO-Nummern ergeben sich aus der Reihenfolge in `tools/slots.py`. Je Kanal:

* KO 0: Erreichbar (DPT 1.011)
* KO 1-273: Messwerte
* KO 300-341: Einstellungen, je Funktion Eingang (gerade Nummer) und Status (ungerade)

Das Diagnoseobjekt liegt modulweit vor den Kanälen.

# Konsole

* `gdw` - Status aller Kanäle
* `gdwread <Kanal> <Start> [Anzahl]` - Register lesen, dezimal, z.B. `gdwread 1 35000 33`
* `gdwscan <Kanal>` - Assistent mit der IP des Kanals ausführen und das Ergebnis ausgeben
