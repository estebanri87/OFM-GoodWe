#include "GDWChannelOwnerModule.h"

GDWChannelOwnerModule::GDWChannelOwnerModule(uint8_t numberOfChannels)
    : _numberOfChannels(numberOfChannels)
{
    if (_numberOfChannels > 0)
        _pChannels = new OpenKNX::Channel*[numberOfChannels]();
}

GDWChannelOwnerModule::~GDWChannelOwnerModule()
{
    if (_pChannels != nullptr)
    {
        delete[] _pChannels;
        _pChannels = nullptr;
    }
}

void GDWChannelOwnerModule::setup(bool configured)
{
    OpenKNX::Module::setup(configured);
}

OpenKNX::Channel* GDWChannelOwnerModule::createChannel(uint8_t _channelIndex)
{
    return nullptr;
}

void GDWChannelOwnerModule::setup()
{
    OpenKNX::Module::setup();
    if (_pChannels != nullptr)
    {
        logDebugP("Setting up %d channels", _numberOfChannels);
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
        {
            logDebugP("Create channel %d", _channelIndex);
            logIndentUp();
            _pChannels[_channelIndex] = createChannel(_channelIndex);
            logIndentDown();
        }
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
        {
            OpenKNX::Channel* channel = _pChannels[_channelIndex];
            if (channel != nullptr)
            {
                channel->init();
                channel->setup(true);
                channel->setup();
            }
        }
    }
}

void GDWChannelOwnerModule::loop(bool configured)
{
    OpenKNX::Module::loop(configured);
    if (_pChannels != nullptr)
    {
        uint8_t processed = 0;
        do
        {
            OpenKNX::Channel* channel = _pChannels[_currentChannel];
            if (channel != nullptr)
                channel->loop(configured);
        }
        while (openknx.freeLoopIterate(_numberOfChannels, _currentChannel, processed));
    }
}

void GDWChannelOwnerModule::loop()
{
    OpenKNX::Module::loop();
    if (_pChannels != nullptr)
    {
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
        {
            OpenKNX::Channel* channel = _pChannels[_channelIndex];
            if (channel != nullptr)
                channel->loop();
        }
    }
}

OpenKNX::Channel* GDWChannelOwnerModule::getChannel(uint8_t channelIndex)
{
    return _pChannels != nullptr ? _pChannels[channelIndex] : nullptr;
}

uint8_t GDWChannelOwnerModule::getNumberOfChannels()
{
    return _numberOfChannels;
}

uint8_t GDWChannelOwnerModule::getNumberOfUsedChannels()
{
    uint8_t activeChannels = 0;
    if (_pChannels != nullptr)
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
            if (_pChannels[_channelIndex] != nullptr)
                activeChannels++;
    return activeChannels;
}

#ifdef OPENKNX_DUALCORE
void GDWChannelOwnerModule::setup1(bool configured)
{
    OpenKNX::Module::setup1(configured);
}

void GDWChannelOwnerModule::setup1()
{
    OpenKNX::Module::setup1();
    if (_pChannels != nullptr)
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
            if (_pChannels[_channelIndex] != nullptr)
                _pChannels[_channelIndex]->setup1();
}

void GDWChannelOwnerModule::loop1(bool configured)
{
    OpenKNX::Module::loop1(configured);
    if (_pChannels != nullptr)
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
            if (_pChannels[_channelIndex] != nullptr)
                _pChannels[_channelIndex]->loop1(configured);
}

void GDWChannelOwnerModule::loop1()
{
    OpenKNX::Module::loop1();
    if (_pChannels != nullptr)
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
            if (_pChannels[_channelIndex] != nullptr)
                _pChannels[_channelIndex]->loop1();
}
#endif

#if (MASK_VERSION & 0x0900) != 0x0900
void GDWChannelOwnerModule::processInputKo(GroupObject &ko)
{
    OpenKNX::Module::processInputKo(ko);
    if (_pChannels != nullptr)
        for (uint8_t _channelIndex = 0; _channelIndex < _numberOfChannels; _channelIndex++)
            if (_pChannels[_channelIndex] != nullptr)
                _pChannels[_channelIndex]->processInputKo(ko);
}
#endif
