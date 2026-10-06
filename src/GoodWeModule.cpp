#include "GoodWeModule.h"
#include "GoodWeChannel.h"
#include "NetworkModule.h"
#include "knxprod.h"
#include <stdio.h>
#include <string.h>

GoodWeModule::GoodWeModule()
    : GDWChannelOwnerModule(GDW_ChannelCount)
{
}

const std::string GoodWeModule::name()
{
    return "GoodWe";
}

const std::string GoodWeModule::version()
{
#ifdef MODULE_GoodWe_Version
    return MODULE_GoodWe_Version;
#else
    return "";
#endif
}

OpenKNX::Channel* GoodWeModule::createChannel(uint8_t _channelIndex /* in Makros verwendet */)
{
    // Nur aktivierte, nicht suspendierte Kanaele anlegen. Deaktivierte Kanaele sind in der
    // ETS nicht sichtbar und haben keine gueltige Konfiguration.
    if (ParamGDW_CHType == 0 || ParamGDW_CHSuspended)
        return nullptr;
    return new GoodWeChannel(_channelIndex);
}

GoodWeChannel* GoodWeModule::channelAt(uint8_t index)
{
    if (index >= getNumberOfChannels())
        return nullptr;
    return static_cast<GoodWeChannel*>(getChannel(index));
}

void GoodWeModule::loop(bool configured)
{
    // Waehrend der Assistent laeuft, ruhen die Kanaele: der Dongle vertraegt nur einen Client
    // zuverlaessig, und der Kanal-Poll ist selbst einer.
    if (!_assistant.running())
        GDWChannelOwnerModule::loop(configured);

    // Der Assistent laeuft auch ohne gueltige Konfiguration: er ist das Werkzeug, mit dem
    // eine solche ueberhaupt erst entsteht.
    _assistant.loop();
    if (_assistant.finished() && !_assistantReported)
        reportAssistant();
}

void GoodWeModule::reportAssistant()
{
    _assistantReported = true;
    const GoodWe::DeviceInfo& info = _assistant.info();
    if (_assistant.error() != GoodWeAssistant::ErrNone)
    {
        logInfoP("Assistent Kanal %d: Fehler %d", (int)_assistant.channel(), (int)_assistant.error());
        return;
    }
    logInfoP("Assistent Kanal %d: %s %s (%s), %s %d, Adresse %d, %d Objekte verfuegbar",
             (int)_assistant.channel(), info.family == GoodWe::FamilyEt ? "Hybrid" : "netzgekoppelt",
             info.model, info.serial, _assistant.protocol() == GoodWeClient::Tcp ? "TCP" : "UDP",
             _assistant.protocol() == GoodWeClient::Tcp ? 502 : 8899, (int)_assistant.address(),
             (int)_assistant.availableCount());
    if (_assistantFromConsole)
    {
        uint8_t result[GoodWeAssistant::RESULT_BYTES];
        _assistant.buildResult(result);
        const uint8_t* bits = result + 12;
        logIndentUp();
        for (uint16_t k = 0; k < GoodWe::SENSOR_COUNT; k++)
        {
            if (bits[k / 8] & (1 << (k % 8)))
                logInfoP("%s", GoodWe::SENSORS[k].name);
        }
        logIndentDown();
    }
}

// Alle Antworten beginnen mit [0]=Status (0 = angenommen) und [1]=fertig (0 = laeuft noch).
// Die Antwort auf CmdStatus ist groesser als eine APDU; das ETS-Skript holt sie ueber
// BASE_invokeFunctionPropertyWrapper ab, der stueckelt.
bool GoodWeModule::processFunctionProperty(uint8_t objectIndex, uint8_t propertyId, uint8_t length,
                                           uint8_t* data, uint8_t* resultData, uint8_t& resultLength)
{
    if (objectIndex != FUNCTION_OBJECT || propertyId != FUNCTION_PROPERTY || length < 1)
        return false;

    switch (data[0])
    {
        case CmdStart:
        {
            if (length < 6)
                return false;
            if (_assistant.running())
            {
                resultData[0] = 1; // abgelehnt, laeuft bereits
                resultLength = 1;
                return true;
            }
            // Die Adresse kommt aus dem Aufruf: das Geraet kennt nur die zuletzt
            // heruntergeladenen Parameter, nicht die gerade eingetippte IP.
            char host[16];
            snprintf(host, sizeof(host), "%u.%u.%u.%u", data[2], data[3], data[4], data[5]);
            _assistant.start(data[1], host, openknxNetwork.established());
            _assistantFromConsole = false;
            _assistantReported = false;
            logInfoP("Assistent Kanal %d: lese %s aus", (int)data[1], host);
            resultData[0] = 0;
            resultData[1] = 0;
            resultLength = 2;
            return true;
        }

        case CmdStatus:
        {
            resultData[0] = 0;
            if (_assistant.running())
            {
                resultData[1] = 0;
                resultLength = 2;
                return true;
            }
            resultData[1] = 1;
            resultLength = (uint8_t)(2 + _assistant.buildResult(resultData + 2));
            return true;
        }

        default:
            return false;
    }
}

