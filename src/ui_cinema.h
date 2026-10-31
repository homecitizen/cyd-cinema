#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "movies_db.h"
#include "config.h"

// ═══════════════════════════════════════════════════════════════════════════
//  COLORES ESTILO LETTERBOXD (RGB565)
// ═══════════════════════════════════════════════════════════════════════════
#define CLR_LB_BG          0x10E4   // #14181C - Carbon profundo
#define CLR_LB_CARD        0x2126   // #202830 - Panel oscuro
#define CLR_LB_BORDER      0x39E9   // #38424E - Borde sutil
#define CLR_LB_WHITE       0xFFFF   // #FFFFFF - Blanco puro
#define CLR_LB_MUTED       0x9D98   // #9AB0C0 - Gris azulado
#define CLR_LB_GREEN       0x072A   // #00E054 - Verde Letterboxd
#define CLR_LB_ORANGE      0xFC00   // #FF8000 - Naranja Letterboxd / Halloween
#define CLR_LB_CYAN        0x45FE   // #40BCF4 - Azul cian Letterboxd
#define CLR_LB_GOLD        0xFDC0   // #FFC800 - Dorado

// Dimensiones de la caratula/poster (108x162 px exactos)
#define POSTER_X           8
#define POSTER_Y           8
#define POSTER_W           108
#define POSTER_H           162

// Vista activa
enum DisplayView {
    VIEW_QUOTE,
    VIEW_SYNOPSIS
};

// ═══════════════════════════════════════════════════════════════════════════
//  SANITIZADOR DE CARACTERES (TILDES, ENES, CARACTERES ESPECIALES)
//  Convierte secuencias UTF-8 multibyte en caracteres ASCII legibles
//  para que las fuentes de TFT_eSPI no omitan ninguna letra.
// ═══════════════════════════════════════════════════════════════════════════
inline String cleanText(const char* input) {
    if (!input) return "";
    String out = "";
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        uint8_t c = (uint8_t)input[i];
        if (c < 0x80) {
            out += (char)c;
        } else if (c == 0xC3 && i + 1 < len) {
            uint8_t c2 = (uint8_t)input[++i];
            switch (c2) {
                case 0x81: out += 'A'; break; // Á
                case 0x89: out += 'E'; break; // É
                case 0x8D: out += 'I'; break; // Í
                case 0x93: out += 'O'; break; // Ó
                case 0x9A: out += 'U'; break; // Ú
                case 0x91: out += 'N'; break; // Ñ
                case 0xA1: out += 'a'; break; // á
                case 0xA9: out += 'e'; break; // é
                case 0xAD: out += 'i'; break; // í
                case 0xB3: out += 'o'; break; // ó
                case 0xBA: out += 'u'; break; // ú
                case 0xB1: out += 'n'; break; // ñ
                case 0xBC: out += 'u'; break; // ü
                default:   out += ' '; break;
            }
        } else if (c == 0xC2 && i + 1 < len) {
            uint8_t c2 = (uint8_t)input[++i];
            switch (c2) {
                case 0xA1: /* ¡ */ break;
                case 0xBF: /* ¿ */ break;
                case 0xAB: out += '"'; break; // «
                case 0xBB: out += '"'; break; // »
                default: break;
            }
        } else if (c == 0xE2 && i + 2 < len) {
            uint8_t c2 = (uint8_t)input[i+1];
            uint8_t c3 = (uint8_t)input[i+2];
            i += 2;
            if (c2 == 0x80 && (c3 == 0x9C || c3 == 0x9D)) out += '"';
            else if (c2 == 0x80 && (c3 == 0x98 || c3 == 0x99)) out += '\'';
            else if (c2 == 0x80 && c3 == 0xA6) out += "...";
            else if (c2 == 0x80 && c3 == 0x94) out += "-";
        }
    }
    return out;
}

class CinemaUI {
public:
    CinemaUI(TFT_eSPI& tftRef) : tft(tftRef), currentView(VIEW_QUOTE) {}

    void init() {
        tft.fillScreen(CLR_LB_BG);
    }

    void drawLetterboxdLogo(int x, int y) {
        // Los 3 circulos iconicos de Letterboxd: Naranja, Verde, Azul
        tft.fillCircle(x,     y, 3, CLR_LB_ORANGE);
        tft.fillCircle(x + 8, y, 3, CLR_LB_GREEN);
        tft.fillCircle(x + 16,y, 3, CLR_LB_CYAN);
    }

    void drawPlaceholderPoster(const MovieItem& movie) {
        tft.fillRoundRect(POSTER_X, POSTER_Y, POSTER_W, POSTER_H, 4, CLR_LB_CARD);
        tft.drawRoundRect(POSTER_X, POSTER_Y, POSTER_W, POSTER_H, 4, CLR_LB_BORDER);

        int cx = POSTER_X + POSTER_W / 2;
        int cy = POSTER_Y + 54;
        tft.drawCircle(cx, cy, 22, CLR_LB_MUTED);
        tft.drawCircle(cx, cy, 7, CLR_LB_MUTED);
        for (int a = 0; a < 360; a += 45) {
            float rad = a * 0.0174533f;
            tft.drawLine(cx + cos(rad) * 7, cy + sin(rad) * 7,
                         cx + cos(rad) * 22, cy + sin(rad) * 22, CLR_LB_MUTED);
        }

        tft.setTextColor(CLR_LB_WHITE, CLR_LB_CARD);
        tft.setTextDatum(TC_DATUM);
        tft.drawString(cleanText(movie.title), cx, POSTER_Y + 95, 2);

        char yearStr[10];
        snprintf(yearStr, sizeof(yearStr), "(%d)", movie.year);
        tft.setTextColor(CLR_LB_MUTED, CLR_LB_CARD);
        tft.drawString(yearStr, cx, POSTER_Y + 130, 2);
    }

