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

// Estado del recomendador
static int           currentMovieIdx = 0;
static unsigned long lastSlideChange = 0;
static bool          wifiConnected   = false;
static bool          apModeActive    = false;
static MovieTheme    activeSeasonTheme = THEME_ALL_YEAR;

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
    <p>Introduce la red Wi-Fi para que tu pantalla descargue mas recomendaciones y listas en tiempo real.</p>
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
//  DETECCION DE TEMPORADA / FECHA
// ═══════════════════════════════════════════════════════════════════════════
void updateSeasonTheme() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    if (localtime_r(&now, &timeinfo) && timeinfo.tm_year > (2020 - 1900)) {
        int month = timeinfo.tm_mon + 1; // 1 - 12
        int day   = timeinfo.tm_mday;

        // Temporada Halloween: 15 de octubre a 4 de noviembre
        if ((month == 10 && day >= 15) || (month == 11 && day <= 4)) {
            activeSeasonTheme = THEME_HALLOWEEN;
            Serial.println("[Temporada] Modo Halloween activo");
        }
        // Temporada Navidad: 1 de diciembre a 6 de enero
        else if (month == 12 || (month == 1 && day <= 6)) {
            activeSeasonTheme = THEME_CHRISTMAS;
            Serial.println("[Temporada] Modo Navidad activo");
        }
        else {
            activeSeasonTheme = THEME_ALL_YEAR;
        }
    } else {
        // Por defecto en esta fecha: Especial Halloween
        activeSeasonTheme = THEME_HALLOWEEN;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  RENDERIZADO Y CAMBIO DE PELICULA
// ═══════════════════════════════════════════════════════════════════════════
void displayMovie(int idx) {
    if (idx < 0) idx = TOTAL_EMBEDDED_MOVIES - 1;
    if (idx >= TOTAL_EMBEDDED_MOVIES) idx = 0;
    currentMovieIdx = idx;

    const MovieItem& movie = EMBEDDED_MOVIES[currentMovieIdx];
    Serial.printf("\n[Cine] Mostrando: %s (%d) - %s\n", movie.title, movie.year, movie.tag);

    // 1. Dibujar informacion de texto, valoracion y estilo Letterboxd
    ui.renderMovieInfo(movie);

    // 2. Renderizar el POSTER REAL (108x162 px)
    bool posterDrawn = false;

    // A) Poster real pre-integrado en Flash (carga instantanea y offline)
    if (movie.posterData && movie.posterLen > 0) {
        TJpgDec.drawJpg(POSTER_X, POSTER_Y, movie.posterData, movie.posterLen);
        tft.drawRoundRect(POSTER_X - 1, POSTER_Y - 1, POSTER_W + 2, POSTER_H + 2, 4, CLR_LB_BORDER);
        posterDrawn = true;
    }
    // B) Si no estuviera integrado pero hay conexion Wi-Fi, descargarlo en vivo
    else if (wifiConnected && movie.posterUrl && strlen(movie.posterUrl) > 10) {
        posterDrawn = posterFetcher.fetchAndDisplay(movie.posterUrl);
    }

    // C) Fallback solo si fallaran ambos
    if (!posterDrawn) {
        ui.drawPlaceholderPoster(movie);
    }

    lastSlideChange = millis();
}

void nextMovie() {
    int nextIdx = (currentMovieIdx + 1) % TOTAL_EMBEDDED_MOVIES;
    
    // Si estamos en una temporada tematica (ej: Halloween), buscar la siguiente de esa categoria
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
//  PANTALLA DE INICIO (SPLASH)
// ═══════════════════════════════════════════════════════════════════════════
void showSplashScreen() {
    tft.fillScreen(CLR_LB_BG);

    // Tres circulos Letterboxd
    int cx = SCREEN_W / 2;
    tft.fillCircle(cx - 24, 70, 8, CLR_LB_ORANGE);
    tft.fillCircle(cx,      70, 8, CLR_LB_GREEN);
    tft.fillCircle(cx + 24, 70, 8, CLR_LB_CYAN);

    tft.setTextColor(CLR_LB_WHITE, CLR_LB_BG);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("CYD CINEMA", cx, 96, 4);

    tft.setTextColor(CLR_LB_MUTED, CLR_LB_BG);
    tft.drawString("Recomendador de Peliculas", cx, 126, 2);
    tft.drawString("Estilo Letterboxd", cx, 146, 2);

    tft.setTextColor(CLR_LB_GREEN, CLR_LB_BG);
    tft.drawString("Iniciando...", cx, 185, 2);
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

    // Encender retroiluminacion
    pinMode(21, OUTPUT);
    digitalWrite(21, HIGH);

    showSplashScreen();

    // Inicializar controlador tactil
    touchController.begin();

    // Configurar decodificador JPEG para posters
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    // Leer credenciales Wi-Fi (de config.h o de memoria NVRAM)
    prefs.begin("cyd-cinema", true);
    String ssid = prefs.getString("ssid", String(WIFI_SSID));
    String pass = prefs.getString("pass", String(WIFI_PASS));
    prefs.end();

    if (ssid.length() > 0) {
        Serial.printf("[Wi-Fi] Conectando a '%s'...\n", ssid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), pass.c_str());

        unsigned long startConnect = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - startConnect < WIFI_TIMEOUT_SEC * 1000)) {
            delay(300);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println("\n[Wi-Fi] Conectado exitosamente.");
            Serial.printf("[Wi-Fi] IP: %s\n", WiFi.localIP().toString().c_str());

            // Sincronizar reloj de internet (NTP)
            configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET, NTP_SERVER);
            delay(500);
            updateSeasonTheme();
        } else {
            Serial.println("\n[Wi-Fi] No se pudo conectar a la red.");
        }
    } else {
        Serial.println("[Wi-Fi] Sin red configurada. Iniciando portal de configuracion.");
        startConfigAP();
    }

    // Por defecto hoy en Halloween empezar en The Shining
    if (activeSeasonTheme == THEME_HALLOWEEN) {
        currentMovieIdx = 0;
    }

    delay(600);
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

        // Mitad izquierda (poster o pie de pagina de sinopsis):
        // Alternar entre la Cita celebre y la Sinopsis detallada
        if (touchX < (POSTER_X + POSTER_W + 12)) {
            ui.toggleView();
            displayMovie(currentMovieIdx);
        }
        // Mitad derecha (informacion):
        // Pasar a la siguiente pelicula recomendada
        else {
            nextMovie();
        }
    }

    // 3. Cambio automatico tras cumplirse el intervalo de tiempo
    if (millis() - lastSlideChange >= (SLIDE_INTERVAL_SEC * 1000UL)) {
        nextMovie();
    }

    // 4. Comandos por consola serie USB
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.equalsIgnoreCase("NEXT") || cmd == "n") {
            nextMovie();
        } else if (cmd.equalsIgnoreCase("PREV") || cmd == "p") {
            prevMovie();
        } else if (cmd.equalsIgnoreCase("TOGGLE") || cmd == "t") {
            ui.toggleView();
            displayMovie(currentMovieIdx);
        } else if (cmd.equalsIgnoreCase("HALLOWEEN")) {
            activeSeasonTheme = THEME_HALLOWEEN;
            nextMovie();
        } else if (cmd.equalsIgnoreCase("NAVIDAD") || cmd.equalsIgnoreCase("XMAS")) {
            activeSeasonTheme = THEME_CHRISTMAS;
            nextMovie();
        } else if (cmd.equalsIgnoreCase("ALL")) {
            activeSeasonTheme = THEME_ALL_YEAR;
            nextMovie();
        }
    }

    delay(20);
}
