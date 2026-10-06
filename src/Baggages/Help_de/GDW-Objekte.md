### Objekte

Jedes Objekt hat ein eigenes Häkchen. Nur angehakte Objekte werden als KO angelegt und
gesendet. Die Objekte sind in Gruppen eingeteilt; erst wenn der Gruppenschalter angehakt
ist, erscheinen ihre Zeilen. Zuklappen ändert nichts an der Auswahl - angehakte Objekte
bleiben als KO erhalten. Die Gruppen entsprechen den Daten der goodwe-Bibliothek:

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

