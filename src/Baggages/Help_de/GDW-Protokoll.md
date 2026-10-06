### Protokoll

* **UDP (Port 8899)**: ältere WiFi-Kits und LAN-Kits. Das ist der Weg, den auch die
  SolarGo-App nutzt.
* **Modbus TCP (Port 502)**: LAN-Kit V2.0 (WLA0000-01-00P) und Kit-20 (WLA0000-03). Das
  Kit-20 kann **nur** TCP. Modbus TCP muss in SolarGo eingeschaltet sein.

Die Suche von „Wechselrichter auslesen" probiert erst UDP, dann TCP - wie Home Assistant.

