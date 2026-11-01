#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <time.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

#include "config.h"
#include "movies_db.h"
#include "ui_cinema.h"
#include "poster_fetcher.h"
#include "touch_controller.h"

// ═══════════════════════════════════════════════════════════════════════════
//  OBJETOS GLOBALES
// ═══════════════════════════════════════════════════════════════════════════
TFT_eSPI        tft = TFT_eSPI();
CinemaUI        ui(tft);
PosterFetcher   posterFetcher(tft);
CinemaTouch     touchController;
WebServer       webServer(80);
Preferences     prefs;

static const char* MONTH_NAMES_ES[] = {
    "ENE", "FEB", "MAR", "ABR", "MAY", "JUN",
    "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"
};
static const char* MONTH_NAMES_EN[] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

// Estado del dispositivo
static AppLanguage   currentLang       = LANG_ES;
static AppMode       currentMode       = APP_MODE_AUTO;
static bool          isDisplayingMusic = false;
static uint32_t      lastMusicPacket   = 0;
static uint8_t*      musicRxBuf        = nullptr;
static constexpr size_t MAX_MUSIC_PAYLOAD = 48 * 1024;

// Estado del recomendador de cine
static MovieView     currentView       = VIEW_QUOTE;
static int           currentMovieIdx   = 0;
static int           filmOfTheDayIdx   = 0;
static char          currentDateStr[16] = "";
static int           lastDayRecorded   = -1;
static unsigned long lastSlideChange   = 0;
static bool          wifiConnected     = false;
static bool          apModeActive      = false;
static MovieTheme    activeSeasonTheme = THEME_HALLOWEEN;

void displayMovie(int idx);
void nextMovie();
void prevMovie();
void updateCalendarAndSeason();

