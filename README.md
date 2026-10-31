# 🎬 CYD Cinema — Recomendador de Cine Estilo Letterboxd
## ESP32-2432S028 (CYD 2.8" ST7789 320×240 + Touch XPT2046)

Dispositivo de sobremesa autónomo que muestra recomendaciones de películas, pósters oficiales de alta calidad, fichas técnicas y citas célebres con la estética oscura y minimalista de **Letterboxd**.

![Vista Previa Letterboxd](tools/preview_cinema.png)

---

## ✨ Características principales

1. **Detección Estacional Inteligente (NTP)**:
   - 🎃 **Especial Halloween / Terror** *(Octubre / Noviembre)*: Clásicos del horror (*The Shining, Halloween, Scream, Psycho, Alien, Hereditary, The Thing, The Silence of the Lambs...*).
   - 🎄 **Especial Navidad / Invierno** *(Diciembre / Enero)*: Cine navideño icónico (*Die Hard, Home Alone, It's a Wonderful Life, Klaus, The Nightmare Before Christmas...*).
   - 🎂 **Aniversarios de Cine**: Conmemoración de obras maestras (*Pulp Fiction, The Matrix, Interstellar, Jaws...*).
   - ⭐ **Top Letterboxd & Clásicos de Culto**: *Parasite, Whiplash, Blade Runner 2049, Fight Club, Spirited Away, La La Land, The Godfather...*
2. **Estética 100% Letterboxd**:
   - Paleta de color oscura: fondo carbón `#14181C`, paneles `#202830`, verde `#00E054`, naranja `#FF8000` y cian `#40BCF4`.
   - Valoración por estrellas (ej: `★ 4.3 / 5`).
   - Logotipo de los tres círculos icónicos de Letterboxd.
3. **Pósters en vivo por Wi-Fi**:
   - Descarga directa de carátulas JPEG ligeras (TMDb / CDN) decodificadas por hardware con `TJpg_Decoder`.
   - **Modo Offline**: Si no hay conexión Wi-Fi, la pantalla muestra elegantes tarjetas vectoriales con rollos de celuloide y el catálogo integrado (nunca se queda en blanco).
4. **Pantalla Táctil Interactiva (XPT2046)**:
   - **Tocar la mitad derecha**: Pasa inmediatamente a la siguiente película.
   - **Tocar la mitad izquierda (póster / botón inferior)**: Alterna entre la **Cita Célebre** y la **Sinopsis completa**.
   - **Rotación automática**: Cambia de película cada 35 segundos (configurable en `config.h`).
5. **Configuración Wi-Fi sin complicaciones**:
   - Si no introduces tu red en `src/config.h`, el ESP32 crea automáticamente una red Wi-Fi llamada `CYD-Cinema-Setup`. Conéctate con tu móvil, entra en `http://192.168.4.1` y guarda tu Wi-Fi.

---

## 🛠️ Estructura del proyecto

```
cyd-cinema/
├── platformio.ini              ← Configuración optimizada con driver ST7789 y librerías
├── src/
│   ├── main.cpp                ← Bucle principal, Wi-Fi, servidor web y eventos táctiles
│   ├── config.h                ← Credenciales Wi-Fi, rotación, intervalo de cambio
│   ├── movies_db.h             ← Base de datos integrada con películas, citas y sinopsis
│   ├── ui_cinema.h             ← Motor de renderizado estilo Letterboxd
│   ├── poster_fetcher.h        ← Descarga HTTP(S) y decodificador JPEG
│   └── touch_controller.h      ← Controlador táctil XPT2046 (bus HSPI)
├── data/
│   └── feed.json               ← Ejemplo de feed dinámico descargable
└── tools/
    ├── preview_card.py         ← Generador de vista previa en PC con Pillow
    ├── preview_cinema.png      ← Imagen de muestra de la interfaz
    └── sync_letterboxd.py      ← Importador de listas CSV de Letterboxd a JSON
```

---

## 🚀 Cómo grabarlo en tu CYD (ESP32-2432S028)

### Opción A: Con PlatformIO (Recomendada)
1. Conecta la CYD por cable USB al ordenador.
2. Abre la carpeta `cyd-cinema` en VS Code con PlatformIO.
3. (Opcional) Abre `src/config.h` y pon el nombre de tu Wi-Fi en `WIFI_SSID` y la clave en `WIFI_PASS`.
4. Haz clic en **Upload** (icono de la flecha en la barra inferior) o ejecuta:
```powershell
pio run -t upload --upload-port COM3
```

---

### Opción B: Con Arduino IDE
Configura tu Arduino IDE con los siguientes ajustes:
- **Placa**: `ESP32 Dev Module`
- **Partición**: `Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)`
- **Librerías a instalar**:
  1. `TFT_eSPI` (configurar pines en `User_Setup.h` o `User_Setup_Select.h` según `ESPECIFICACIONES_PANTALLA_CYD.md`).
  2. `TJpg_Decoder` (de Bodmer).
  3. `ArduinoJson` (de Benoît Blanchon).
  4. `XPT2046_Touchscreen` (de Paul Stoffregen).

---

## 📱 Control por puerto Serie (cuando está conectado al PC)

Si tienes la pantalla conectada al PC mientras trabajas, puedes controlarla desde cualquier monitor serie a 115200 baudios:
- Enviar `NEXT` o `n`: Siguiente película.
- Enviar `PREV` o `p`: Película anterior.
- Enviar `TOGGLE` o `t`: Alternar entre cita célebre y sinopsis.
- Enviar `HALLOWEEN`: Forzar modo especial de terror.
- Enviar `NAVIDAD`: Forzar modo navideño.
- Enviar `ALL`: Modo variado normal.
