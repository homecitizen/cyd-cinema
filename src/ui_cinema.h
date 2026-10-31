#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "movies_db.h"

// ═══════════════════════════════════════════════════════════════════════════
//  PALETA DE COLORES LETTERBOXD (RGB565)
// ═══════════════════════════════════════════════════════════════════════════
#define CLR_LB_BG          0x10C3    // #14181C (Gris muy oscuro azulado de fondo)
#define CLR_LB_CARD        0x1926    // #1F252B (Contenedor de tarjeta)
#define CLR_LB_BORDER      0x29C8    // #2C343D (Bordes sutiles)
#define CLR_LB_GREEN       0x070A    // #00E054 (Verde neon Letterboxd)
#define CLR_LB_ORANGE      0xFC00    // #FF8000 (Naranja Letterboxd)
#define CLR_LB_CYAN        0x45FC    // #40BCF4 (Azul cian Letterboxd)
#define CLR_LB_WHITE       0xFFFF    // #FFFFFF (Blanco puro texto)
#define CLR_LB_MUTED       0x9CD3    // #9AB0BE (Texto secundario legible)
#define CLR_LB_GOLD        0xFE60    // #FFCC00 (Dorado / Estrellas)

// Dimensiones de la caratula/poster (108x162 px exactos)
#define POSTER_X           8
#define POSTER_Y           8
#define POSTER_W           108
#define POSTER_H           162

// Vistas disponibles en la columna de detalles
enum MovieView {
    VIEW_QUOTE = 0,      // Frase mitica
    VIEW_CURIOSITY = 1,  // Curiosidad y anecdotas de rodaje
    VIEW_SYNOPSIS = 2    // Sinopsis del argumento
};

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

class UICinema {
public:
    UICinema(TFT_eSPI& display) : tft(display) {}

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
        if (theme == THEME_HALLOWEEN)        tagColor = CLR_LB_ORANGE;
        else if (theme == THEME_CHRISTMAS)   tagColor = CLR_LB_CYAN;
        else if (theme == THEME_ANNIVERSARY) tagColor = CLR_LB_GOLD;
        else if (theme == THEME_OSCARS)      tagColor = CLR_LB_GOLD;

        String cTag = cleanText(tag);
        int len = cTag.length();
        int badgeW = len * 6 + 14;
        tft.fillRoundRect(x, y, badgeW, 16, 4, CLR_LB_CARD);
        tft.drawRoundRect(x, y, badgeW, 16, 4, tagColor);

        tft.setTextColor(tagColor, CLR_LB_CARD);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(cTag, x + 7, y + 2, 1);
    }

    void renderMovie(const MovieItem& movie, MovieView view, const char* dateStr, bool isFilmOfTheDay) {
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

        // C) Boton interactivo para alternar vista (Cita / Curiosidad / Sinopsis)
        int btnY = 217;
        tft.fillRoundRect(10, btnY, 104, 18, 3, CLR_LB_CARD);
        tft.drawRoundRect(10, btnY, 104, 18, 3, CLR_LB_BORDER);

        tft.setTextColor(CLR_LB_MUTED, CLR_LB_CARD);
        tft.setTextDatum(MC_DATUM);
        if (view == VIEW_QUOTE) {
            tft.drawString("[Curiosidad]", 10 + 52, btnY + 9, 1);
        } else if (view == VIEW_CURIOSITY) {
            tft.drawString("[Sinopsis]", 10 + 52, btnY + 9, 1);
        } else {
            tft.drawString("[Cita]", 10 + 52, btnY + 9, 1);
        }

        // ── 2. COLUMNA DERECHA DE DETALLES (X=128..316) ─────────────────────
        int rx = POSTER_X + POSTER_W + 12; // 128
        int rw = 320 - rx - 8;

        // Badge tematico o insignia "PELICULA DEL DIA"
        if (isFilmOfTheDay) {
            drawBadge(rx, 8, "PELICULA DEL DIA", THEME_ANNIVERSARY);
        } else {
            drawBadge(rx, 8, movie.tag, movie.theme);
        }

        // Fecha actual en la esquina superior derecha (ej: "31 OCT")
        if (dateStr && strlen(dateStr) > 0) {
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.setTextDatum(TR_DATUM);
            tft.drawString(dateStr, 320 - 8, 10, 1);
        }

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

        // Vista seleccionada: Cita, Curiosidad o Sinopsis
        if (view == VIEW_QUOTE) {
            drawQuoteView(rx, 124, rw, movie.quote);
        } else if (view == VIEW_CURIOSITY) {
            drawCuriosityView(rx, 124, rw, movie.curiosity);
        } else {
            drawSynopsisView(rx, 124, rw, movie.synopsis);
        }

        // Indicador siguiente en esquina inferior derecha
        tft.setTextColor(CLR_LB_BORDER, CLR_LB_BG);
        tft.setTextDatum(TR_DATUM);
        tft.drawString("Siguiente >", 320 - 8, 222, 1);
    }

    void drawQuoteView(int x, int y, int w, const char* quote) {
        tft.setTextColor(CLR_LB_CYAN, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("CITA CELEBRE:", x, y, 1);

        tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
        String q = cleanText(quote);
        
        int curY = y + 15;
        while (q.length() > 0 && curY < 215) {
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

    void drawCuriosityView(int x, int y, int w, const char* cur) {
        tft.setTextColor(CLR_LB_GOLD, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("* CURIOSIDAD DE RODAJE:", x, y, 1);

        tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
        String c = cleanText(cur);
        int curY = y + 14;
        while (c.length() > 0 && curY < 218) {
            if (c.length() <= 26) {
                tft.drawString(c, x, curY, 2);
                break;
            }
            int sp = c.lastIndexOf(' ', 26);
            if (sp <= 0) sp = 26;
            tft.drawString(c.substring(0, sp), x, curY, 2);
            c = c.substring(sp);
            c.trim();
            curY += 16;
        }
    }

    void drawSynopsisView(int x, int y, int w, const char* syn) {
        tft.setTextColor(CLR_LB_GREEN, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString("SINOPSIS:", x, y, 1);

        tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
        String s = cleanText(syn);
        int curY = y + 14;
        while (s.length() > 0 && curY < 218) {
            if (s.length() <= 26) {
                tft.drawString(s, x, curY, 2);
                break;
            }
            int sp = s.lastIndexOf(' ', 26);
            if (sp <= 0) sp = 26;
            tft.drawString(s.substring(0, sp), x, curY, 2);
            s = s.substring(sp);
            s.trim();
            curY += 16;
        }
    }

private:
    TFT_eSPI& tft;
};

using CinemaUI = UICinema;
