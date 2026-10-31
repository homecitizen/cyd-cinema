#pragma once
#include <Arduino.h>

enum MovieTheme {
    THEME_ALL_YEAR,
    THEME_HALLOWEEN,
    THEME_CHRISTMAS,
    THEME_ANNIVERSARY,
    THEME_LETTERBOXD_TOP
};

struct MovieItem {
    const char* title;
    int year;
    const char* director;
    const char* genre;
    float rating;            // Sobre 5.0 (estilo Letterboxd)
    MovieTheme theme;
    const char* tag;         // Etiqueta del badge (ej: "HALLOWEEN PICK")
    const char* quote;       // Frase mítica
    const char* posterUrl;   // URL pública de TMDb (resolución ligera w154)
    const char* synopsis;    // Sinopsis breve
};

// ═══════════════════════════════════════════════════════════════════════════
//  CATÁLOGO INTEGRADO DE PELÍCULAS Y CITAS
// ═══════════════════════════════════════════════════════════════════════════
static const MovieItem EMBEDDED_MOVIES[] = {
    // ─── ESPECIAL HALLOWEEN / TERROR ───────────────────────────────────────
    {
        "The Shining", 1980, "Stanley Kubrick", "Terror Psicológico", 4.3f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«¡Aquí está Johnny!»",
        "https://image.tmdb.org/t/p/w154/74xTEgt7R36Fpooo50r9T25onhq.jpg",
        "Jack Torrance se traslada con su familia al aislado hotel Overlook para cuidar de él en invierno."
    },
    {
        "Halloween", 1978, "John Carpenter", "Slasher", 4.0f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«¡La noche en que ÉL regresó a casa!»",
        "https://image.tmdb.org/t/p/w154/bM0mv9UKn6hF1F5X29d5bB8Z5W.jpg",
        "Quince años después de asesinar a su hermana, Michael Myers escapa de un hospital psiquiátrico."
    },
    {
        "Scream", 1996, "Wes Craven", "Terror / Meta", 3.9f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«¿Cuál es tu película de terror favorita?»",
        "https://image.tmdb.org/t/p/w154/7mH2B0aFvU9aC3w4zK1K8w2B9f.jpg",
        "Un año después del asesinato de su madre, Sidney Prescott es acechada por un asesino con máscara de fantasma."
    },
    {
        "Psycho", 1960, "Alfred Hitchcock", "Suspense", 4.4f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«El mejor amigo de un muchacho es su madre.»",
        "https://image.tmdb.org/t/p/w154/811DjJTon9gD6hZ8Q1V4X4G8h6.jpg",
        "Una secretaria de Phoenix malversa fondos y se aloja en un recóndito motel regentado por Norman Bates."
    },
    {
        "Alien", 1979, "Ridley Scott", "Ciencia Ficción", 4.3f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«En el espacio nadie puede oír tus gritos.»",
        "https://image.tmdb.org/t/p/w154/vfrQk5IPloGg1v9RzKu2a3G8h7.jpg",
        "La tripulación de la nave espacial Nostromo intercepta una señal de socorro desde un planeta desconocido."
    },
    {
        "Hereditary", 2018, "Ari Aster", "Terror Sobrenatural", 4.0f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«Hay cosas que se heredan... y no puedes escapar.»",
        "https://image.tmdb.org/t/p/w154/p9fuzXP239QXiup1PPoV1WUpogC.jpg",
        "Tras la muerte de la matriarca, la familia Graham comienza a desentrañar oscuros secretos sobre sus antepasados."
    },
    {
        "The Thing", 1982, "John Carpenter", "Terror Sci-Fi", 4.3f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«El hombre es el lugar más cálido donde esconderse.»",
        "https://image.tmdb.org/t/p/w154/9d5wK2c3Z3d3v4X5Z5v5Z5v5Z5.jpg",
        "Un equipo de investigadores en la Antártida se enfrenta a un organismo alienígena que imita a sus víctimas."
    },
    {
        "The Silence of the Lambs", 1991, "Jonathan Demme", "Thriller", 4.4f,
        THEME_HALLOWEEN, "🎃 ESPECIAL HALLOWEEN",
        "«Uno del censo intentó interrogarme una vez. Me comí su hígado con habas...»",
        "https://image.tmdb.org/t/p/w154/uS9m8OBk1A8eM9N9X4G8h6Z5v5.jpg",
        "Una joven agente del FBI busca el consejo del encarcelado Dr. Hannibal Lecter para atrapar a un asesino en serie."
    },

    // ─── ESPECIAL NAVIDAD / INVIERNO ───────────────────────────────────────
    {
        "Die Hard", 1988, "John McTiernan", "Acción Navideña", 4.1f,
        THEME_CHRISTMAS, "🎄 CINE NAVIDEÑO",
        "«¡Yippee-ki-yay, hijo de p***!»",
        "https://image.tmdb.org/t/p/w154/yFihMWenjZsl7e4rhsoz55FhRjA.jpg",
        "El oficial de policía de Nueva York John McClane intenta salvar a su esposa y rehenes en el Nakatomi Plaza."
    },
    {
        "Home Alone", 1990, "Chris Columbus", "Comedia Familiar", 3.9f,
        THEME_CHRISTMAS, "🎄 CINE NAVIDEÑO",
        "«¡Quédate con el cambio, sabandija asquerosa!»",
        "https://image.tmdb.org/t/p/w154/9wSbe4CwHjxnvvL27rV0w5xUq4.jpg",
        "Un travieso niño de ocho años es olvidado por error en casa durante las vacaciones familiares de Navidad."
    },
    {
        "It's a Wonderful Life", 1946, "Frank Capra", "Drama Fantástico", 4.4f,
        THEME_CHRISTMAS, "🎄 CINE NAVIDEÑO",
        "«Cada vez que suena una campana, un ángel recibe sus alas.»",
        "https://image.tmdb.org/t/p/w154/bSqtjrhDZj127VXZc169yvYh111.jpg",
        "Un ángel desciende a la Tierra para mostrarle a un desesperado empresario cómo habría sido el mundo sin él."
    },
    {
        "The Nightmare Before Christmas", 1993, "Henry Selick", "Animación / Stop-Motion", 4.0f,
        THEME_CHRISTMAS, "🎄 CINE NAVIDEÑO",
        "«¿Qué es? ¿Qué es? ¡Hay luces de color!»",
        "https://image.tmdb.org/t/p/w154/pyGZZf8V9K9G8X1W4Z5v5Z5v5Z5.jpg",
        "Jack Skellington, el rey de Ciudad de Halloween, descubre Ciudad de la Navidad y decide apoderarse de la fiesta."
    },
    {
        "Klaus", 2019, "Sergio Pablos", "Animación", 4.1f,
        THEME_CHRISTMAS, "🎄 CINE NAVIDEÑO",
        "«Un verdadero acto de bondad siempre inspira otro.»",
        "https://image.tmdb.org/t/p/w154/q125brvdNDmVPw49h9G8X1W4Z5.jpg",
        "Un cartero egoísta y un solitario fabricante de juguetes entablan una amistad que transforma una sombría isla."
    },

    // ─── ANIVERSARIOS Y GRANDES HITOS ──────────────────────────────────────
    {
        "Pulp Fiction", 1994, "Quentin Tarantino", "Crimen / Culto", 4.5f,
        THEME_ANNIVERSARY, "🎂 ANIVERSARIO CINE",
        "«Ezequiel 25:17. El camino del hombre justo...»",
        "https://image.tmdb.org/t/p/w154/vQWk5RFW77tWW2F7B0Z5v5Z5v5.jpg",
        "Las vidas de dos sicarios, la esposa de un gánster y un boxeador se entrelazan en violentas historias de redención."
    },
    {
        "The Matrix", 1999, "Lana & Lilly Wachowski", "Ciencia Ficción", 4.4f,
        THEME_ANNIVERSARY, "🎂 ANIVERSARIO CINE",
        "«Toma la pastilla azul y despertarás. Toma la roja y te mostraré qué tan profundo es el hoyo...»",
        "https://image.tmdb.org/t/p/w154/f89U3ADr1oiB1s9G8X1W4Z5v5Z.jpg",
        "Un hacker informático descubre la verdadera naturaleza de su realidad y su papel en la guerra contra sus controladores."
    },
    {
        "Interstellar", 2014, "Christopher Nolan", "Ciencia Ficción", 4.4f,
        THEME_ANNIVERSARY, "🎂 ANIVERSARIO (10 AÑOS)",
        "«El amor es lo único que somos capaces de percibir que trasciende el tiempo y el espacio.»",
        "https://image.tmdb.org/t/p/w154/gEU2QniE6E77NI6BlusAnX71o9.jpg",
        "Un equipo de exploradores viaja a través de un agujero de gusano en el espacio en busca de un nuevo hogar para la humanidad."
    },
    {
        "Jaws", 1975, "Steven Spielberg", "Aventura / Suspense", 4.2f,
        THEME_ANNIVERSARY, "🎂 ANIVERSARIO CINE",
        "«Va a necesitar un barco más grande.»",
        "https://image.tmdb.org/t/p/w154/s2cbuu50ne3Q7bB0Z5v5Z5v5Z5.jpg",
        "Un gran tiburón blanco aterroriza una pequeña ciudad playera de Nueva Inglaterra."
    },

    // ─── TOP LETTERBOXD & OBRAS MAESTRAS ────────────────────────────────────
    {
        "Parasite", 2019, "Bong Joon-ho", "Thriller / Drama", 4.6f,
        THEME_LETTERBOXD_TOP, "⭐ TOP LETTERBOXD",
        "«El plan que nunca falla es el que no tiene plan.»",
        "https://image.tmdb.org/t/p/w154/7IiTTgloJzvGI1TAYymCfbfl3vT.jpg",
        "La humilde familia Kim se infiltra meticulosamente en el opulento hogar de los acaudalados Park."
    },
    {
        "Whiplash", 2014, "Damien Chazelle", "Drama Musical", 4.4f,
        THEME_LETTERBOXD_TOP, "⭐ TOP LETTERBOXD",
        "«No hay dos palabras en el idioma inglés más perjudiciales que 'buen trabajo'.»",
        "https://image.tmdb.org/t/p/w154/7fn624j5lj3xTwh8ew9G8X1W4Z.jpg",
        "Un prometedor joven baterista se inscribe en un conservatorio donde sus sueños son guiados por un despiadado instructor."
    },
    {
        "Blade Runner 2049", 2017, "Denis Villeneuve", "Ciencia Ficción / Neo-noir", 4.2f,
        THEME_LETTERBOXD_TOP, "⭐ TOP LETTERBOXD",
        "«Todas las mejores memorias son de alguien más.»",
        "https://image.tmdb.org/t/p/w154/gajva2L0rPYkEWjPtF65hW1X4G.jpg",
        "El descubrimiento de un secreto largamente oculto por parte del oficial K lo lleva a localizar al antiguo blade runner Rick Deckard."
    },
    {
        "Fight Club", 1999, "David Fincher", "Drama Psicológico", 4.3f,
        THEME_LETTERBOXD_TOP, "⭐ TOP LETTERBOXD",
        "«La primera regla del Club de la Lucha es: no hablas del Club de la Lucha.»",
        "https://image.tmdb.org/t/p/w154/pB8BM7pdSp6B6Ih7QZ4DrQ3PmJK.jpg",
        "Un oficinista insomne y un carismático fabricante de jabón forman un club clandestino que evoluciona hacia algo mucho mayor."
    },
    {
        "Spirited Away", 2001, "Hayao Miyazaki", "Animación Ghibli", 4.5f,
        THEME_LETTERBOXD_TOP, "⭐ TOP LETTERBOXD",
        "«Una vez que conoces a alguien, nunca lo olvidas realmente.»",
        "https://image.tmdb.org/t/p/w154/393rWP9b44h1W4Z5v5Z5v5Z5v5.jpg",
        "Una niña de diez años queda atrapada en un mundo gobernado por dioses y brujas donde los humanos se transforman en bestias."
    },
    {
        "La La Land", 2016, "Damien Chazelle", "Romance Musical", 4.1f,
        THEME_LETTERBOXD_TOP, "⭐ TOP LETTERBOXD",
        "«Aquí está para los que sueñan, por tontos que parezcan...»",
        "https://image.tmdb.org/t/p/w154/uDO8zWDhfWz00b0Z5v5Z5v5Z5.jpg",
        "Un pianista de jazz y una aspirante a actriz se enamoran mientras persiguen sus pasiones en Los Ángeles."
    },
    {
        "The Godfather", 1972, "Francis Ford Coppola", "Drama / Crimen", 4.5f,
        THEME_LETTERBOXD_TOP, "⭐ TOP LETTERBOXD",
        "«Le voy a hacer una oferta que no podrá rechazar.»",
        "https://image.tmdb.org/t/p/w154/3bhkrj58Vtu7enYsRolD1fZdja1.jpg",
        "El patriarca de una dinastía del crimen organizado transfiere el control de su imperio a su reacio hijo menor."
    }
};

static const size_t TOTAL_EMBEDDED_MOVIES = sizeof(EMBEDDED_MOVIES) / sizeof(EMBEDDED_MOVIES[0]);