void GoodWeModule::diagnose(uint8_t channelIndex, const char* text)
{
    if (!ParamGDW_GDWUseDiag)
        return;
    char buf[15];
    snprintf(buf, sizeof(buf), "GW%d %s", channelIndex + 1, text);
    KoGDW_GDWDiag.value(buf, DPT_String_8859_1);
}

void GoodWeModule::showHelp()
{
    openknx.console.printHelpLine("gdw", "GoodWe: Status aller Wechselrichter");
    openknx.console.printHelpLine("gdwread", "GoodWe: Register lesen, z.B. 'gdwread 1 35100 10' (Kanal, Start, Anzahl dezimal)");
    openknx.console.printHelpLine("gdwscan", "GoodWe: Assistent für einen Kanal ausführen, z.B. 'gdwscan 1'");
}

bool GoodWeModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "gdw")
    {
        logInfoP("GoodWe: %d von %d Kanälen aktiv", (int)getNumberOfUsedChannels(), (int)getNumberOfChannels());
        logIndentUp();
        for (uint8_t i = 0; i < getNumberOfChannels(); i++)
        {
            GoodWeChannel* ch = channelAt(i);
            if (ch == nullptr)
                continue;
            const GoodWe::DeviceInfo& info = ch->info();
            logInfoP("Kanal %d: %s, %s %s (%s), %d Messwerte, Blöcke 0x%03X%s", i + 1, ch->host(),
                     ch->reachable() ? "erreichbar" : "nicht erreichbar", info.model, info.serial,
                     (int)ch->activeSensors(), (unsigned)ch->blockMask(), ch->busy() ? ", Anfrage läuft" : "");
        }
        logIndentDown();
        return true;
    }

    // "gdwread <Kanal> <Start> [Anzahl]" - dezimal, wie in der GoodWe-Dokumentation
    if (cmd.rfind("gdwread", 0) == 0)
    {
        unsigned channel = 0, start = 0, count = 1;
        if (sscanf(cmd.c_str(), "gdwread %u %u %u", &channel, &start, &count) < 2)
        {
            logInfoP("Aufruf: gdwread <Kanal> <Start> [Anzahl], z.B. 'gdwread 1 35000 33'");
            return true;
        }
        GoodWeChannel* ch = (channel >= 1) ? channelAt((uint8_t)(channel - 1)) : nullptr;
        if (ch == nullptr || !ch->configured())
        {
            logInfoP("Kanal %u ist nicht aktiv", channel);
            return true;
        }
        if (count == 0 || count > GoodWeClient::MAX_REGISTERS)
        {
            logInfoP("Anzahl muss zwischen 1 und %d liegen", (int)GoodWeClient::MAX_REGISTERS);
            return true;
        }
        if (!ch->requestDump((uint16_t)start, (uint16_t)count))
            logInfoP("Kanal %u: Dump läuft bereits", channel);
        else
            logInfoP("Kanal %u: lese %u Register ab %u ...", channel, count, start);
        return true;
    }

    // "gdwscan <Kanal>" - Assistent mit der IP des Kanals
    if (cmd.rfind("gdwscan", 0) == 0)
    {
        unsigned channel = 0;
        if (sscanf(cmd.c_str(), "gdwscan %u", &channel) < 1)
        {
            logInfoP("Aufruf: gdwscan <Kanal>");
            return true;
        }
        GoodWeChannel* ch = (channel >= 1) ? channelAt((uint8_t)(channel - 1)) : nullptr;
        if (ch == nullptr || !ch->configured())
        {
            logInfoP("Kanal %u ist nicht aktiv", channel);
            return true;
        }
        if (_assistant.running())
        {
            logInfoP("Assistent läuft bereits");
            return true;
        }
        _assistant.start((uint8_t)channel, ch->host(), openknxNetwork.established());
        _assistantFromConsole = true;
        _assistantReported = false;
        logInfoP("Assistent Kanal %u: lese %s aus ...", channel, ch->host());
        return true;
    }

    return false;
}