// ═══════════════════════════════════════════════════════════════════════════
//  PORTAL CAUTIVO / CONFIGURACION WEB
// ═══════════════════════════════════════════════════════════════════════════
void handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CYD Cinema & Music</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #14181C; color: #FFF; margin: 0; padding: 20px; text-align: center; }
    .card { background: #1F252B; border: 1px solid #2C343D; border-radius: 12px; padding: 24px; max-width: 400px; margin: 0 auto; box-shadow: 0 8px 24px rgba(0,0,0,0.5); text-align: left; }
    h1 { color: #00E054; margin-top: 0; font-size: 22px; text-align: center; }
    label { color: #9AB0BE; font-size: 13px; font-weight: bold; text-transform: uppercase; display: block; margin-top: 14px; margin-bottom: 4px; }
    input[type="text"], input[type="password"], select { width: 100%; padding: 10px; border: 1px solid #2C343D; border-radius: 6px; background: #14181C; color: #FFF; box-sizing: border-box; font-size: 14px; }
    button { background: #00E054; color: #14181C; border: none; font-weight: bold; width: 100%; padding: 12px; border-radius: 6px; font-size: 15px; cursor: pointer; margin-top: 20px; }
    button:hover { background: #02c74a; }
    .note { color: #6F8290; font-size: 12px; margin-top: 6px; }
  </style>
</head>
<body>
  <div class="card">
    <h1>CYD Cinema & Music</h1>
    <form method="POST" action="/save">
      <label>Idioma / Language</label>
      <select name="lang">
)rawliteral";

    html += String("<option value='0'") + (currentLang == LANG_ES ? " selected" : "") + ">Espanol (ES)</option>";
    html += String("<option value='1'") + (currentLang == LANG_EN ? " selected" : "") + ">English (EN)</option>";

    html += R"rawliteral(
      </select>

      <label>Modo de Funcionamiento / Mode</label>
      <select name="mode">
)rawliteral";

    html += String("<option value='0'") + (currentMode == APP_MODE_AUTO ? " selected" : "") + ">Hibrido Automatico (Cine en reposo / Musica al reproducir)</option>";
    html += String("<option value='1'") + (currentMode == APP_MODE_CINEMA ? " selected" : "") + ">Solo Cine (Recomendaciones Letterboxd)</option>";
    html += String("<option value='2'") + (currentMode == APP_MODE_MUSIC ? " selected" : "") + ">Solo Musica (Apple Music Now Playing)</option>";

    html += R"rawliteral(
      </select>

      <label>Red Wi-Fi (SSID)</label>
      <input type="text" name="ssid" placeholder="Nombre de red">

      <label>Contrasena Wi-Fi</label>
      <input type="password" name="pass" placeholder="Clave Wi-Fi">

      <button type="submit">Guardar y Aplicar</button>
    </form>
    <p class="note">Toca la esquina superior derecha en la pantalla para cambiar idioma o modo en cualquier momento.</p>
  </div>
</body>
</html>
)rawliteral";

    webServer.send(200, "text/html", html);
}

void handleSave() {
    if (webServer.hasArg("lang")) {
        currentLang = (AppLanguage)webServer.arg("lang").toInt();
        prefs.begin("cyd-cinema", false);
        prefs.putUChar("lang", (uint8_t)currentLang);
        prefs.end();
    }
    if (webServer.hasArg("mode")) {
        currentMode = (AppMode)webServer.arg("mode").toInt();
        prefs.begin("cyd-cinema", false);
        prefs.putUChar("mode", (uint8_t)currentMode);
        prefs.end();
    }
    String newSsid = webServer.arg("ssid");
    String newPass = webServer.arg("pass");
    bool wifiChanged = false;
    if (newSsid.length() > 0) {
        prefs.begin("cyd-cinema", false);
        prefs.putString("ssid", newSsid);
        prefs.putString("pass", newPass);
        prefs.end();
        wifiChanged = true;
    }

    webServer.send(200, "text/html", "<html><body style='background:#14181C;color:#00E054;text-align:center;padding:40px;'><h2>Ajustes guardados correctamente!</h2></body></html>");

    updateCalendarAndSeason();
    if (currentMode == APP_MODE_MUSIC) {
        isDisplayingMusic = true;
        ui.showMusicIdleScreen(currentLang);
    } else {
        isDisplayingMusic = false;
        displayMovie(currentMovieIdx);
    }

    if (wifiChanged) {
        delay(1000);
        ESP.restart();
    }
}

void startConfigAP() {
    apModeActive = true;
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);
    IPAddress IP = WiFi.softAPIP();

    webServer.on("/", handleRoot);
    webServer.on("/save", HTTP_POST, handleSave);
    webServer.begin();

    Serial.println("\n[Wi-Fi AP] Modo configuracion iniciado");
    Serial.printf("[Wi-Fi AP] Conectate a '%s' y entra en http://%s\n", AP_SSID, IP.toString().c_str());
}

// ═══════════════════════════════════════════════════════════════════════════
//  CALENDARIO DINAMICO Y TEMPORADAS
// ═══════════════════════════════════════════════════════════════════════════
void updateCalendarAndSeason() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > (2020 - 1900)) {
        int month = timeinfo.tm_mon + 1; // 1 - 12
        int day   = timeinfo.tm_mday;
        int yday  = timeinfo.tm_yday;    // 0 - 365

        if (currentLang == LANG_EN) {
            snprintf(currentDateStr, sizeof(currentDateStr), "%s %02d", MONTH_NAMES_EN[timeinfo.tm_mon], day);
        } else {
            snprintf(currentDateStr, sizeof(currentDateStr), "%02d %s", day, MONTH_NAMES_ES[timeinfo.tm_mon]);
        }

        // Pelicula destacada del dia
        filmOfTheDayIdx = yday % TOTAL_EMBEDDED_MOVIES;

        // 1. Halloween y Terror (15 Oct - 4 Nov)
        if ((month == 10 && day >= 15) || (month == 11 && day <= 4)) {
            activeSeasonTheme = THEME_HALLOWEEN;
        }
        // 2. Cine Navideno (1 Dic - 6 Ene)
        else if (month == 12 || (month == 1 && day <= 6)) {
            activeSeasonTheme = THEME_CHRISTMAS;
        }
        // 3. Temporada Premios Oscar y Grandes Obras (15 Ene - 15 Mar)
        else if ((month == 1 && day > 6) || month == 2 || (month == 3 && day <= 15)) {
            activeSeasonTheme = THEME_OSCARS;
        }
        // 4. Resto del ano: Catalogo continuo
        else {
            activeSeasonTheme = THEME_ALL_YEAR;
        }
    } else {
        if (currentLang == LANG_EN) snprintf(currentDateStr, sizeof(currentDateStr), "OCT 31");
        else snprintf(currentDateStr, sizeof(currentDateStr), "31 OCT");
        activeSeasonTheme = THEME_HALLOWEEN;
        filmOfTheDayIdx = 0;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  RENDERIZADO Y NAVEGACION CINE
// ═══════════════════════════════════════════════════════════════════════════
void displayMovie(int idx) {
    if (idx < 0) idx = TOTAL_EMBEDDED_MOVIES - 1;
    if (idx >= (int)TOTAL_EMBEDDED_MOVIES) idx = 0;
    currentMovieIdx = idx;
    isDisplayingMusic = false;

    const MovieItem& movie = EMBEDDED_MOVIES[currentMovieIdx];
    bool isFilmOfDay = (currentMovieIdx == filmOfTheDayIdx);
    const char* viewName = (currentView == VIEW_QUOTE) ? "Cita" : (currentView == VIEW_CURIOSITY ? "Curiosidad" : "Sinopsis");
    Serial.printf("\n[Cine] Mostrando: %s (%d) [%s] (%s | %s)\n", 
                  movie.title, movie.year, movie.getTag(currentLang), 
                  (currentLang == LANG_EN ? "EN" : "ES"), viewName);

    // 1. Limpieza de pantalla completa
    tft.fillScreen(CLR_LB_BG);

    // 2. Dibujar interfaz, badges, valoracion y contenido segun idioma y modo
    ui.renderMovie(movie, currentView, currentDateStr, isFilmOfDay, currentLang, currentMode);

    // 3. Renderizar el poster oficial JPEG
    bool posterDrawn = false;
    if (movie.posterData && movie.posterLen > 0) {
        TJpgDec.drawJpg(POSTER_X, POSTER_Y, movie.posterData, movie.posterLen);
        tft.drawRoundRect(POSTER_X - 1, POSTER_Y - 1, POSTER_W + 2, POSTER_H + 2, 4, CLR_LB_BORDER);
        posterDrawn = true;
    } else if (wifiConnected && movie.posterUrl && strlen(movie.posterUrl) > 10) {
        posterDrawn = posterFetcher.fetchAndDisplay(movie.posterUrl);
    }

    if (!posterDrawn) {
        ui.drawPlaceholderPoster(movie);
    }

    lastSlideChange = millis();
}

void nextMovie() {
    currentView = VIEW_QUOTE;
    int nextIdx = (currentMovieIdx + 1) % TOTAL_EMBEDDED_MOVIES;
    
    if (activeSeasonTheme != THEME_ALL_YEAR) {
        int tries = 0;
        while (EMBEDDED_MOVIES[nextIdx].theme != activeSeasonTheme && tries < (int)TOTAL_EMBEDDED_MOVIES) {
            nextIdx = (nextIdx + 1) % TOTAL_EMBEDDED_MOVIES;
            tries++;
        }
    }
    displayMovie(nextIdx);
}

void prevMovie() {
    currentView = VIEW_QUOTE;
    int prevIdx = (currentMovieIdx - 1 + TOTAL_EMBEDDED_MOVIES) % TOTAL_EMBEDDED_MOVIES;
    if (activeSeasonTheme != THEME_ALL_YEAR) {
        int tries = 0;
        while (EMBEDDED_MOVIES[prevIdx].theme != activeSeasonTheme && tries < (int)TOTAL_EMBEDDED_MOVIES) {
            prevIdx = (prevIdx - 1 + TOTAL_EMBEDDED_MOVIES) % TOTAL_EMBEDDED_MOVIES;
            tries++;
        }
    }
    displayMovie(prevIdx);
}

// ═══════════════════════════════════════════════════════════════════════════
//  PROTOCOLO MUSICA (APPLE MUSIC NOW PLAYING) Y COMANDOS SERIE
// ═══════════════════════════════════════════════════════════════════════════
void handleMusicPacket(uint8_t type, const uint8_t* p, uint32_t len) {
    lastMusicPacket = millis();

    // En modo Cine exclusivo se ignoran los frames de musica
    if (currentMode == APP_MODE_CINEMA) return;

    if (!isDisplayingMusic) {
        isDisplayingMusic = true;
        Serial.println("[Modo] Deteccion de audio USB -> REPRODUCTOR APPLE MUSIC");
    }

    switch (type) {
        case 0x01: // JPEG completo (320x240)
            TJpgDec.drawJpg(0, 0, p, len);
            break;
        case 0x02: // Parche JPEG
            if (len > 4) {
                uint16_t px = p[0] | (p[1] << 8);
                uint16_t py = p[2] | (p[3] << 8);
                TJpgDec.drawJpg(px, py, p + 4, len - 4);
            }
            break;
        case 0x03: // Brillo
            if (len >= 1) ledcWrite(0, p[0]);
            break;
        default:
            break;
    }
}

void handleAsciiCommand(String cmd) {
    cmd.trim();
    if (cmd.equalsIgnoreCase("NEXT") || cmd == "n") {
        nextMovie();
    } else if (cmd.equalsIgnoreCase("PREV") || cmd == "p") {
        prevMovie();
    } else if (cmd.equalsIgnoreCase("VIEW") || cmd.equalsIgnoreCase("v") || cmd.equalsIgnoreCase("t")) {
        currentView = (MovieView)((currentView + 1) % 3);
        displayMovie(currentMovieIdx);
    } else if (cmd.equalsIgnoreCase("LANG") || cmd == "l") {
        currentLang = (currentLang == LANG_ES) ? LANG_EN : LANG_ES;
        prefs.begin("cyd-cinema", false);
        prefs.putUChar("lang", (uint8_t)currentLang);
        prefs.end();
        updateCalendarAndSeason();
        displayMovie(currentMovieIdx);
        Serial.printf("[Idioma] Cambiado a: %s\n", (currentLang == LANG_EN ? "EN (English)" : "ES (Espanol)"));
    } else if (cmd.equalsIgnoreCase("MODE") || cmd == "m") {
        currentMode = (AppMode)((currentMode + 1) % 3);
        prefs.begin("cyd-cinema", false);
        prefs.putUChar("mode", (uint8_t)currentMode);
        prefs.end();
        if (currentMode == APP_MODE_MUSIC) {
            isDisplayingMusic = true;
            ui.showMusicIdleScreen(currentLang);
        } else {
            isDisplayingMusic = false;
            displayMovie(currentMovieIdx);
        }
        Serial.printf("[Modo] Cambiado a: %d\n", (int)currentMode);
    } else if (cmd.equalsIgnoreCase("CINEMA") || cmd.equalsIgnoreCase("CINE")) {
        currentMode = APP_MODE_CINEMA;
        isDisplayingMusic = false;
        displayMovie(currentMovieIdx);
    } else if (cmd.equalsIgnoreCase("MUSIC") || cmd.equalsIgnoreCase("MUS")) {
        currentMode = APP_MODE_MUSIC;
        isDisplayingMusic = true;
        ui.showMusicIdleScreen(currentLang);
    } else if (cmd.equalsIgnoreCase("AUTO")) {
        currentMode = APP_MODE_AUTO;
        displayMovie(currentMovieIdx);
    } else if (cmd.equalsIgnoreCase("READY")) {
        Serial.println("READY");
    }
}

void pollSerial() {
    static uint8_t  hdr[7];
    static size_t   hdrGot = 0;
    static bool     inPayload = false;
    static uint8_t  pktType = 0;
    static uint32_t pktLen = 0, pktGot = 0;
    static uint32_t lastPktRx = 0;
    static String   asciiCmd = "";

    while (Serial.available()) {
        if (!inPayload) {
            uint8_t c = Serial.peek();
            
            // Paquete binario Apple Music (0xA5 0x5A)
            if (hdrGot > 0 || c == 0xA5) {
                c = Serial.read();
                lastPktRx = millis();
                if (hdrGot == 0) {
                    if (c == 0xA5) hdr[hdrGot++] = c;
                } else if (hdrGot == 1) {
                    if (c == 0x5A) hdr[hdrGot++] = c;
                    else hdrGot = (c == 0xA5) ? 1 : 0;
                } else {
                    hdr[hdrGot++] = c;
                    if (hdrGot == 7) {
                        pktType = hdr[2];
                        pktLen  = hdr[3] | (hdr[4] << 8) | (hdr[5] << 16) | ((uint32_t)hdr[6] << 24);
                        hdrGot = 0;
                        if (pktLen > MAX_MUSIC_PAYLOAD) continue;
                        if (pktLen == 0) { handleMusicPacket(pktType, musicRxBuf, 0); continue; }
                        pktGot = 0;
                        inPayload = true;
                    }
                }
            } else {
                // Comando de texto ASCII
                c = Serial.read();
                if (c == '\r') continue;
                if (c == '\n') {
                    if (asciiCmd.length() > 0) {
                        handleAsciiCommand(asciiCmd);
                    }
                    asciiCmd = "";
                } else {
                    if (asciiCmd.length() < 64) asciiCmd += (char)c;
                }
            }
        } else {
            // Leyendo payload del fotograma JPEG
            size_t n = min((size_t)Serial.available(), (size_t)(pktLen - pktGot));
            pktGot += Serial.readBytes(musicRxBuf + pktGot, n);
            lastPktRx = millis();
            if (pktGot >= pktLen) {
                inPayload = false;
                handleMusicPacket(pktType, musicRxBuf, pktLen);
            }
        }
    }

    if ((hdrGot || inPayload) && millis() - lastPktRx > 700) {
        hdrGot = 0;
        inPayload = false;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════════════════
void setup() {
    Serial.setRxBufferSize(16384);
    Serial.begin(921600);
    delay(100);
    Serial.println("\n--- CYD Cinema & Music Dual Player ---");

    // Inicializar pantalla TFT
    tft.init();
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(CLR_LB_BG);

    // Inicializar controlador tactil
    touchController.begin();

    // Configurar decodificador JPEG
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    // Asignar buffer de fotogramas de musica
    musicRxBuf = (uint8_t*)malloc(MAX_MUSIC_PAYLOAD);

    // Leer preferencias guardadas (idioma, modo y Wi-Fi)
    prefs.begin("cyd-cinema", true);
    currentLang = (AppLanguage)prefs.getUChar("lang", (uint8_t)LANG_ES);
    currentMode = (AppMode)prefs.getUChar("mode", (uint8_t)APP_MODE_AUTO);
    String ssid = prefs.getString("ssid", WIFI_SSID);
    String pass = prefs.getString("pass", WIFI_PASS);
    prefs.end();

    if (ssid.length() > 0) {
        Serial.printf("[Wi-Fi] Conectando a '%s'...\n", ssid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());

        int timeout = WIFI_TIMEOUT_SEC * 2;
        while (WiFi.status() != WL_CONNECTED && timeout-- > 0) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println("\n[Wi-Fi] Conectado exitosamente.");
            Serial.printf("[Wi-Fi] IP: %s\n", WiFi.localIP().toString().c_str());

            webServer.on("/", handleRoot);
            webServer.on("/save", HTTP_POST, handleSave);
            webServer.begin();

            configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET, NTP_SERVER);
            delay(500);
            updateCalendarAndSeason();
        } else {
            Serial.println("\n[Wi-Fi] No se pudo conectar a la red.");
            updateCalendarAndSeason();
        }
    } else {
        Serial.println("[Wi-Fi] Sin red configurada. Iniciando portal de configuracion.");
        startConfigAP();
        updateCalendarAndSeason();
    }

    // Indicar al PC que el receptor esta listo
    Serial.println("READY");

    // Iniciar segun el modo guardado
    if (currentMode == APP_MODE_MUSIC) {
        isDisplayingMusic = true;
        ui.showMusicIdleScreen(currentLang);
    } else {
        currentMovieIdx = (activeSeasonTheme == THEME_HALLOWEEN) ? 0 : filmOfTheDayIdx;
        delay(300);
        displayMovie(currentMovieIdx);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════════════════════
void loop() {
    // 1. Atender servidor web
    webServer.handleClient();

    // 2. Procesar datos del puerto serie USB (paquetes de musica o comandos)
    pollSerial();

    // 3. Gestion tactil
    int touchX = 0, touchY = 0;
    static bool wasTouched = false;

    if (touchController.checkTouch(touchX, touchY)) {
        if (!wasTouched) {
            wasTouched = true;

            // CASO A: MODO MUSICA
            if (isDisplayingMusic) {
                bool isLiveMusicActive = (lastMusicPacket > 0 && (millis() - lastMusicPacket < 4000));

                if (isLiveMusicActive) {
                    // Si la musica esta sonando activamente en el PC, reenviar el toque (play/pausa/seek/vol)
                    Serial.printf("T %d %d D\n", touchX, touchY);

                    // Toque en esquina superior derecha permite salir a modo Cine en cualquier momento
                    if (touchX > 260 && touchY < 45) {
                        currentMode = APP_MODE_AUTO;
                        prefs.begin("cyd-cinema", false);
                        prefs.putUChar("mode", (uint8_t)currentMode);
                        prefs.end();
                        isDisplayingMusic = false;
                        displayMovie(currentMovieIdx);
                    }
                } else {
                    // PANTALLA DE REPOSO ("Esperando conexion del PC...")
                    // Cualquier toque sobre el boton o la pantalla vuelve inmediatamente a Modo Cine!
                    currentMode = APP_MODE_AUTO;
                    prefs.begin("cyd-cinema", false);
                    prefs.putUChar("mode", (uint8_t)currentMode);
                    prefs.end();
                    isDisplayingMusic = false;
                    displayMovie(currentMovieIdx);
                    Serial.println("[Modo] Boton 'Cambiar a Cine' activado -> Regresando a Cine");
                }
            }
            // CASO B: MODO CINE
            else {
                // 1. Tocar el boton de IDIOMA [ES] / [EN] (X=260..290, Y<=28)
                if (touchX >= 260 && touchX <= 292 && touchY <= 30) {
                    currentLang = (currentLang == LANG_ES) ? LANG_EN : LANG_ES;
                    prefs.begin("cyd-cinema", false);
                    prefs.putUChar("lang", (uint8_t)currentLang);
                    prefs.end();
                    updateCalendarAndSeason();
                    displayMovie(currentMovieIdx);
                }
                // 2. Tocar el boton de MODO [AUTO] / [CINE] / [MUS] (X=220..258, Y<=28)
                else if (touchX >= 220 && touchX <= 258 && touchY <= 30) {
                    currentMode = (AppMode)((currentMode + 1) % 3);
                    prefs.begin("cyd-cinema", false);
                    prefs.putUChar("mode", (uint8_t)currentMode);
                    prefs.end();
                    if (currentMode == APP_MODE_MUSIC) {
                        isDisplayingMusic = true;
                        ui.showMusicIdleScreen(currentLang);
                    } else {
                        isDisplayingMusic = false;
                        displayMovie(currentMovieIdx);
                    }
                }
                // 3. Tocar la mitad izquierda (poster o boton inferior):
                // Alternar: Cita -> Curiosidad -> Sinopsis
                else if (touchX < (POSTER_X + POSTER_W + 12)) {
                    currentView = (MovieView)((currentView + 1) % 3);
                    displayMovie(currentMovieIdx);
                }
                // 4. Tocar la mitad derecha:
                // Pasar a la siguiente pelicula
                else {
                    nextMovie();
                }
            }
        }
    } else {
        if (wasTouched) {
            wasTouched = false;
            if (isDisplayingMusic) {
                Serial.printf("T %d %d U\n", touchX, touchY);
            }
        }
    }

    // 4. Retorno a Cine por inactividad musical en modo Hibrido (APP_MODE_AUTO)
    if (isDisplayingMusic && currentMode == APP_MODE_AUTO) {
        if (lastMusicPacket > 0 && (millis() - lastMusicPacket > 9000)) {
            Serial.println("[Modo] Musica inactiva -> Volviendo suavemente a Cine");
            lastMusicPacket = 0;
            isDisplayingMusic = false;
            displayMovie(currentMovieIdx);
        }
    }

    // 5. Cambio automatico en modo Cine tras cumplirse el intervalo
    if (!isDisplayingMusic) {
        // Deteccion de cambio de dia (medianoche)
        time_t now = time(nullptr);
        struct tm timeinfo;
        if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > (2020 - 1900)) {
            if (lastDayRecorded != timeinfo.tm_mday) {
                lastDayRecorded = timeinfo.tm_mday;
                updateCalendarAndSeason();
            }
        }

        // Pase automatico: Cita (30s) -> Curiosidad (30s) -> Siguiente pelicula
        if (millis() - lastSlideChange >= (SLIDE_INTERVAL_SEC * 1000UL)) {
            if (currentView == VIEW_QUOTE) {
                currentView = VIEW_CURIOSITY;
                displayMovie(currentMovieIdx);
            } else {
                nextMovie();
            }
        }
    }

    delay(15);
}
