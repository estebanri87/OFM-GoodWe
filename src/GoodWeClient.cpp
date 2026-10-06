// KNX-frei halten: hier darf kein OpenKNX-/knx-Header hinein.
#include "GoodWeClient.h"
#include <Arduino.h>
#include <errno.h>
#include <fcntl.h>
#include <lwip/sockets.h>
#include <string.h>
#include <unistd.h> // close()

namespace
{
constexpr uint8_t FC_READ = 0x03;
constexpr uint8_t FC_WRITE = 0x06;
constexpr uint8_t FC_WRITE_MULTI = 0x10;

// Je Versuch. Die Bibliothek nutzt 1 s; die Dongles antworten unter Last aber auch mal
// spaeter. Wird nur ueber millis() geprueft, nie gewartet.
constexpr uint32_t ATTEMPT_TIMEOUT_MS = 2000;
// UDP: ein zweiter Versuch. Mehr nicht - Wiederholungsstuerme bringen manche Dongles zum
// Absturz (mletenay/home-assistant-goodwe-inverter #454).
constexpr uint8_t UDP_ATTEMPTS = 2;
constexpr uint8_t TCP_ATTEMPTS = 1;

constexpr uint16_t WAKEUP_PORT = 48899;
constexpr char WAKEUP_TEXT[] = "WIFIKIT-214028-READ";

inline uint16_t be16(const uint8_t* p)
{
    return (uint16_t)((p[0] << 8) | p[1]);
}
} // namespace

uint16_t GoodWeClient::crc16(const uint8_t* data, size_t len)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++)
            crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0xA001) : (uint16_t)(crc >> 1);
    }
    return crc;
}

void GoodWeClient::configure(const char* host, uint16_t port, Protocol protocol, uint8_t address)
{
    _host[0] = 0;
    if (host != nullptr)
    {
        strncpy(_host, host, sizeof(_host) - 1);
        _host[sizeof(_host) - 1] = 0;
    }
    _port = port != 0 ? port : (protocol == Tcp ? 502 : 8899);
    _protocol = protocol;
    _address = address != 0 ? address : 0xF7;
}

bool GoodWeClient::beginRead(uint16_t start, uint16_t count)
{
    if (busy())
        return false;
    if (count == 0 || count > MAX_REGISTERS)
    {
        fail(ErrArgs);
        return false;
    }
    return begin(FC_READ, start, count, nullptr, 0);
}

bool GoodWeClient::beginWrite(uint16_t reg, uint16_t value)
{
    return begin(FC_WRITE, reg, value, nullptr, 0);
}

bool GoodWeClient::beginWriteMulti(uint16_t reg, const uint16_t* values, uint8_t count)
{
    if (busy())
        return false;
    if (values == nullptr || count == 0 || count > MAX_WRITE_REGISTERS)
    {
        fail(ErrArgs);
        return false;
    }
    return begin(FC_WRITE_MULTI, reg, count, values, count);
}

bool GoodWeClient::begin(uint8_t fc, uint16_t reg, uint16_t value, const uint16_t* values, uint8_t count)
{
    if (busy())
        return false;
    if (!configured())
    {
        fail(ErrNotConfigured);
        return false;
    }

    _fc = fc;
    _reqReg = reg;
    _reqValue = value;
    _regCount = 0;
    _exception = 0;
    _result = Pending;

    // PDU zusammenbauen; bei UDP mit Adresse vorne und CRC hinten, bei TCP mit MBAP-Kopf.
    uint8_t pdu[6 + 1 + 2 * MAX_WRITE_REGISTERS];
    size_t n = 0;
    pdu[n++] = fc;
    pdu[n++] = (uint8_t)(reg >> 8);
    pdu[n++] = (uint8_t)reg;
    pdu[n++] = (uint8_t)(value >> 8);
    pdu[n++] = (uint8_t)value;
    if (fc == FC_WRITE_MULTI)
    {
        pdu[n++] = (uint8_t)(count * 2);
        for (uint8_t i = 0; i < count; i++)
        {
            pdu[n++] = (uint8_t)(values[i] >> 8);
            pdu[n++] = (uint8_t)values[i];
        }
    }

    size_t i = 0;
    if (_protocol == Tcp)
    {
        _tid++;
        const uint16_t len = (uint16_t)(1 + n); // Unit + PDU
        _request[i++] = (uint8_t)(_tid >> 8);
        _request[i++] = (uint8_t)_tid;
        _request[i++] = 0;
        _request[i++] = 0;
        _request[i++] = (uint8_t)(len >> 8);
        _request[i++] = (uint8_t)len;
        _request[i++] = _address;
        memcpy(_request + i, pdu, n);
        i += n;
    }
    else
    {
        _request[i++] = _address;
        memcpy(_request + i, pdu, n);
        i += n;
        const uint16_t crc = crc16(_request, i);
        _request[i++] = (uint8_t)crc; // LSB zuerst
        _request[i++] = (uint8_t)(crc >> 8);
    }
    _requestLen = i;

    _attemptsLeft = (_protocol == Tcp) ? TCP_ATTEMPTS : UDP_ATTEMPTS;
    return startAttempt();
}

