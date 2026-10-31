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

// ═══════════════════════════════════════════════════════════════════════════
//  SANITIZADOR DE CARACTERES (TILDES, ENES, CARACTERES ESPECIALES)
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
    CinemaUI(TFT_eSPI& tftRef) : tft(tftRef) {}

    void init() {
        tft.fillScreen(CLR_LB_BG);
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

    void renderMovie(const MovieItem& movie, bool isSynopsisView) {
        // ── 1. ZONA INFERIOR BAJO EL POSTER (X=8..116, Y=172..238) ────────
        // A) Logotipo Letterboxd (3 puntos) + texto
        int lx = 14;
        int ly = 177;
        tft.fillCircle(lx,      ly, 3, CLR_LB_ORANGE);
        tft.fillCircle(lx + 8,  ly, 3, CLR_LB_GREEN);
        tft.fillCircle(lx + 16, ly, 3, CLR_LB_CYAN);

        tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("LETTERBOXD", lx + 26, ly - 4, 1);

        // B) Pildora de valoracion con estrellas
        int ratingY = 193;
        tft.fillRoundRect(10, ratingY, 104, 18, 4, CLR_LB_CARD);
        tft.drawRoundRect(10, ratingY, 104, 18, 4, CLR_LB_GREEN);

        char buf[20];
        snprintf(buf, sizeof(buf), "* %.1f / 5", movie.rating);
        tft.setTextColor(CLR_LB_GREEN, CLR_LB_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(buf, 10 + 52, ratingY + 9, 2);

        // C) Boton interactivo para alternar vista (Cita / Sinopsis)
        int btnY = 217;
        tft.fillRoundRect(10, btnY, 104, 18, 3, CLR_LB_CARD);
        tft.drawRoundRect(10, btnY, 104, 18, 3, CLR_LB_BORDER);

        tft.setTextColor(CLR_LB_MUTED, CLR_LB_CARD);
        tft.setTextDatum(MC_DATUM);
        if (!isSynopsisView) {
            tft.drawString("[Ver Sinopsis]", 10 + 52, btnY + 9, 1);
        } else {
            tft.drawString("[Ver Cita]", 10 + 52, btnY + 9, 1);
        }

        // ── 2. COLUMNA DERECHA DE DETALLES (X=128..316) ─────────────────────
        int rx = POSTER_X + POSTER_W + 12; // 128
        int rw = SCREEN_W - rx - 8;

        // Badge tematico
        drawBadge(rx, 8, movie.tag, movie.theme);

        // Titulo de la pelicula (negrita Font 4)
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

        // Ano y Director
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

        // Cita o Sinopsis segun vista activa
        if (!isSynopsisView) {
            drawQuoteView(rx, 126, rw, movie.quote);
        } else {
            drawSynopsisView(rx, 126, rw, movie.synopsis);
        }

        // Indicador siguiente en esquina inferior derecha
        tft.setTextColor(CLR_LB_BORDER, CLR_LB_BG);
        tft.setTextDatum(TR_DATUM);
        tft.drawString("Siguiente >", SCREEN_W - 8, 222, 1);
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

private:
    TFT_eSPI& tft;
};
