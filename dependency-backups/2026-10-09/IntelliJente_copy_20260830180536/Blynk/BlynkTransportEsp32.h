/**
 * @file       BlynkTransportEsp32.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      ESP32 transport for Blynk
 */

#ifndef BlynkTransportEsp32_h
#define BlynkTransportEsp32_h

#include <WiFi.h>
#include <WiFiClient.h>

class BlynkTransportEsp32
{
public:
    BlynkTransportEsp32() {}

    int connect(const char* host, uint16_t port) {
        m_client.connect(host, port);
        return m_client.connected() ? 1 : 0;
    }

    size_t write(const uint8_t* data, size_t len) {
        return m_client.write(data, len);
    }

    int read(uint8_t* data, size_t len) {
        return m_client.read(data, len);
    }

    bool connected() {
        return m_client.connected();
    }

    void disconnect() {
        m_client.stop();
    }

    int available() {
        return m_client.available();
    }

private:
    WiFiClient m_client;
};

#endif