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

// Estado del recomendador de cine
static AppLanguage   currentLang       = LANG_ES;
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
  <title>CYD Cinema</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #14181C; color: #FFF; margin: 0; padding: 20px; text-align: center; }
    .card { background: #1F252B; border: 1px solid #2C343D; border-radius: 12px; padding: 24px; max-width: 400px; margin: 0 auto; box-shadow: 0 8px 24px rgba(0,0,0,0.5); text-align: left; }
    h1 { color: #00E054; margin-top: 0; font-size: 22px; text-align: center; }
    label { color: #9AB0BE; font-size: 13px; font-weight: bold; text-transform: uppercase; display: block; margin-top: 14px; margin-bottom: 4px; }
    input[type="text"], input[type="password"], select { width: 100%; padding: 10px; border: 1px solid #2C343D; border-radius: 6px; background: #14181C; color: #FFF; box-sizing: border-box; font-size: 14px; }
    button { background: #00E054; color: #14181C; border: none; font-weight: bold; width: 100%; padding: 12px; border-radius: 6px; font-size: 15px; cursor: pointer; margin-top: 20px; }
    button:hover { background: #02c74a; }
    .note { color: #6F8290; font-size: 12px; margin-top: 8px; line-height: 1.4; }
  </style>
</head>
<body>
  <div class="card">
    <h1>CYD Cinema</h1>
    <form method="POST" action="/save">
      <label>Idioma / Language</label>
      <select name="lang">
)rawliteral";

    html += String("<option value='0'") + (currentLang == LANG_ES ? " selected" : "") + ">Espanol (ES)</option>";
    html += String("<option value='1'") + (currentLang == LANG_EN ? " selected" : "") + ">English (EN)</option>";

    html += R"rawliteral(
      </select>

      <label>Red Wi-Fi (SSID)</label>
      <input type="text" name="ssid" placeholder="Nombre de red">

      <label>Contrasena Wi-Fi</label>
      <input type="password" name="pass" placeholder="Clave Wi-Fi">

      <button type="submit">Guardar y Conectar</button>
    </form>
    <p class="note">Toca la pildora [ES] o [EN] en la pantalla tactil para alternar el idioma al instante.</p>
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
    displayMovie(currentMovieIdx);

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

        // Pelicula destacada del dia segun el calendario de 365 dias
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

    const MovieItem& movie = EMBEDDED_MOVIES[currentMovieIdx];
    bool isFilmOfDay = (currentMovieIdx == filmOfTheDayIdx);
    const char* viewName = (currentView == VIEW_QUOTE) ? "Cita" : (currentView == VIEW_CURIOSITY ? "Curiosidad" : "Sinopsis");
    Serial.printf("\n[Cine] Mostrando: %s (%d) [%s] (%s | %s)\n", 
                  movie.title, movie.year, movie.getTag(currentLang), 
                  (currentLang == LANG_EN ? "EN" : "ES"), viewName);

    // 1. Limpieza de pantalla completa
    tft.fillScreen(CLR_LB_BG);

    // 2. Dibujar interfaz, badges, valoracion y contenido segun idioma
    ui.renderMovie(movie, currentView, currentDateStr, isFilmOfDay, currentLang);

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
//  SETUP
// ═══════════════════════════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- CYD Cinema (ESP32-2432S028) ---");

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

    // Leer preferencias guardadas (idioma y Wi-Fi)
    prefs.begin("cyd-cinema", true);
    currentLang = (AppLanguage)prefs.getUChar("lang", (uint8_t)LANG_ES);
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

    currentMovieIdx = (activeSeasonTheme == THEME_HALLOWEEN) ? 0 : filmOfTheDayIdx;
    delay(300);
    displayMovie(currentMovieIdx);
}

// ═══════════════════════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════════════════════
void loop() {
    // 1. Atender servidor web
    webServer.handleClient();

    // 2. Gestion tactil
    int touchX = 0, touchY = 0;
    static bool wasTouched = false;

    if (touchController.checkTouch(touchX, touchY)) {
        if (!wasTouched) {
            wasTouched = true;

            // A) Tocar el boton de IDIOMA [ES] / [EN] (en barra inferior X=124..160, Y>=205 o esquina superior derecha X>=260, Y<=35)
            if ((touchX >= 124 && touchX <= 165 && touchY >= 205) || (touchX >= 260 && touchY <= 35)) {
                currentLang = (currentLang == LANG_ES) ? LANG_EN : LANG_ES;
                prefs.begin("cyd-cinema", false);
                prefs.putUChar("lang", (uint8_t)currentLang);
                prefs.end();
                updateCalendarAndSeason();
                displayMovie(currentMovieIdx);
            }
            // B) Tocar la mitad izquierda (poster o boton inferior bajo poster):
            // Alternar: Cita -> Curiosidad -> Sinopsis
            else if (touchX < (POSTER_X + POSTER_W + 12)) {
                currentView = (MovieView)((currentView + 1) % 3);
                displayMovie(currentMovieIdx);
            }
            // C) Tocar la mitad derecha:
            // Pasar a la siguiente pelicula
            else {
                nextMovie();
            }
        }
    } else {
        wasTouched = false;
    }

    // 3. Deteccion de cambio de dia (medianoche)
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > (2020 - 1900)) {
        if (lastDayRecorded != timeinfo.tm_mday) {
            lastDayRecorded = timeinfo.tm_mday;
            updateCalendarAndSeason();
        }
    }

    // 4. Pase automatico tras cumplirse el intervalo:
    // Cita (35s) -> Curiosidad (35s) -> Siguiente pelicula
    if (millis() - lastSlideChange >= (SLIDE_INTERVAL_SEC * 1000UL)) {
        if (currentView == VIEW_QUOTE) {
            currentView = VIEW_CURIOSITY;
            displayMovie(currentMovieIdx);
        } else {
            nextMovie();
        }
    }

    delay(20);
}