bool GoodWeClient::startAttempt()
{
    closeSocket();
    _attemptsLeft--;
    _sent = 0;
    _received = 0;
    if (!openSocket())
    {
        fail(ErrSocket);
        return false;
    }
    _deadline = millis() + ATTEMPT_TIMEOUT_MS;
    _state = (_protocol == Tcp) ? Connecting : Sending;
    return true;
}

bool GoodWeClient::openSocket()
{
    _sock = (_protocol == Tcp) ? socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)
                               : socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sock < 0)
        return false;

    const int flags = fcntl(_sock, F_GETFL, 0);
    if (flags < 0 || fcntl(_sock, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        closeSocket();
        return false;
    }

    if (_protocol == Udp)
        return true; // sendto() adressiert jedes Paket selbst

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    if (inet_pton(AF_INET, _host, &addr.sin_addr) != 1)
    {
        closeSocket();
        return false;
    }
    const int rc = connect(_sock, (struct sockaddr*)&addr, sizeof(addr));
    if (rc == 0 || errno == EINPROGRESS || errno == EALREADY)
        return true;
    closeSocket();
    return false;
}

void GoodWeClient::closeSocket()
{
    if (_sock >= 0)
    {
        close(_sock);
        _sock = -1;
    }
}

void GoodWeClient::fail(Result reason)
{
    closeSocket();
    _result = reason;
    _state = Complete;
}

void GoodWeClient::retryOrFail(Result reason)
{
    if (_attemptsLeft > 0 && startAttempt())
        return;
    if (_state != Complete)
        fail(reason);
}

