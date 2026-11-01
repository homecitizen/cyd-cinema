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
#define CLR_LB_PINK        0xFB56    // #FA2D48 (Rosa Apple Music)

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

// Modos de funcionamiento del dispositivo
enum AppMode {
    APP_MODE_AUTO = 0,    // Hibrido: Cine en reposo; si el PC envia musica pasa a Apple Music; al pausar vuelve a Cine
    APP_MODE_CINEMA = 1,  // Fijo: Solo recomendaciones de Cine (Letterboxd)
    APP_MODE_MUSIC = 2    // Fijo: Solo reproductor Apple Music
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
        int badgeW = len * 6 + 12;
        tft.fillRoundRect(x, y, badgeW, 16, 4, CLR_LB_CARD);
        tft.drawRoundRect(x, y, badgeW, 16, 4, tagColor);

        tft.setTextColor(tagColor, CLR_LB_CARD);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(cTag, x + 6, y + 2, 1);
    }

    void renderMovie(const MovieItem& movie, MovieView view, const char* dateStr,
                     bool isFilmOfTheDay, AppLanguage lang, AppMode mode) {
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
            tft.drawString((lang == LANG_EN) ? "[Trivia]" : "[Curiosidad]", 10 + 52, btnY + 9, 1);
        } else if (view == VIEW_CURIOSITY) {
            tft.drawString((lang == LANG_EN) ? "[Synopsis]" : "[Sinopsis]", 10 + 52, btnY + 9, 1);
        } else {
            tft.drawString((lang == LANG_EN) ? "[Quote]" : "[Cita]", 10 + 52, btnY + 9, 1);
        }

        // ── 2. COLUMNA DERECHA DE DETALLES (X=128..316) ─────────────────────
        int rx = POSTER_X + POSTER_W + 12; // 128
        int rw = 320 - rx - 8;

        // Badge tematico o insignia "PELICULA DEL DIA" (con espacio libre completo)
        if (isFilmOfTheDay) {
            const char* fotd = (lang == LANG_EN) ? "FILM OF THE DAY" : "PELICULA DEL DIA";
            drawBadge(rx, 8, fotd, THEME_ANNIVERSARY);
        } else {
            drawBadge(rx, 8, movie.getTag(lang), movie.theme);
        }

        // Fecha actual en la esquina superior derecha (ej: "31 OCT")
        if (dateStr && strlen(dateStr) > 0) {
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.setTextDatum(TR_DATUM);
            tft.drawString(dateStr, 320 - 6, 10, 1);
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

        // Genero segun idioma
        char genreStr[60];
        const char* gPrefix = (lang == LANG_EN) ? "Genre" : "Genero";
        snprintf(genreStr, sizeof(genreStr), "%s: %s", gPrefix, cleanText(movie.getGenre(lang)).c_str());
        tft.drawString(genreStr, rx, 96, 2);

        // Linea divisoria
        tft.drawFastHLine(rx, 116, rw, CLR_LB_BORDER);

        // Vista seleccionada: Cita, Curiosidad o Sinopsis
        if (view == VIEW_QUOTE) {
            const char* qHead = (lang == LANG_EN) ? "ICONIC QUOTE:" : "CITA CELEBRE:";
            drawTextView(rx, 124, rw, qHead, CLR_LB_CYAN, movie.getQuote(lang), CLR_LB_WHITE);
        } else if (view == VIEW_CURIOSITY) {
            const char* cHead = (lang == LANG_EN) ? "* BEHIND THE SCENES:" : "* CURIOSIDAD DE RODAJE:";
            drawTextView(rx, 124, rw, cHead, CLR_LB_GOLD, movie.getCuriosity(lang), CLR_LB_WHITE);
        } else {
            const char* sHead = (lang == LANG_EN) ? "SYNOPSIS:" : "SINOPSIS:";
            drawTextView(rx, 124, rw, sHead, CLR_LB_GREEN, movie.getSynopsis(lang), CLR_LB_MUTED);
        }

        // ── BARRA INFERIOR DE CONTROLES (Y=218..238) ──────────────────────
        // Pildora de Modo (X=128..158): [AUTO] / [CINE] / [MUS]
        const char* modeLabel = (mode == APP_MODE_AUTO) ? "AUTO" : (mode == APP_MODE_CINEMA ? "CINE" : "MUS");
        uint16_t modeColor = (mode == APP_MODE_AUTO) ? CLR_LB_CYAN : (mode == APP_MODE_CINEMA ? CLR_LB_GREEN : CLR_LB_PINK);
        tft.fillRoundRect(rx, 218, 32, 16, 3, CLR_LB_CARD);
        tft.drawRoundRect(rx, 218, 32, 16, 3, modeColor);
        tft.setTextColor(modeColor, CLR_LB_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(modeLabel, rx + 16, 218 + 8, 1);

        // Pildora de Idioma (X=166..188): [ES] o [EN] (tappable!)
        const char* langLabel = (lang == LANG_EN) ? "EN" : "ES";
        tft.fillRoundRect(rx + 38, 218, 24, 16, 3, CLR_LB_CARD);
        tft.drawRoundRect(rx + 38, 218, 24, 16, 3, CLR_LB_BORDER);
        tft.setTextColor(CLR_LB_WHITE, CLR_LB_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(langLabel, rx + 38 + 12, 218 + 8, 1);

        // Indicador siguiente en esquina inferior derecha
        tft.setTextColor(CLR_LB_BORDER, CLR_LB_BG);
        tft.setTextDatum(TR_DATUM);
        tft.drawString((lang == LANG_EN) ? "Next >" : "Siguiente >", 320 - 8, 222, 1);
    }

    void drawTextView(int x, int y, int w, const char* header, uint16_t hColor,
                      const char* content, uint16_t cColor) {
        tft.setTextColor(hColor, CLR_LB_BG);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(header, x, y, 1);

        tft.setTextColor(cColor, CLR_LB_BG);
        String s = cleanText(content);
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

    void showMusicIdleScreen(AppLanguage lang) {
        tft.fillScreen(CLR_LB_BG);
        
        // Cabecera LetterDeck Music
        tft.setTextColor(CLR_LB_PINK, CLR_LB_BG);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("* LetterDeck Music *", 160, 55, 4);

        tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
        if (lang == LANG_EN) {
            tft.drawString("Waiting for PC connection...", 160, 100, 2);
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.drawString("Run 'py nowplaying.py' on Windows", 160, 125, 2);
        } else {
            tft.drawString("Esperando conexion del PC...", 160, 100, 2);
            tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
            tft.drawString("Ejecuta 'py nowplaying.py' en Windows", 160, 125, 2);
        }

        // Gran boton interactivo para volver a modo cine
        tft.fillRoundRect(50, 170, 220, 42, 8, CLR_LB_CARD);
        tft.drawRoundRect(50, 170, 220, 42, 8, CLR_LB_GREEN);
        tft.setTextColor(CLR_LB_GREEN, CLR_LB_CARD);
        tft.setTextDatum(MC_DATUM);
        tft.drawString((lang == LANG_EN) ? "<- Switch to Cinema" : "<- Cambiar a Cine", 160, 191, 4);
    }

private:
    TFT_eSPI& tft;
};

using CinemaUI = UICinema;
