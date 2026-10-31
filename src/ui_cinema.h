#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "movies_db.h"
#include "config.h"

// ═══════════════════════════════════════════════════════════════════════════
//  COLORES ESTILO LETTERBOXD (RGB565)
// ═══════════════════════════════════════════════════════════════════════════
#define CLR_LB_BG          0x10E4   // #14181C - Carbón profundo
#define CLR_LB_CARD        0x2126   // #202830 - Panel oscuro
#define CLR_LB_BORDER      0x39E9   // #38424E - Borde sutil
#define CLR_LB_WHITE       0xFFFF   // #FFFFFF - Blanco puro
#define CLR_LB_MUTED       0x9D98   // #9AB0C0 - Gris azulado
#define CLR_LB_GREEN       0x072A   // #00E054 - Verde Letterboxd
#define CLR_LB_ORANGE      0xFC00   // #FF8000 - Naranja Letterboxd / Halloween
#define CLR_LB_CYAN        0x45FE   // #40BCF4 - Azul cian Letterboxd
#define CLR_LB_GOLD        0xFDC0   // #FFC800 - Dorado estrella

// Dimensiones de la carátula/póster
#define POSTER_X           8
#define POSTER_Y           8
#define POSTER_W           108
#define POSTER_H           162

// Vista activa
enum DisplayView {
    VIEW_QUOTE,
    VIEW_SYNOPSIS
};

class CinemaUI {
public:
    CinemaUI(TFT_eSPI& tftRef) : tft(tftRef), currentView(VIEW_QUOTE) {}

    void init() {
        tft.fillScreen(CLR_LB_BG);
    }

    void drawLetterboxdLogo(int x, int y) {
        // Los 3 círculos icónicos de Letterboxd: Naranja, Verde, Azul
        tft.fillCircle(x,     y, 3, CLR_LB_ORANGE);
        tft.fillCircle(x + 8, y, 3, CLR_LB_GREEN);
        tft.fillCircle(x + 16,y, 3, CLR_LB_CYAN);
    }

    void drawPlaceholderPoster(const MovieItem& movie) {
        // Fondo del póster en estilo tarjeta Letterboxd
        tft.fillRoundRect(POSTER_X, POSTER_Y, POSTER_W, POSTER_H, 6, CLR_LB_CARD);
        tft.drawRoundRect(POSTER_X, POSTER_Y, POSTER_W, POSTER_H, 6, CLR_LB_BORDER);

        // Icono de rollo de película en el centro
        int cx = POSTER_X + POSTER_W / 2;
        int cy = POSTER_Y + 54;
        tft.drawCircle(cx, cy, 22, CLR_LB_MUTED);
        tft.drawCircle(cx, cy, 7, CLR_LB_MUTED);
        for (int a = 0; a < 360; a += 45) {
            float rad = a * 0.0174533f;
            tft.drawLine(cx + cos(rad) * 7, cy + sin(rad) * 7,
                         cx + cos(rad) * 22, cy + sin(rad) * 22, CLR_LB_MUTED);
        }

        // Título abreviado en el póster
        tft.setTextColor(CLR_LB_WHITE, CLR_LB_CARD);
        tft.setTextDatum(TC_DATUM);
        tft.drawString(movie.title, cx, POSTER_Y + 95, 2);

        // Año en el póster
        char yearStr[10];
        snprintf(yearStr, sizeof(yearStr), "(%d)", movie.year);
        tft.setTextColor(CLR_LB_MUTED, CLR_LB_CARD);
        tft.drawString(yearStr, cx, POSTER_Y + 130, 2);
    }

    void drawRatingStars(int x, int y, float rating) {
        // Dibuja estrellas o valoración estilo Letterboxd (ej: "★ 4.3 / 5")
        tft.setTextColor(CLR_LB_GREEN, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);

        char buf[20];
        snprintf(buf, sizeof(buf), "★ %.1f / 5", rating);
        tft.drawString(buf, x, y, 2);
    }

    void drawBadge(int x, int y, const char* tag, MovieTheme theme) {
        uint16_t tagColor = CLR_LB_GREEN;
        if (theme == THEME_HALLOWEEN)   tagColor = CLR_LB_ORANGE;
        else if (theme == THEME_CHRISTMAS) tagColor = CLR_LB_CYAN;
        else if (theme == THEME_ANNIVERSARY) tagColor = CLR_LB_GOLD;

        int len = strlen(tag);
        int badgeW = len * 6 + 14;
        tft.fillRoundRect(x, y, badgeW, 16, 4, CLR_LB_CARD);
        tft.drawRoundRect(x, y, badgeW, 16, 4, tagColor);

        tft.setTextColor(tagColor, CLR_LB_CARD);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(tag, x + 7, y + 2, 1);
    }

