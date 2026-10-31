#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <TJpg_Decoder.h>
#include <TFT_eSPI.h>
#include "ui_cinema.h"

// Puntero global para la funcion de callback de TJpg_Decoder
static TFT_eSPI* _globalTft = nullptr;

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (!_globalTft) return false;
    // Solo dibujar si entra dentro de la pantalla
    if (x < SCREEN_W && y < SCREEN_H) {
        _globalTft->pushImage(x, y, w, h, bitmap);
    }
    // IMPORTANTE: siempre retornar true para no abortar la decodificacion del JPEG
    return true;
}

class PosterFetcher {
public:
    PosterFetcher(TFT_eSPI& tftRef) : tft(tftRef) {
        _globalTft = &tft;
        TJpgDec.setJpgScale(1);
        TJpgDec.setSwapBytes(true);
        TJpgDec.setCallback(tft_output);
    }

    bool fetchAndDisplay(const char* url) {
        if (!url || strlen(url) < 8) return false;
        if (WiFi.status() != WL_CONNECTED) return false;

        WiFiClientSecure client;
        client.setInsecure(); // No validar certificados SSL para ahorrar RAM
        client.setTimeout(6);

        HTTPClient https;
        https.setUserAgent("ESP32-Cinema/1.0");

        if (!https.begin(client, url)) {
            Serial.println("[Poster] Error en https.begin");
            return false;
        }

        int httpCode = https.GET();
        if (httpCode != HTTP_CODE_OK) {
            Serial.printf("[Poster] HTTP GET fallo: %d\n", httpCode);
            https.end();
            return false;
        }

        int len = https.getSize();
        if (len <= 0 || len > 50000) {
            Serial.printf("[Poster] Tamano no compatible: %d bytes\n", len);
            https.end();
            return false;
        }

        uint8_t* buffer = (uint8_t*)malloc(len);
        if (!buffer) {
            Serial.println("[Poster] Memoria insuficiente en heap");
            https.end();
            return false;
        }

        WiFiClient* stream = https.getStreamPtr();
        int bytesRead = 0;
        int timeoutMs = 6000;
        unsigned long start = millis();

        while (https.connected() && bytesRead < len && (millis() - start < timeoutMs)) {
            size_t avail = stream->available();
            if (avail) {
                int readNow = stream->readBytes(buffer + bytesRead, len - bytesRead);
                bytesRead += readNow;
            } else {
                delay(10);
            }
        }

        https.end();

        if (bytesRead != len) {
            Serial.printf("[Poster] Descarga incompleta (%d/%d)\n", bytesRead, len);
            free(buffer);
            return false;
        }

        Serial.printf("[Poster] Decodificando JPEG (%d bytes)...\n", len);
        TJpgDec.drawJpg(POSTER_X, POSTER_Y, buffer, len);
        free(buffer);

        tft.drawRoundRect(POSTER_X - 1, POSTER_Y - 1, POSTER_W + 2, POSTER_H + 2, 4, CLR_LB_BORDER);
        return true;
    }

private:
    TFT_eSPI& tft;
};
