#pragma once
#include "GDWChannelOwnerModule.h"
#include "GoodWeAssistant.h"
#include "OpenKNX.h"

class GoodWeChannel;

// Lokale Anbindung von GoodWe-Wechselrichtern (ET- und DT-Familie) ueber den WiFi/LAN-Dongle.
//
// Ein Kanal = ein Wechselrichter. Die gesamte Geraetelogik liegt im Kanal; das Modul verwaltet
// die Kanaele, den Inbetriebnahme-Assistenten, die Diagnosebefehle und das Diagnose-KO.
class GoodWeModule : public GDWChannelOwnerModule
{
  public:
    GoodWeModule();

    const std::string name() override;
    const std::string version() override;

    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* in Makros verwendet, nicht umbenennen */) override;

    void loop(bool configured) override;

    void showHelp() override;
    bool processCommand(const std::string cmd, bool diagnoseKo) override;

    // Gegenstelle des ETS-Assistenten. Belegt: 0x9E Common, 0x9F FileTransfer,
    // 0xA0 Network/Logic/Presence, 0xA1 SolarmanPV, 0xA2 IPCamera, 0xA3 HueGateway.
    bool processFunctionProperty(uint8_t objectIndex, uint8_t propertyId, uint8_t length,
                                 uint8_t* data, uint8_t* resultData, uint8_t& resultLength) override;

    // Kurzer Meldungstext auf das Diagnose-KO (falls aktiviert)
    void diagnose(uint8_t channelIndex, const char* text);

  private:
    static const uint8_t FUNCTION_OBJECT = 0xA4;
    static const uint8_t FUNCTION_PROPERTY = 1;

    enum Command : uint8_t
    {
        CmdStart = 1,  // [1] Kanal, [2..5] IPv4
        CmdStatus = 2, // laeuft noch / fertig + Ergebnis
    };

    GoodWeAssistant _assistant;
    bool _assistantFromConsole = false;
    bool _assistantReported = true;

    GoodWeChannel* channelAt(uint8_t index);
    void reportAssistant();
};

extern GoodWeModule openknxGoodWeModule;
