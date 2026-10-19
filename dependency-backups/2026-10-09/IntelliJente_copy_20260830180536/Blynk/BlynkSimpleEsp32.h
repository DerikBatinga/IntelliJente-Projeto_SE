/**
 * @file       BlynkSimpleEsp32.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Blynk transport for ESP32
 */

#ifndef BlynkSimpleEsp32_h
#define BlynkSimpleEsp32_h

#include <WiFi.h>
#include <WiFiClient.h>

// Transport layer
#include <Blynk/BlynkTransportEsp32.h>

#ifndef BLYNK_INFO_DEVICE
#define BLYNK_INFO_DEVICE "ESP32"
#endif

#define BLYNK_SEND_CHUNK 1024
#define BLYNK_BUFFERS_SIZE 1024

class BlynkEsp32
    : public BlynkProtocol<BlynkTransportEsp32>
{
public:
    BlynkEsp32(const char* auth, const char* ssid, const char* pass)
        : BlynkProtocol<BlynkTransportEsp32>(transport)
        , m_auth(auth)
        , m_ssid(ssid)
        , m_pass(pass)
    {}

    void begin() {
        WiFi.begin(m_ssid, m_pass);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
        }
    }

    bool isConnected() {
        return transport.connected();
    }

private:
    const char* m_auth;
    const char* m_ssid;
    const char* m_pass;
};

BlynkEsp32 blynk;

#define Blynk blynk

void BlynkBegin(const char* auth, const char* ssid, const char* pass) {
    blynk = BlynkEsp32(auth, ssid, pass);
    blynk.begin();
}

#endif