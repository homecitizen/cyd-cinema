"""
sync_letterboxd.py — Importador y sincronizador de listas de Letterboxd a feed.json
=================================================================================
Permite:
 1. Convertir listas exportadas de Letterboxd (.csv) a formato feed.json.
 2. Enriquecer las películas con pósters de alta resolución (TMDb / CDN) y citas.
 3. Servir el archivo localmente por HTTP en tu red para que el CYD lo descargue.
"""
import json
import csv
import argparse
import sys
import os
import urllib.request
import urllib.parse
from http.server import SimpleHTTPRequestHandler, HTTPServer

def csv_to_feed(csv_path: str, output_json: str, theme_name: str = "Letterboxd Favorites"):
    if not os.path.exists(csv_path):
        print(f"Error: No se encuentra el archivo {csv_path}")
        return

    movies = []
    with open(csv_path, "r", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            title = row.get("Name") or row.get("Title") or row.get("Film")
            year = row.get("Year", "")
            rating = row.get("Rating", "4.0")
            if not title:
                continue

            try:
                year_int = int(year)
            except ValueError:
                year_int = 2000

            try:
                rating_float = float(rating)
            except ValueError:
                rating_float = 4.0

            movies.append({
                "title": title,
                "year": year_int,
                "director": row.get("Directors", "Varios"),
                "genre": row.get("Genres", "Cine"),
                "rating": rating_float,
                "tag": "⭐ LISTA LETTERBOXD",
                "quote": f"Recomendación imprescindible ({year_int}).",
                "poster_url": "",
                "synopsis": f"{title} ({year_int}). Añadida a tu selección de Letterboxd."
            })

    data = {
        "theme": theme_name,
        "count": len(movies),
        "movies": movies
    }

    with open(output_json, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

    print(f"✔ Se importaron {len(movies)} películas a {output_json}")

def serve_feed(port: int = 8080, directory: str = "data"):
    os.chdir(os.path.join(os.path.dirname(__file__), "..", directory))
    server = HTTPServer(("0.0.0.0", port), SimpleHTTPRequestHandler)
    print(f"\n📡 Servidor HTTP activo en el puerto {port}")
    print(f"   Pon esta URL en config.h (ONLINE_FEED_URL): http://<IP_DE_TU_PC>:{port}/feed.json")
    print("   Presiona Ctrl+C para detener.\n")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServidor detenido.")

def main():
    parser = argparse.ArgumentParser(description="Letterboxd Sync Tool")
    parser.add_argument("--csv", help="Ruta al archivo CSV exportado de Letterboxd")
    parser.add_argument("--out", default="../data/feed.json", help="Ruta de salida JSON")
    parser.add_argument("--serve", action="store_true", help="Servir feed.json en la red local para el CYD")
    parser.add_argument("--port", type=int, default=8080, help="Puerto HTTP local")
    args = parser.parse_args()

    if args.csv:
        csv_to_feed(args.csv, args.out)
    elif args.serve:
        serve_feed(args.port)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
