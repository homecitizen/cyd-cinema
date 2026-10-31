#pragma once
#include <Arduino.h>
#include "embedded_posters.h"

enum MovieTheme {
    THEME_ALL_YEAR,
    THEME_HALLOWEEN,
    THEME_CHRISTMAS,
    THEME_OSCARS,
    THEME_ANNIVERSARY,
    THEME_LETTERBOXD_TOP
};

struct MovieItem {
    const char* title;
    int year;
    const char* director;
    const char* genre;
    float rating;              // Sobre 5.0 (estilo Letterboxd)
    MovieTheme theme;
    const char* tag;           // Etiqueta del badge (ej: "HALLOWEEN PICK")
    const char* quote;         // Frase mitica
    const char* curiosity;     // Curiosidad o anecdota fascinante de rodaje
    const char* posterUrl;     // URL publica de TMDb
    const char* synopsis;      // Sinopsis breve
    const uint8_t* posterData; // Imagen JPEG integrada en Flash (108x162 px)
    size_t posterLen;
};

// ═══════════════════════════════════════════════════════════════════════════
//  CATALOGO INTEGRADO DE PELICULAS, FRASES Y CURIOSIDADES (CON POSTERS REALES)
// ═══════════════════════════════════════════════════════════════════════════
static const MovieItem EMBEDDED_MOVIES[] = {
    // ─── ESPECIAL HALLOWEEN / TERROR ───────────────────────────────────────
    {
        "The Shining", 1980, "Stanley Kubrick", "Terror Psicologico", 4.3f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"Aqui esta Johnny!\"",
        "Jack Nicholson improviso 'Here is Johnny!'. Kubrick rodo la toma de la puerta 60 veces con hachas reales.",
        "https://image.tmdb.org/t/p/w185/uAR0AWqhQL1hQa69UDEbb2rE5Wx.jpg",
        "Jack Torrance se traslada con su familia al aislado hotel Overlook para cuidar de el durante el invierno.",
        poster_the_shining, poster_the_shining_len
    },
    {
        "Halloween", 1978, "John Carpenter", "Slasher", 4.0f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"La noche en que EL regreso a casa!\"",
        "La mascara de Michael Myers era de William Shatner (Capitan Kirk) de 2 dolares pintada de blanco.",
        "https://image.tmdb.org/t/p/w185/wijlZ3HaYMvlDTPqJoTCWKFkCPU.jpg",
        "Quince anos despues de asesinar a su hermana, Michael Myers escapa de un hospital psiquiatrico.",
        poster_halloween, poster_halloween_len
    },
    {
        "Scream", 1996, "Wes Craven", "Terror / Meta", 3.9f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"Cual es tu pelicula de terror favorita?\"",
        "El doblador de Ghostface llamaba a los telefonos reales de los actores desde una furgoneta oculta.",
        "https://image.tmdb.org/t/p/w185/lr9ZIrmuwVmZhpZuTCW8D9g0ZJe.jpg",
        "Un ano despues del asesinato de su madre, Sidney Prescott es acechada por un asesino con mascara.",
        poster_scream, poster_scream_len
    },
    {
        "Psycho", 1960, "Alfred Hitchcock", "Suspense", 4.4f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"El mejor amigo de un muchacho es su madre.\"",
        "Hitchcock uso sirope de chocolate en la ducha porque en blanco y negro lucia mas denso que la sangre.",
        "https://image.tmdb.org/t/p/w185/yz4QVqPx3h1hD1DfqqQkCq3rmxW.jpg",
        "Una secretaria de Phoenix malversa fondos y se aloja en un recondito motel regentado por Norman Bates.",
        poster_psycho, poster_psycho_len
    },
    {
        "Alien", 1979, "Ridley Scott", "Ciencia Ficcion", 4.3f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"En el espacio nadie puede oir tus gritos.\"",
        "La reaccion al reventapechos fue autentica: Ridley Scott no aviso al elenco de la explosion sangrienta.",
        "https://image.tmdb.org/t/p/w185/vfrQk5IPloGg1v9Rzbh2Eg3VGyM.jpg",
        "La tripulacion de la nave espacial Nostromo intercepta una senal de socorro desde un planeta desconocido.",
        poster_alien, poster_alien_len
    },
    {
        "The Thing", 1982, "John Carpenter", "Terror Sci-Fi", 4.3f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"El hombre es el lugar mas calido donde esconderse.\"",
        "No hay ningun personaje femenino con dialogo; la unica voz de mujer es la del ordenador de ajedrez.",
        "https://image.tmdb.org/t/p/w185/tzGY49kseSE9QAKk47uuDGwnSCu.jpg",
        "Un equipo de investigadores en la Antartida se enfrenta a un organismo alienigena que imita a sus victimas.",
        poster_the_thing, poster_the_thing_len
    },
    {
        "The Silence of the Lambs", 1991, "Jonathan Demme", "Thriller", 4.4f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"Uno del censo intento interrogarme. Me comi su higado con habas...\"",
        "Anthony Hopkins solo sale 16 minutos en toda la pelicula, suficientes para ganar el Oscar a Mejor Actor.",
        "https://image.tmdb.org/t/p/w185/uS9m8OBk1A8eM9I042bx8XXpqAq.jpg",
        "Una joven agente del FBI busca el consejo del encarcelado Dr. Hannibal Lecter para atrapar a un asesino.",
        poster_the_silence_of_the_lambs, poster_the_silence_of_the_lambs_len
    },
    {
        "Hereditary", 2018, "Ari Aster", "Terror Psicologico", 4.0f,
        THEME_HALLOWEEN, "HALLOWEEN PICK",
        "\"Hay cosas que se heredan... y no puedes escapar.\"",
        "Alex Wolff (Peter) se dio un golpe real contra el pupitre en clase, dislocandose la mandibula en la toma.",
        "https://image.tmdb.org/t/p/w185/4GFPuL14eXi66V96xBWY73Y9PfR.jpg",
        "Tras la muerte de la matriarca, la familia Graham comienza a desentranar oscuros secretos sobre sus antepasados.",
        poster_hereditary, poster_hereditary_len
    },

    // ─── ESPECIAL NAVIDAD / INVIERNO ───────────────────────────────────────
    {
        "Die Hard", 1988, "John McTiernan", "Accion Navidena", 4.1f,
        THEME_CHRISTMAS, "CINE NAVIDENO",
        "\"Yippee-ki-yay, hijo de p***!\"",
        "La sorpresa de Alan Rickman al caer es real: el equipo lo solto a la cuenta de 'dos' en vez de a 'tres'.",
        "https://image.tmdb.org/t/p/w185/7Bjd8kfmDSOzpmhySpEhkUyK2oH.jpg",
        "El oficial de policia John McClane intenta salvar a su esposa y rehenes en el rascacielos Nakatomi Plaza.",
        poster_die_hard, poster_die_hard_len
    },
    {
        "Home Alone", 1990, "Chris Columbus", "Comedia Familiar", 3.9f,
        THEME_CHRISTMAS, "CINE NAVIDENO",
        "\"Quedate con el cambio, sabandija asquerosa!\"",
        "La tarantula en la cara de Daniel Stern era autentica. Tuvo que fingir el grito sin emitir ruido.",
        "https://image.tmdb.org/t/p/w185/onTSipZ8R3bliBdKfPtsDuHTdlL.jpg",
        "Un travieso nino de ocho anos es olvidado por error en casa durante las vacaciones familiares de Navidad.",
        poster_home_alone, poster_home_alone_len
    },
    {
        "The Nightmare Before Christmas", 1993, "Henry Selick", "Animacion Stop-Motion", 4.0f,
        THEME_CHRISTMAS, "CINE NAVIDENO",
        "\"Que es? Que es? Hay luces de color!\"",
        "Requirio 100 personas durante 3 anos: rodar solo un minuto de film tomaba una semana de stop-motion.",
        "https://image.tmdb.org/t/p/w185/oQffRNjK8e19rF7xVYEN8ew0j7b.jpg",
        "Jack Skellington, el rey de Ciudad de Halloween, descubre Ciudad de la Navidad y decide apoderarse de la fiesta.",
        poster_the_nightmare_before_christmas, poster_the_nightmare_before_christmas_len
    },
    {
        "It's a Wonderful Life", 1946, "Frank Capra", "Drama Fantastico", 4.4f,
        THEME_CHRISTMAS, "CINE NAVIDENO",
        "\"Cada vez que suena una campana, un angel recibe sus alas.\"",
        "Invento la nieve de cine moderna mezclando agua, jabon y espuma de extintor para caminar sin ruido.",
        "https://image.tmdb.org/t/p/w185/bSqt9rhDZx1Q7UZ86dBPKdNomp2.jpg",
        "Un angel desciende a la Tierra para mostrarle a un desesperado empresario como habria sido el mundo sin el.",
        poster_its_a_wonderful_life, poster_its_a_wonderful_life_len
    },

    // ─── ANIVERSARIOS Y GRANDES HITOS ──────────────────────────────────────
    {
        "Pulp Fiction", 1994, "Quentin Tarantino", "Crimen / Culto", 4.5f,
        THEME_ANNIVERSARY, "ANIVERSARIO (30 ANOS)",
        "\"Ezequiel 25:17. El camino del hombre justo...\"",
        "El coche descapotable de 1964 de Vincent Vega era del propio Tarantino y fue robado durante el rodaje.",
        "https://image.tmdb.org/t/p/w185/vQWk5YBFWF4bZaofAbv0tShwBvQ.jpg",
        "Las vidas de dos sicarios, la esposa de un ganster y un boxeador se entrelazan en historias de redencion.",
        poster_pulp_fiction, poster_pulp_fiction_len
    },
    {
        "The Matrix", 1999, "Lana & Lilly Wachowski", "Ciencia Ficcion", 4.4f,
        THEME_ANNIVERSARY, "ANIVERSARIO (25 ANOS)",
        "\"Toma la pastilla azul y despiertas. Toma la roja y te muestro el hoyo...\"",
        "La mitica lluvia de codigo verde digital son simbolos de recetas de sushi del libro de cocina del creador.",
        "https://image.tmdb.org/t/p/w185/dXNAPwY7VrqMAo51EKhhCJfaGb5.jpg",
        "Un hacker informatico descubre la verdadera naturaleza de su realidad y su papel en la guerra contra las maquinas.",
        poster_the_matrix, poster_the_matrix_len
    },
    {
        "Interstellar", 2014, "Christopher Nolan", "Ciencia Ficcion", 4.4f,
        THEME_ANNIVERSARY, "ANIVERSARIO (10 ANOS)",
        "\"El amor es lo unico capaz de trascender el tiempo y el espacio.\"",
        "Para disenar el agujero negro Gargantua, crearon ecuaciones fisicas que generaron papers cientificos.",
        "https://image.tmdb.org/t/p/w185/yQvGrMoipbRoddT0ZR8tPoR7NfX.jpg",
        "Un equipo de exploradores viaja a traves de un agujero de gusano en busca de un nuevo hogar para la humanidad.",
        poster_interstellar, poster_interstellar_len
    },

    // ─── TOP LETTERBOXD & OBRAS MAESTRAS ────────────────────────────────────
    {
        "Parasite", 2019, "Bong Joon-ho", "Thriller / Drama", 4.6f,
        THEME_LETTERBOXD_TOP, "TOP LETTERBOXD",
        "\"El plan que nunca falla es el que no tiene plan.\"",
        "La imponente casa minimalista de la familia Park no existia: fue un decorado construido desde cero.",
        "https://image.tmdb.org/t/p/w185/7IiTTgloJzvGI1TAYymCfbfl3vT.jpg",
        "La humilde familia Kim se infiltra meticulosamente en el opulento hogar de los acaudalados Park.",
        poster_parasite, poster_parasite_len
    },
    {
        "Whiplash", 2014, "Damien Chazelle", "Drama Musical", 4.4f,
        THEME_LETTERBOXD_TOP, "TOP LETTERBOXD",
        "\"No hay palabras mas perjudiciales que 'buen trabajo'.\"",
        "La sangre en los platillos de la bateria era real: las manos de Miles Teller sangraron por el esfuerzo.",
        "https://image.tmdb.org/t/p/w185/7fn624j5lj3xTme2SgiLCeuedmO.jpg",
        "Un prometedor joven baterista se inscribe en un conservatorio donde sus suenos son guiados por un despiadado instructor.",
        poster_whiplash, poster_whiplash_len
    },
    {
        "Blade Runner 2049", 2017, "Denis Villeneuve", "Ciencia Ficcion", 4.2f,
        THEME_LETTERBOXD_TOP, "TOP LETTERBOXD",
        "\"Todas las mejores memorias son de alguien mas.\"",
        "Apenas se uso pantalla verde: Villeneuve exigio construir decorados gigantes y niebla practica real.",
        "https://image.tmdb.org/t/p/w185/gajva2L0rPYkEWjzgFlBXCAVBE5.jpg",
        "El descubrimiento de un secreto oculto lleva al oficial K a localizar al antiguo blade runner Rick Deckard.",
        poster_blade_runner_2049, poster_blade_runner_2049_len
    },
    {
        "Fight Club", 1999, "David Fincher", "Drama Psicologico", 4.3f,
        THEME_LETTERBOXD_TOP, "TOP LETTERBOXD",
        "\"La primera regla del Club es: no hablas del Club de la Lucha.\"",
        "Brad Pitt y Edward Norton aprendieron a hacer jabon real y se rompieron piezas dentales para el film.",
        "https://image.tmdb.org/t/p/w185/jSziioSwPVrOy9Yow3XhWIBDjq1.jpg",
        "Un oficinista insomne y un carismatico fabricante de jabon forman un club clandestino que se descontrola.",
        poster_fight_club, poster_fight_club_len
    },
    {
        "The Godfather", 1972, "Francis Ford Coppola", "Drama / Crimen", 4.5f,
        THEME_LETTERBOXD_TOP, "TOP LETTERBOXD",
        "\"Le voy a hacer una oferta que no podra rechazar.\"",
        "El gato de Don Corleone era un callejero que Marlon Brando encontro en el set y metio en su regazo.",
        "https://image.tmdb.org/t/p/w185/3bhkrj58Vtu7enYsRolD1fZdja1.jpg",
        "El patriarca de una dinastia del crimen organizado transfiere el control de su imperio a su reacio hijo menor.",
        poster_the_godfather, poster_the_godfather_len
    }
};

static const size_t TOTAL_EMBEDDED_MOVIES = sizeof(EMBEDDED_MOVIES) / sizeof(EMBEDDED_MOVIES[0]);