    void drawRatingStars(int x, int y, float rating) {
        tft.setTextColor(CLR_LB_GREEN, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);

        char buf[20];
        snprintf(buf, sizeof(buf), "* %.1f / 5", rating);
        tft.drawString(buf, x, y, 2);
    }

    void drawBadge(int x, int y, const char* tag, MovieTheme theme) {
        uint16_t tagColor = CLR_LB_GREEN;
        if (theme == THEME_HALLOWEEN)   tagColor = CLR_LB_ORANGE;
        else if (theme == THEME_CHRISTMAS) tagColor = CLR_LB_CYAN;
        else if (theme == THEME_ANNIVERSARY) tagColor = CLR_LB_GOLD;

        String cTag = cleanText(tag);
        int len = cTag.length();
        int badgeW = len * 6 + 14;
        tft.fillRoundRect(x, y, badgeW, 16, 4, CLR_LB_CARD);
        tft.drawRoundRect(x, y, badgeW, 16, 4, tagColor);

        tft.setTextColor(tagColor, CLR_LB_CARD);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(cTag, x + 7, y + 2, 1);
    }

    void renderMovieInfo(const MovieItem& movie) {
        // Limpiar columna derecha completa
        int rx = POSTER_X + POSTER_W + 12;
        int rw = SCREEN_W - rx - 6;
        tft.fillRect(rx, 0, rw + 6, SCREEN_H, CLR_LB_BG);

        // Limpiar debajo del poster
        tft.fillRect(0, POSTER_Y + POSTER_H + 2, POSTER_X + POSTER_W + 4, SCREEN_H - (POSTER_Y + POSTER_H + 2), CLR_LB_BG);

        // 1. Valoracion y logo Letterboxd bajo el poster
        drawRatingStars(POSTER_X + 2, POSTER_Y + POSTER_H + 8, movie.rating);
        drawLetterboxdLogo(POSTER_X + POSTER_W - 22, POSTER_Y + POSTER_H + 16);

        // 2. Badge tematico
        drawBadge(rx, 8, movie.tag, movie.theme);

        // 3. Titulo de la pelicula
        tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        
        String title = cleanText(movie.title);
        if (title.length() > 16) {
            int splitIdx = title.lastIndexOf(' ', 16);
            if (splitIdx > 0) {
                tft.drawString(title.substring(0, splitIdx), rx, 30, 4);
                tft.drawString(title.substring(splitIdx + 1), rx, 54, 4);
            } else {
                tft.drawString(title.substring(0, 15) + "...", rx, 30, 4);
            }
        } else {
            tft.drawString(title, rx, 30, 4);
        }

        // 4. Meta: Ano y Director
        char metaStr[80];
        snprintf(metaStr, sizeof(metaStr), "%d . %s", movie.year, cleanText(movie.director).c_str());
        tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
        tft.drawString(metaStr, rx, 78, 2);

        // Genero
        char genreStr[60];
        snprintf(genreStr, sizeof(genreStr), "Genero: %s", cleanText(movie.genre).c_str());
        tft.drawString(genreStr, rx, 96, 2);

        // Linea divisoria
        tft.drawFastHLine(rx, 116, rw, CLR_LB_BORDER);

        // 5. Cita o Sinopsis
        if (currentView == VIEW_QUOTE) {
            drawQuoteView(rx, 126, rw, movie.quote);
        } else {
            drawSynopsisView(rx, 126, rw, movie.synopsis);
        }

        // 6. Pie de pagina navegable
        drawFooterNav();
    }

    void drawQuoteView(int x, int y, int w, const char* quote) {
        tft.setTextColor(CLR_LB_CYAN, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("CITA CELEBRE:", x, y, 1);

        tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
        String q = cleanText(quote);
        
        int curY = y + 15;
        while (q.length() > 0 && curY < 205) {
            if (q.length() <= 23) {
                tft.drawString(q, x, curY, 2);
                break;
            }
            int sp = q.lastIndexOf(' ', 23);
            if (sp <= 0) sp = 23;
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
        String s = cleanText(syn);
        int curY = y + 15;
        while (s.length() > 0 && curY < 205) {
            if (s.length() <= 25) {
                tft.drawString(s, x, curY, 2);
                break;
            }
            int sp = s.lastIndexOf(' ', 25);
            if (sp <= 0) sp = 25;
            tft.drawString(s.substring(0, sp), x, curY, 2);
            s = s.substring(sp);
            s.trim();
            curY += 18;
        }
    }

    void drawFooterNav() {
        int y = 222;
        tft.setTextColor(CLR_LB_BORDER, CLR_LB_BG);
        tft.setTextDatum(TR_DATUM);
        tft.drawString("Siguiente >", SCREEN_W - 8, y, 1);

        tft.setTextDatum(TL_DATUM);
        if (currentView == VIEW_QUOTE) {
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.drawString("[Ver Sinopsis]", POSTER_X + POSTER_W + 12, y, 1);
        } else {
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.drawString("[Ver Cita]", POSTER_X + POSTER_W + 12, y, 1);
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
