### Abfrage

**Abfrageintervall** (Sekunden, mindestens 10): Abstand zwischen zwei Lesezyklen. Gelesen
werden nur die Registerblöcke, in denen ein angehaktes Objekt liegt.

Zu schnelles Abfragen kann die Übertragung an die SEMS-Cloud stoppen und manche Dongles zum
Absturz bringen. Wer SEMS weiter nutzt, sollte 60 Sekunden oder mehr wählen. Einstellungen
werden alle 5 Minuten und 2 Sekunden nach jedem Schreiben gelesen. Alle 10 Minuten geht ein
Weckruf an den Dongle (UDP 48899), weil manche Dongles nachts einfrieren.

