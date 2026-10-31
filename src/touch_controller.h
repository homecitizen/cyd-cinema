#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "config.h"

// Rango típico de calibración del panel táctil resistivo de la CYD 2.8"
#define TS_MINX 200
#define TS_MAXX 3800
#define TS_MINY 240
#define TS_MAXY 3800

class CinemaTouch {
public:
    CinemaTouch() : touchSPI(HSPI), ts(XPT2046_CS, XPT2046_IRQ) {}

    void begin() {
        touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
        ts.begin(touchSPI);
        ts.setRotation(1); // Mismo sentido landscape que la pantalla
    }

    bool checkTouch(int& screenX, int& screenY) {
        if (!ts.touched()) return false;

        unsigned long now = millis();
        if (now - lastTouchTime < 350) { // Antirrebote
            return false;
        }

        TS_Point p = ts.getPoint();
        if (p.z < 200) return false; // Descartar roces leves

        // Mapear coordenadas analógicas a píxeles 320x240
        int x = map(p.x, TS_MINX, TS_MAXX, 0, SCREEN_W);
        int y = map(p.y, TS_MINY, TS_MAXY, 0, SCREEN_H);

        x = constrain(x, 0, SCREEN_W - 1);
        y = constrain(y, 0, SCREEN_H - 1);

        screenX = x;
        screenY = y;
        lastTouchTime = now;
        return true;
    }

private:
    SPIClass touchSPI;
    XPT2046_Touchscreen ts;
    unsigned long lastTouchTime = 0;
};