void GoodWeClient::poll()
{
    if (_state == Idle || _state == Complete)
        return;

    if ((int32_t)(millis() - _deadline) >= 0)
    {
        retryOrFail(ErrTimeout);
        return;
    }

    switch (_state)
    {
        case Connecting:
        {
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(_sock, &wfds);
            struct timeval tv = {0, 0}; // nie warten
            const int rc = select(_sock + 1, nullptr, &wfds, nullptr, &tv);
            if (rc < 0)
            {
                fail(ErrSocket);
                return;
            }
            if (rc == 0 || !FD_ISSET(_sock, &wfds))
                return; // noch nicht verbunden

            int soErr = 0;
            socklen_t len = sizeof(soErr);
            if (getsockopt(_sock, SOL_SOCKET, SO_ERROR, &soErr, &len) < 0 || soErr != 0)
            {
                fail(ErrConnect);
                return;
            }
            _state = Sending;
            return;
        }

        case Sending:
        {
            int n;
            if (_protocol == Udp)
            {
                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(_port);
                if (inet_pton(AF_INET, _host, &addr.sin_addr) != 1)
                {
                    fail(ErrSocket);
                    return;
                }
                n = sendto(_sock, _request, _requestLen, 0, (struct sockaddr*)&addr, sizeof(addr));
                if (n == (int)_requestLen)
                    _sent = _requestLen;
            }
            else
            {
                n = send(_sock, _request + _sent, _requestLen - _sent, 0);
                if (n > 0)
                    _sent += (size_t)n;
            }
            if (n < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
            {
                retryOrFail(ErrSocket);
                return;
            }
            if (_sent >= _requestLen)
                _state = Receiving;
            return;
        }

        case Receiving:
        {
            while (_received < sizeof(_response))
            {
                int n;
                if (_protocol == Udp)
                {
                    struct sockaddr_in from;
                    socklen_t fromLen = sizeof(from);
                    n = recvfrom(_sock, _response + _received, sizeof(_response) - _received, 0,
                                 (struct sockaddr*)&from, &fromLen);
                    if (n > 0)
                    {
                        struct in_addr expected;
                        inet_pton(AF_INET, _host, &expected);
                        if (from.sin_addr.s_addr != expected.s_addr)
                            continue; // fremdes Paket
                    }
                }
                else
                {
                    n = recv(_sock, _response + _received, sizeof(_response) - _received, 0);
                    if (n == 0)
                    {
                        // Gegenstelle hat geschlossen - auswerten, was da ist.
                        parse();
                        return;
                    }
                }
                if (n < 0)
                {
                    if (errno != EWOULDBLOCK && errno != EAGAIN)
                        retryOrFail(ErrSocket);
                    break;
                }
                _received += (size_t)n;
            }
            if (_state != Receiving)
                return;

            // Antworten kommen gelegentlich in zwei Datagrammen; erst auswerten, wenn die
            // angekuendigte Laenge erreicht ist (wie PartialResponseException der Bibliothek).
            const size_t expected = expectedLength();
            if (expected != 0 && _received >= expected)
                parse();
            return;
        }

        default:
            return;
    }
}

size_t GoodWeClient::expectedLength() const
{
    if (_protocol == Tcp)
    {
        // TID(2) Proto(2) Len(2) Unit(1) FC(1) ...
        if (_received < 9)
            return 0;
        const uint8_t fc = _response[7];
        if (fc & 0x80)
            return 9;
        if (fc == FC_READ)
            return 9 + (size_t)_response[8];
        return 12;
    }

    // AA 55 Adr FC ...
    if (_received < 5)
        return 0;
    const uint8_t fc = _response[3];
    if (fc & 0x80)
        return 7;
    if (fc == FC_READ)
        return 7 + (size_t)_response[4];
    return 10;
}

void GoodWeClient::parse()
{
    closeSocket();
    _state = Complete;

    const size_t expected = expectedLength();
    if (expected == 0 || _received < expected)
    {
        _result = ErrFrame;
        return;
    }

    const uint8_t* pdu; // ab FC
    if (_protocol == Tcp)
    {
        if (_response[2] != 0 || _response[3] != 0)
        {
            _result = ErrFrame;
            return;
        }
        pdu = _response + 7;
    }
    else
    {
        if (_response[0] != 0xAA || _response[1] != 0x55)
        {
            _result = ErrFrame;
            return;
        }
        // CRC ueber den Rahmen ohne AA 55, LSB zuerst
        const uint16_t crcCalc = crc16(_response + 2, expected - 4);
        const uint16_t crcRecv = (uint16_t)(_response[expected - 2] | (_response[expected - 1] << 8));
        if (crcCalc != crcRecv)
        {
            _result = ErrCrc;
            return;
        }
        pdu = _response + 3;
    }

    const uint8_t fc = pdu[0];
    if (fc != _fc)
    {
        if (fc == (uint8_t)(_fc | 0x80))
        {
            _exception = pdu[1];
            _result = ErrException;
        }
        else
        {
            _result = ErrMismatch;
        }
        return;
    }

    if (fc == FC_READ)
    {
        const uint8_t byteCount = pdu[1];
        if (byteCount != (uint8_t)(_reqValue * 2))
        {
            _result = ErrMismatch;
            return;
        }
        for (uint16_t i = 0; i < _reqValue; i++)
            _regs[i] = be16(pdu + 2 + 2 * i);
        _regCount = _reqValue;
        _result = Ok;
        return;
    }

    // Schreibantworten wiederholen Register und Wert bzw. Anzahl.
    if (be16(pdu + 1) != _reqReg || be16(pdu + 3) != _reqValue)
    {
        _result = ErrMismatch;
        return;
    }
    _result = Ok;
}

void GoodWeClient::clear()
{
    closeSocket();
    _state = Idle;
    _received = 0;
    _sent = 0;
}

bool GoodWeClient::sendWakeup(const char* host)
{
    if (host == nullptr || host[0] == 0)
        return false;
    const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
        return false;
    const int flags = fcntl(sock, F_GETFL, 0);
    if (flags >= 0)
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(WAKEUP_PORT);
    bool ok = false;
    if (inet_pton(AF_INET, host, &addr.sin_addr) == 1)
        ok = sendto(sock, WAKEUP_TEXT, sizeof(WAKEUP_TEXT) - 1, 0, (struct sockaddr*)&addr, sizeof(addr)) > 0;
    close(sock);
    return ok;
}

const char* GoodWeClient::resultText(Result result)
{
    switch (result)
    {
        case Ok: return "OK";
        case Pending: return "laeuft";
        case ErrNotConfigured: return "nicht konfiguriert";
        case ErrBusy: return "bereits aktiv";
        case ErrSocket: return "Socket-Fehler";
        case ErrConnect: return "Verbindung abgelehnt";
        case ErrTimeout: return "Zeitueberschreitung";
        case ErrFrame: return "ungueltiger Rahmen";
        case ErrCrc: return "CRC falsch";
        case ErrException: return "Modbus-Exception";
        case ErrMismatch: return "Antwort passt nicht";
        case ErrArgs: return "ungueltige Anzahl";
    }
    return "unbekannt";
}
