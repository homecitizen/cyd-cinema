#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════════════════
//  AJUSTES DE WI-FI
//  (Puedes poner tu Wi-Fi aquí directamente, o dejarlo vacío para usar
//   el portal cautivo de configuración desde tu móvil).
// ═══════════════════════════════════════════════════════════════════════════
#define WIFI_SSID     ""       // Ej: "MiFibra-5G"
#define WIFI_PASS     ""       // Ej: "clave1234"

// Tiempo máximo intentando conectar al Wi-Fi antes de iniciar portal cautivo (segundos)
#define WIFI_TIMEOUT_SEC   15

// Nombre de la red Wi-Fi que emite el ESP32 si no hay conexión configurada
#define AP_SSID            "CYD-Cinema-Setup"

// ═══════════════════════════════════════════════════════════════════════════
//  INTERVALO Y HORARIO
// ═══════════════════════════════════════════════════════════════════════════
// Tiempo entre recomendaciones automáticas (en segundos)
#define SLIDE_INTERVAL_SEC 35

// Zona horaria (España peninsular: GMT+1 en invierno, GMT+2 en verano)
#define NTP_SERVER         "pool.ntp.org"
#define GMT_OFFSET_SEC     3600
#define DAYLIGHT_OFFSET    3600

// ═══════════════════════════════════════════════════════════════════════════
//  FEED ONLINE DINÁMICO (OPCIONAL)
//  URL desde donde descargar recomendaciones actualizadas o listas de Letterboxd.
//  Si falla o está vacío, usa el catálogo temático offline integrado.
// ═══════════════════════════════════════════════════════════════════════════
#define ONLINE_FEED_URL    ""

// ═══════════════════════════════════════════════════════════════════════════
//  HARDWARE Y ROTACIÓN (ESP32-2432S028 / ST7789)
// ═══════════════════════════════════════════════════════════════════════════
#define TFT_ROTATION       1    // 1 = Horizontal 320x240 (cable a la derecha), 3 = 180°

// Pines del táctil XPT2046 (bus HSPI dedicado)
#define XPT2046_CLK        25
#define XPT2046_CS         33
#define XPT2046_MOSI       32
#define XPT2046_MISO       39
#define XPT2046_IRQ        36

// Dimensiones de pantalla
#define SCREEN_W           320
#define SCREEN_H           240
