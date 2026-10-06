# Changelog

## 0.1.0

- Erste Version: GoodWe Hybrid (ET-Familie) und netzgekoppelt (DT-Familie) über UDP 8899 oder
  Modbus TCP 502.
- 274 Messwerte und 21 Einstellungen aus der goodwe-Bibliothek 0.4.10, jedes Objekt einzeln
  wählbar.
- ETS-Assistent „Wechselrichter auslesen": erkennt Protokoll, Typ, Modbus-Adresse und
  verfügbare Objekte.
- Kanalauswahl als eigener Tab (Typ-Variante mit `BASE_SyncChannelType`).
- Ohne Testgerät entwickelt, Register ungeprüft.
- ETS-Darstellung der Objektauswahl korrigiert: je Objekt eine normale Parameterzeile mit
  Text und Häkchen statt einer Tabelle mit nur einer Spalte, in der die Häkchen unsichtbar
  waren. Einheiten stehen jetzt im Zeilentext.
