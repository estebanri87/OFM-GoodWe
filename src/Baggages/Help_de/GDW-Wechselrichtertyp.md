### Wechselrichtertyp

* **Automatisch erkennen**: Die Firmware fragt beim Start zuerst die Geräteinformation der
  Hybrid-Familie (Register 35000), dann die der netzgekoppelten Familie (30001) ab.
* **Hybrid (ET, EH, BT, BH, ES-G2)**: Wechselrichter mit Batterieanschluss.
* **Netzgekoppelt (DT, MS, D-NS, XS)**: reine PV-Wechselrichter. Sie schalten nachts ab und
  sind dann nicht erreichbar - das ist normal.

„Wechselrichter auslesen" setzt den Typ automatisch.