    void renderMovie(const MovieItem& movie, bool hasPosterImage = false) {
        // Limpiar área de contenido
        tft.fillRect(POSTER_X + POSTER_W + 4, 0, SCREEN_W - (POSTER_X + POSTER_W + 4), SCREEN_H, CLR_LB_BG);
        tft.fillRect(0, POSTER_Y + POSTER_H + 2, POSTER_X + POSTER_W + 4, SCREEN_H - (POSTER_Y + POSTER_H + 2), CLR_LB_BG);

        // 1. Póster (placeholder si no hay imagen descargada)
        if (!hasPosterImage) {
            drawPlaceholderPoster(movie);
        }

        // 2. Valoración y logo Letterboxd bajo el póster
        drawRatingStars(POSTER_X + 2, POSTER_Y + POSTER_H + 8, movie.rating);
        drawLetterboxdLogo(POSTER_X + POSTER_W - 22, POSTER_Y + POSTER_H + 16);

        // 3. Columna derecha: Información
        int rx = POSTER_X + POSTER_W + 12;
        int rw = SCREEN_W - rx - 8;

        // Badge temático
        drawBadge(rx, 8, movie.tag, movie.theme);

        // Título de la película
        tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        
        // Si el título es largo, partir en 2 líneas
        String title = String(movie.title);
        if (title.length() > 17) {
            int splitIdx = title.lastIndexOf(' ', 17);
            if (splitIdx > 0) {
                tft.drawString(title.substring(0, splitIdx), rx, 30, 4);
                tft.drawString(title.substring(splitIdx + 1), rx, 54, 4);
            } else {
                tft.drawString(title.substring(0, 16) + "…", rx, 30, 4);
            }
        } else {
            tft.drawString(title, rx, 30, 4);
        }

        // Año, Director y Género
        char metaStr[80];
        snprintf(metaStr, sizeof(metaStr), "%d · %s", movie.year, movie.director);
        tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
        tft.drawString(metaStr, rx, 78, 2);

        char genreStr[40];
        snprintf(genreStr, sizeof(genreStr), "Género: %s", movie.genre);
        tft.drawString(genreStr, rx, 96, 2);

        // Línea divisoria elegante
        tft.drawFastHLine(rx, 116, rw, CLR_LB_BORDER);

        // 4. Contenido dinámico según la vista (Cita o Sinopsis)
        if (currentView == VIEW_QUOTE) {
            drawQuoteView(rx, 126, rw, movie.quote);
        } else {
            drawSynopsisView(rx, 126, rw, movie.synopsis);
        }

        // 5. Barra inferior con indicador táctil
        drawFooterNav();
    }

    void drawQuoteView(int x, int y, int w, const char* quote) {
        tft.setTextColor(CLR_LB_CYAN, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("CITA CÉLEBRE:", x, y, 1);

        tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
        String q = String(quote);
        
        // Renderizado con salto de línea suave
        int curY = y + 14;
        while (q.length() > 0 && curY < 200) {
            if (q.length() <= 24) {
                tft.drawString(q, x, curY, 2);
                break;
            }
            int sp = q.lastIndexOf(' ', 24);
            if (sp <= 0) sp = 24;
            tft.drawString(q.substring(0, sp), x, curY, 2);
            q = q.substring(sp);
            q.trim();
            curY += 18;
        }
    }

    void drawSynopsisView(int x, int y, int w, const char* syn) {
        tft.setTextColor(CLR_LB_ORANGE, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("SINOPSIS:", x, y, 1);

        tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
        String s = String(syn);
        int curY = y + 14;
        while (s.length() > 0 && curY < 200) {
            if (s.length() <= 26) {
                tft.drawString(s, x, curY, 2);
                break;
            }
            int sp = s.lastIndexOf(' ', 26);
            if (sp <= 0) sp = 26;
            tft.drawString(s.substring(0, sp), x, curY, 2);
            s = s.substring(sp);
            s.trim();
            curY += 18;
        }
    }

    void drawFooterNav() {
        int y = 222;
        tft.setTextColor(CLR_LB_BORDER, CLR_LB_BG);
        tft.setTextDatum(TC_DATUM);
        tft.drawString("Toca pantalla para siguiente ►", SCREEN_W / 2 + 50, y, 1);

        // Indicador de modo (Cita / Sinopsis)
        tft.setTextDatum(TL_DATUM);
        if (currentView == VIEW_QUOTE) {
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.drawString("[Ver Sinopsis]", POSTER_X + 2, y, 1);
        } else {
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.drawString("[Ver Cita]", POSTER_X + 2, y, 1);
        }
    }

    void toggleView() {
        currentView = (currentView == VIEW_QUOTE) ? VIEW_SYNOPSIS : VIEW_QUOTE;
    }

    DisplayView getView() const {
        return currentView;
    }

private:
    TFT_eSPI& tft;
    DisplayView currentView;
};
