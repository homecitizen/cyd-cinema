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

static const char* MONTH_NAMES[] = {
    "ENE", "FEB", "MAR", "ABR", "MAY", "JUN",
    "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"
};

// Estado del recomendador
static MovieView     currentView       = VIEW_QUOTE;
static int           currentMovieIdx   = 0;
static int           filmOfTheDayIdx   = 0;
static char          currentDateStr[16] = "";
static int           lastDayRecorded   = -1;
static unsigned long lastSlideChange   = 0;
static bool          wifiConnected     = false;
static bool          apModeActive      = false;
static MovieTheme    activeSeasonTheme = THEME_HALLOWEEN;

// ═══════════════════════════════════════════════════════════════════════════
//  PORTAL CAUTIVO / CONFIGURACION WEB WI-FI
// ═══════════════════════════════════════════════════════════════════════════
const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CYD Cinema - Configurar Wi-Fi</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #14181C; color: #FFF; margin: 0; padding: 24px; text-align: center; }
    .card { background: #202830; border-radius: 12px; padding: 24px; max-width: 380px; margin: 0 auto; box-shadow: 0 8px 24px rgba(0,0,0,0.5); }
    h1 { color: #00E054; margin-top: 0; font-size: 22px; }
    p { color: #9AB0C0; font-size: 14px; }
    input[type="text"], input[type="password"] { width: 100%; padding: 12px; margin: 8px 0 16px; border: 1px solid #38424E; border-radius: 6px; background: #14181C; color: #FFF; box-sizing: border-box; }
    button { background: #00E054; color: #14181C; border: none; font-weight: bold; width: 100%; padding: 12px; border-radius: 6px; font-size: 15px; cursor: pointer; }
    button:hover { background: #02c74a; }
  </style>
</head>
<body>
  <div class="card">
    <h1>CYD Cinema</h1>
    <p>Introduce la red Wi-Fi para sincronizar fecha, hora y listas en tiempo real.</p>
    <form method="POST" action="/save">
      <input type="text" name="ssid" placeholder="Nombre de red (SSID)" required><br>
      <input type="password" name="pass" placeholder="Contrasena Wi-Fi"><br>
      <button type="submit">Guardar y Conectar</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
    webServer.send(200, "text/html", CONFIG_HTML);
}

void handleSave() {
    String newSsid = webServer.arg("ssid");
    String newPass = webServer.arg("pass");

    prefs.begin("cyd-cinema", false);
    prefs.putString("ssid", newSsid);
    prefs.putString("pass", newPass);
    prefs.end();

    webServer.send(200, "text/html", "<html><body style='background:#14181C;color:#00E054;text-align:center;padding:40px;'><h2>Guardado. Reiniciando pantalla...</h2></body></html>");
    delay(1000);
    ESP.restart();
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
//  CALENDARIO DINAMICO Y TEMPORADAS DE TODO EL ANO
// ═══════════════════════════════════════════════════════════════════════════
void updateCalendarAndSeason() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > (2020 - 1900)) {
        int month = timeinfo.tm_mon + 1; // 1 - 12
        int day   = timeinfo.tm_mday;
        int yday  = timeinfo.tm_yday;    // 0 - 365

        snprintf(currentDateStr, sizeof(currentDateStr), "%02d %s", day, MONTH_NAMES[timeinfo.tm_mon]);

        // Pelicula destacada del dia (cambia cada dia del ano matematicamente)
        filmOfTheDayIdx = yday % TOTAL_EMBEDDED_MOVIES;

        // 1. Halloween y Terror (15 Oct - 4 Nov)
        if ((month == 10 && day >= 15) || (month == 11 && day <= 4)) {
            activeSeasonTheme = THEME_HALLOWEEN;
            Serial.printf("[Calendario] %s -> Especial Halloween activo\n", currentDateStr);
        }
        // 2. Cine Navideno (1 Dic - 6 Ene)
        else if (month == 12 || (month == 1 && day <= 6)) {
            activeSeasonTheme = THEME_CHRISTMAS;
            Serial.printf("[Calendario] %s -> Especial Navidad activo\n", currentDateStr);
        }
        // 3. Temporada Premios Oscar y Grandes Obras (15 Ene - 15 Mar)
        else if ((month == 1 && day > 6) || month == 2 || (month == 3 && day <= 15)) {
            activeSeasonTheme = THEME_OSCARS;
            Serial.printf("[Calendario] %s -> Temporada Oscar y Obras Maestras\n", currentDateStr);
        }
        // 4. Resto del ano: Catalogo continuo con Pelicula del Dia
        else {
            activeSeasonTheme = THEME_ALL_YEAR;
            Serial.printf("[Calendario] %s -> Modo Anual Letterboxd\n", currentDateStr);
        }
    } else {
        snprintf(currentDateStr, sizeof(currentDateStr), "31 OCT");
        activeSeasonTheme = THEME_HALLOWEEN;
        filmOfTheDayIdx = 0; // The Shining por defecto
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  RENDERIZADO Y NAVEGACION
// ═══════════════════════════════════════════════════════════════════════════
void displayMovie(int idx) {
    if (idx < 0) idx = TOTAL_EMBEDDED_MOVIES - 1;
    if (idx >= (int)TOTAL_EMBEDDED_MOVIES) idx = 0;
    currentMovieIdx = idx;

    const MovieItem& movie = EMBEDDED_MOVIES[currentMovieIdx];
    bool isFilmOfDay = (currentMovieIdx == filmOfTheDayIdx);
    const char* viewName = (currentView == VIEW_QUOTE) ? "Cita" : (currentView == VIEW_CURIOSITY ? "Curiosidad" : "Sinopsis");
    Serial.printf("\n[Cine] Mostrando: %s (%d) [%s] (Vista: %s)\n", 
                  movie.title, movie.year, movie.tag, viewName);

    // 1. Limpieza de pantalla completa
    tft.fillScreen(CLR_LB_BG);

    // 2. Dibujar informacion, badges, valoracion y contenido segun vista activa
    ui.renderMovie(movie, currentView, currentDateStr, isFilmOfDay);

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
    currentView = VIEW_QUOTE; // Cada nueva pelicula arranca mostrando su cita mitica
    int nextIdx = (currentMovieIdx + 1) % TOTAL_EMBEDDED_MOVIES;
    
    if (activeSeasonTheme != THEME_ALL_YEAR) {
        int tries = 0;
        while (EMBEDDED_MOVIES[nextIdx].theme != activeSeasonTheme && tries < TOTAL_EMBEDDED_MOVIES) {
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
        while (EMBEDDED_MOVIES[prevIdx].theme != activeSeasonTheme && tries < TOTAL_EMBEDDED_MOVIES) {
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
    delay(200);
    Serial.println("\n--- CYD Cinema (ESP32-2432S028) ---");

    // Inicializar pantalla TFT
    tft.init();
    tft.setRotation(TFT_ROTATION);
    tft.fillScreen(CLR_LB_BG);
    tft.setSwapBytes(true);

    // Inicializar controlador tactil
    touchController.begin();

    // Configurar decodificador JPEG
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    // Conectar Wi-Fi si hay credenciales guardadas o por defecto
    prefs.begin("cyd-cinema", true);
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

    // Arrancar con la pelicula destacada
    currentMovieIdx = (activeSeasonTheme == THEME_HALLOWEEN) ? 0 : filmOfTheDayIdx;
    delay(400);
    displayMovie(currentMovieIdx);
}

// ═══════════════════════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════════════════════
void loop() {
    // 1. Si el portal Wi-Fi esta activo, atender clientes
    if (apModeActive) {
        webServer.handleClient();
    }

    // 2. Deteccion de toques en la pantalla
    int touchX = 0, touchY = 0;
    if (touchController.checkTouch(touchX, touchY)) {
        Serial.printf("[Touch] Coordenadas: X=%d, Y=%d\n", touchX, touchY);

        // Mitad izquierda (area del poster o boton interactivo):
        // Alternar ciclicamente: CITA -> CURIOSIDAD -> SINOPSIS -> CITA
        if (touchX < (POSTER_X + POSTER_W + 12)) {
            currentView = (MovieView)((currentView + 1) % 3);
            displayMovie(currentMovieIdx);
        }
        // Mitad derecha (informacion):
        // Pasar a la siguiente pelicula recomendada
        else {
            nextMovie();
        }
    }

    // 3. Comprobacion de cambio de dia (medianoche)
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > (2020 - 1900)) {
        if (lastDayRecorded != timeinfo.tm_mday) {
            lastDayRecorded = timeinfo.tm_mday;
            updateCalendarAndSeason();
        }
    }

    // 4. Cambio automatico tras cumplirse el intervalo de tiempo:
    //    Primero muestra la Cita -> luego revela la Curiosidad de rodaje -> pasa a la siguiente peli
    if (millis() - lastSlideChange >= (SLIDE_INTERVAL_SEC * 1000UL)) {
        if (currentView == VIEW_QUOTE) {
            currentView = VIEW_CURIOSITY;
            displayMovie(currentMovieIdx);
        } else {
            nextMovie();
        }
    }

    // 5. Comandos por consola serie USB
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.equalsIgnoreCase("NEXT") || cmd == "n") {
            nextMovie();
        } else if (cmd.equalsIgnoreCase("PREV") || cmd == "p") {
            prevMovie();
        } else if (cmd.equalsIgnoreCase("VIEW") || cmd.equalsIgnoreCase("v") || cmd.equalsIgnoreCase("t")) {
            currentView = (MovieView)((currentView + 1) % 3);
            displayMovie(currentMovieIdx);
        } else if (cmd.equalsIgnoreCase("HALLOWEEN")) {
            activeSeasonTheme = THEME_HALLOWEEN;
            nextMovie();
        } else if (cmd.equalsIgnoreCase("NAVIDAD") || cmd.equalsIgnoreCase("XMAS")) {
            activeSeasonTheme = THEME_CHRISTMAS;
            nextMovie();
        } else if (cmd.equalsIgnoreCase("OSCARS")) {
            activeSeasonTheme = THEME_OSCARS;
            nextMovie();
        } else if (cmd.equalsIgnoreCase("ALL")) {
            activeSeasonTheme = THEME_ALL_YEAR;
            nextMovie();
        }
    }

    delay(20);
}
