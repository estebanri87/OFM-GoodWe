### Assistent

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

