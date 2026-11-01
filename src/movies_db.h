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

enum AppLanguage {
    LANG_ES = 0,
    LANG_EN = 1
};

struct MovieItem {
    const char* title;
    int year;
    const char* director;
    const char* genre_es;
    const char* genre_en;
    float rating;              // Sobre 5.0 (estilo Letterboxd)
    MovieTheme theme;
    const char* tag_es;
    const char* tag_en;
    const char* quote_es;
    const char* quote_en;
    const char* curiosity_es;
    const char* curiosity_en;
    const char* synopsis_es;
    const char* synopsis_en;
    const char* posterUrl;
    const uint8_t* posterData; // Imagen JPEG integrada en Flash (108x162 px)
    size_t posterLen;

    const char* getGenre(AppLanguage lang) const { return (lang == LANG_EN) ? genre_en : genre_es; }
    const char* getTag(AppLanguage lang) const { return (lang == LANG_EN) ? tag_en : tag_es; }
    const char* getQuote(AppLanguage lang) const { return (lang == LANG_EN) ? quote_en : quote_es; }
    const char* getCuriosity(AppLanguage lang) const { return (lang == LANG_EN) ? curiosity_en : curiosity_es; }
    const char* getSynopsis(AppLanguage lang) const { return (lang == LANG_EN) ? synopsis_en : synopsis_es; }
};

// ═══════════════════════════════════════════════════════════════════════════
//  CATALOGO INTEGRADO BILINGÜE (ES/EN) CON POSTERS REALES INCORPORADOS
// ═══════════════════════════════════════════════════════════════════════════
static const MovieItem EMBEDDED_MOVIES[] = {
    // 1. The Shining (1980)
    {
        "The Shining", 1980, "Stanley Kubrick",
        "Terror Psicologico", "Psychological Horror",
        4.3f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"Aqui esta Johnny!\"", "\"Here is Johnny!\"",
        "Jack Nicholson improviso \"Here is Johnny!\". Kubrick rodo la toma de la puerta 60 veces con hachas reales.",
        "Jack Nicholson improvised \"Here is Johnny!\". Kubrick shot the door scene 60 times using real axes.",
        "Jack Torrance se traslada con su familia al aislado hotel Overlook para cuidar de el durante el invierno.",
        "Jack Torrance accepts a caretaker job at the isolated Overlook Hotel, where sinister forces affect him.",
        "https://image.tmdb.org/t/p/w185/uAR0AWqhQL1hQa69UDEbb2rE5Wx.jpg",
        poster_the_shining, poster_the_shining_len
    },
    // 2. Halloween (1978)
    {
        "Halloween", 1978, "John Carpenter",
        "Slasher", "Slasher Horror",
        4.0f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"La noche en que EL regreso a casa!\"", "\"The night HE came home!\"",
        "La mascara de Michael Myers era de William Shatner (Capitan Kirk) de 2 dolares pintada de blanco.",
        "Michael Myers mask was a 2 dollar William Shatner (Captain Kirk) mask spray-painted white.",
        "Quince anos despues de asesinar a su hermana, Michael Myers escapa de un hospital psiquiatrico.",
        "Fifteen years after murdering his sister, Michael Myers escapes from a mental asylum and returns.",
        "https://image.tmdb.org/t/p/w185/wijlZ3HaYMvlDTPqJoTCWKFkCPU.jpg",
        poster_halloween, poster_halloween_len
    },
    // 3. Scream (1996)
    {
        "Scream", 1996, "Wes Craven",
        "Terror / Meta", "Meta Horror / Slasher",
        3.9f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"Cual es tu pelicula de terror favorita?\"", "\"Whats your favorite scary movie?\"",
        "El doblador de Ghostface llamaba a los telefonos reales de los actores desde una furgoneta oculta.",
        "Ghostface voice actor Roger Jackson made real calls to actors on set from a hidden van outside.",
        "Un ano despues del asesinato de su madre, Sidney Prescott es acechada por un asesino con mascara.",
        "A year after her mothers death, Sidney Prescott is targeted by a masked killer obsessed with horror.",
        "https://image.tmdb.org/t/p/w185/lr9ZIrmuwVmZhpZuTCW8D9g0ZJe.jpg",
        poster_scream, poster_scream_len
    },
    // 4. Psycho (1960)
    {
        "Psycho", 1960, "Alfred Hitchcock",
        "Suspense", "Psychological Thriller",
        4.4f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"El mejor amigo de un muchacho es su madre.\"", "\"A boys best friend is his mother.\"",
        "Hitchcock uso sirope de chocolate en la ducha porque en blanco y negro lucia mas denso que la sangre.",
        "Hitchcock used chocolate syrup in the shower scene because it had better contrast in black and white.",
        "Una secretaria de Phoenix malversa fondos y se aloja en un recondito motel regentado por Norman Bates.",
        "A Phoenix secretary on the run checks into a remote motel run by a polite young man named Norman Bates.",
        "https://image.tmdb.org/t/p/w185/yz4QVqPx3h1hD1DfqqQkCq3rmxW.jpg",
        poster_psycho, poster_psycho_len
    },
    // 5. Alien (1979)
    {
        "Alien", 1979, "Ridley Scott",
        "Ciencia Ficcion", "Sci-Fi Horror",
        4.3f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"En el espacio nadie puede oir tus gritos.\"", "\"In space no one can hear you scream.\"",
        "La reaccion al reventapechos fue autentica: Ridley Scott no aviso al elenco de la explosion sangrienta.",
        "The crew reaction to the chestburster scene was real: Ridley Scott did not warn them about the blood.",
        "La tripulacion de la nave espacial Nostromo intercepta una senal de socorro desde un planeta desconocido.",
        "The crew of the commercial starship Nostromo investigates a distress call from an alien vessel.",
        "https://image.tmdb.org/t/p/w185/vfrQk5IPloGg1v9Rzbh2Eg3VGyM.jpg",
        poster_alien, poster_alien_len
    },
    // 6. The Thing (1982)
    {
        "The Thing", 1982, "John Carpenter",
        "Terror Sci-Fi", "Sci-Fi Horror",
        4.3f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"El hombre es el lugar mas calido donde esconderse.\"", "\"Man is the warmest place to hide.\"",
        "No hay ningun personaje femenino con dialogo; la unica voz de mujer es la del ordenador de ajedrez.",
        "There is no female character with dialogue; the only female voice is the chess computer.",
        "Un equipo de investigadores en la Antartida se enfrenta a un organismo alienigena que imita a sus victimas.",
        "A research team in Antarctica is hunted by a shape-shifting alien that assumes the appearance of victims.",
        "https://image.tmdb.org/t/p/w185/tzGY49kseSE9QAKk47uuDGwnSCu.jpg",
        poster_the_thing, poster_the_thing_len
    },
    // 7. The Silence of the Lambs (1991)
    {
        "The Silence of the Lambs", 1991, "Jonathan Demme",
        "Thriller", "Psychological Thriller",
        4.4f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"Uno del censo intento interrogarme. Me comi su higado con habas...\"", "\"A census taker tried to test me. I ate his liver with some fava beans...\"",
        "Anthony Hopkins solo sale 16 minutos en toda la pelicula, suficientes para ganar el Oscar a Mejor Actor.",
        "Anthony Hopkins appears on screen for only 16 minutes in the entire film, enough to win Best Actor.",
        "Una joven agente del FBI busca el consejo del encarcelado Dr. Hannibal Lecter para atrapar a un asesino.",
        "A young FBI trainee seeks the advice of imprisoned Dr. Hannibal Lecter to catch a serial killer.",
        "https://image.tmdb.org/t/p/w185/uS9m8OBk1A8eM9I042bx8XXpqAq.jpg",
        poster_the_silence_of_the_lambs, poster_the_silence_of_the_lambs_len
    },
    // 8. Hereditary (2018)
    {
        "Hereditary", 2018, "Ari Aster",
        "Terror Psicologico", "Psychological Horror",
        4.0f, THEME_HALLOWEEN,
        "HALLOWEEN PICK", "HALLOWEEN PICK",
        "\"Hay cosas que se heredan... y no puedes escapar.\"", "\"Some things are in your blood... and you cant escape.\"",
        "Alex Wolff (Peter) se dio un golpe real contra el pupitre en clase, dislocandose la mandibula en la toma.",
        "Alex Wolff (Peter) actually slammed his face into the school desk on camera, dislocating his jaw.",
        "Tras la muerte de la matriarca, la familia Graham comienza a desentranar oscuros secretos sobre sus antepasados.",
        "After their reclusive grandmother dies, the Graham family discovers terrifying secrets about their ancestry.",
        "https://image.tmdb.org/t/p/w185/4GFPuL14eXi66V96xBWY73Y9PfR.jpg",
        poster_hereditary, poster_hereditary_len
    },
    // 9. Die Hard (1988)
    {
        "Die Hard", 1988, "John McTiernan",
        "Accion Navidena", "Holiday Action",
        4.1f, THEME_CHRISTMAS,
        "CINE NAVIDENO", "CHRISTMAS SPECIAL",
        "\"Yippee-ki-yay, hijo de p***!\"", "\"Yippee-ki-yay, motherf***er!\"",
        "La sorpresa de Alan Rickman al caer es real: el equipo lo solto a la cuenta de 'dos' en vez de a 'tres'.",
        "Alan Rickmans shock while falling from Nakatomi was real: the stunt team dropped him on two not three.",
        "El oficial de policia John McClane intenta salvar a su esposa y rehenes en el rascacielos Nakatomi Plaza.",
        "NYPD officer John McClane takes on terrorists who seize an LA skyscraper during a Christmas party.",
        "https://image.tmdb.org/t/p/w185/7Bjd8kfmDSOzpmhySpEhkUyK2oH.jpg",
        poster_die_hard, poster_die_hard_len
    },
    // 10. Home Alone (1990)
    {
        "Home Alone", 1990, "Chris Columbus",
        "Comedia Familiar", "Holiday Comedy",
        3.9f, THEME_CHRISTMAS,
        "CINE NAVIDENO", "CHRISTMAS SPECIAL",
        "\"Quedate con el cambio, sabandija asquerosa!\"", "\"Keep the change, ya filthy animal!\"",
        "La tarantula en la cara de Daniel Stern era autentica. Tuvo que fingir el grito sin emitir ruido.",
        "The tarantula on Daniel Sterns face was real. He had to mime his scream in dead silence.",
        "Un travieso nino de ocho anos es olvidado por error en casa durante las vacaciones familiares de Navidad.",
        "An eight-year-old troublemaker must protect his house from burglars when left home alone by mistake.",
        "https://image.tmdb.org/t/p/w185/onTSipZ8R3bliBdKfPtsDuHTdlL.jpg",
        poster_home_alone, poster_home_alone_len
    },
    // 11. The Nightmare Before Christmas (1993)
    {
        "The Nightmare Before Christmas", 1993, "Henry Selick",
        "Animacion Stop-Motion", "Stop-Motion Animation",
        4.0f, THEME_CHRISTMAS,
        "CINE NAVIDENO", "CHRISTMAS SPECIAL",
        "\"Que es? Que es? Hay luces de color!\"", "\"Whats this? Whats this? Color everywhere!\"",
        "Requirio 100 personas durante 3 anos: rodar solo un minuto de film tomaba una semana de stop-motion.",
        "Took a crew of 100 people 3 full years: producing one minute of finished footage took an entire week.",
        "Jack Skellington, el rey de Ciudad de Halloween, descubre Ciudad de la Navidad y decide apoderarse de la fiesta.",
        "Jack Skellington, king of Halloween Town, discovers Christmas Town and tries to take over the holiday.",
        "https://image.tmdb.org/t/p/w185/oQffRNjK8e19rF7xVYEN8ew0j7b.jpg",
        poster_the_nightmare_before_christmas, poster_the_nightmare_before_christmas_len
    },
    // 12. It's a Wonderful Life (1946)
    {
        "It's a Wonderful Life", 1946, "Frank Capra",
        "Drama Fantastico", "Holiday Classic",
        4.4f, THEME_CHRISTMAS,
        "CINE NAVIDENO", "CHRISTMAS SPECIAL",
        "\"Cada vez que suena una campana, un angel recibe sus alas.\"", "\"Every time a bell rings, an angel gets his wings.\"",
        "Invento la nieve de cine moderna mezclando agua, jabon y espuma de extintor para caminar sin ruido.",
        "Invented modern cinema snow by mixing water, soap, and foamite so actors could walk without loud noise.",
        "Un angel desciende a la Tierra para mostrarle a un desesperado empresario como habria sido el mundo sin el.",
        "An angel helps a frustrated businessman by showing what life would have been like without him.",
        "https://image.tmdb.org/t/p/w185/bSqt9rhDZx1Q7UZ86dBPKdNomp2.jpg",
        poster_its_a_wonderful_life, poster_its_a_wonderful_life_len
    },
    // 13. Pulp Fiction (1994)
    {
        "Pulp Fiction", 1994, "Quentin Tarantino",
        "Crimen / Culto", "Crime / Cult Classic",
        4.5f, THEME_ANNIVERSARY,
        "ANIVERSARIO (30 ANOS)", "ANNIVERSARY (30 YRS)",
        "\"Ezequiel 25:17. El camino del hombre justo...\"", "\"Ezekiel 25:17. The path of the righteous man...\"",
        "El coche descapotable de 1964 de Vincent Vega era del propio Tarantino y fue robado durante el rodaje.",
        "The 1964 Chevelle convertible driven by Vincent Vega belonged to Tarantino and was stolen on set.",
        "Las vidas de dos sicarios, la esposa de un ganster y un boxeador se entrelazan en historias de redencion.",
        "The lives of two mob hitmen, a boxer, a gangster and his wife intertwine in four tales of violence.",
        "https://image.tmdb.org/t/p/w185/vQWk5YBFWF4bZaofAbv0tShwBvQ.jpg",
        poster_pulp_fiction, poster_pulp_fiction_len
    },
    // 14. The Matrix (1999)
    {
        "The Matrix", 1999, "Lana & Lilly Wachowski",
        "Ciencia Ficcion", "Sci-Fi Action",
        4.4f, THEME_ANNIVERSARY,
        "ANIVERSARIO (25 ANOS)", "ANNIVERSARY (25 YRS)",
        "\"Toma la pastilla azul y despiertas. Toma la roja y te muestro el hoyo...\"", "\"You take the blue pill, the story ends. Take the red pill, you stay in Wonderland...\"",
        "La mitica lluvia de codigo verde digital son simbolos de recetas de sushi del libro de cocina del creador.",
        "The iconic digital rain is made of Japanese sushi recipes from the designers wife cookbook.",
        "Un hacker informatico descubre la verdadera naturaleza de su realidad y su papel en la guerra contra las maquinas.",
        "A computer hacker learns from mysterious rebels about the true nature of his reality and the machines.",
        "https://image.tmdb.org/t/p/w185/dXNAPwY7VrqMAo51EKhhCJfaGb5.jpg",
        poster_the_matrix, poster_the_matrix_len
    },
    // 15. Interstellar (2014)
    {
        "Interstellar", 2014, "Christopher Nolan",
        "Ciencia Ficcion", "Sci-Fi / Space",
        4.4f, THEME_ANNIVERSARY,
        "ANIVERSARIO (10 ANOS)", "ANNIVERSARY (10 YRS)",
        "\"El amor es lo unico capaz de trascender el tiempo y el espacio.\"", "\"Love is the one thing that transcends time and space.\"",
        "Para disenar el agujero negro Gargantua, crearon ecuaciones fisicas que generaron papers cientificos.",
        "To render the black hole Gargantua, Kip Thorne created new physics equations that yielded published papers.",
        "Un equipo de exploradores viaja a traves de un agujero de gusano en busca de un nuevo hogar para la humanidad.",
        "A team of explorers travels through a wormhole in space in an attempt to ensure humanity survival.",
        "https://image.tmdb.org/t/p/w185/yQvGrMoipbRoddT0ZR8tPoR7NfX.jpg",
        poster_interstellar, poster_interstellar_len
    },
    // 16. Parasite (2019)
    {
        "Parasite", 2019, "Bong Joon-ho",
        "Thriller / Drama", "Thriller / Drama",
        4.6f, THEME_LETTERBOXD_TOP,
        "TOP LETTERBOXD", "TOP LETTERBOXD",
        "\"El plan que nunca falla es el que no tiene plan.\"", "\"You know what plan never fails? No plan at all.\"",
        "La imponente casa minimalista de la familia Park no existia: fue un decorado construido desde cero.",
        "The luxurious modernist Park family mansion did not exist: it was an open-air set built from scratch.",
        "La humilde familia Kim se infiltra meticulosamente en el opulento hogar de los acaudalados Park.",
        "Greed and class discrimination threaten the newly formed symbiotic relationship between families.",
        "https://image.tmdb.org/t/p/w185/7IiTTgloJzvGI1TAYymCfbfl3vT.jpg",
        poster_parasite, poster_parasite_len
    },
    // 17. Whiplash (2014)
    {
        "Whiplash", 2014, "Damien Chazelle",
        "Drama Musical", "Musical Drama",
        4.4f, THEME_LETTERBOXD_TOP,
        "TOP LETTERBOXD", "TOP LETTERBOXD",
        "\"No hay palabras mas perjudiciales que 'buen trabajo'.\"", "\"There are no two words more harmful than 'good job'.\"",
        "La sangre en los platillos de la bateria era real: las manos de Miles Teller sangraron por el esfuerzo.",
        "The blood on the drum kit during rehearsals was authentic: Miles Teller played until his hands bled.",
        "Un prometedor joven baterista se inscribe en un conservatorio donde sus suenos son guiados por un despiadado instructor.",
        "A promising young drummer enrolls at a cut-throat music conservatory where his dreams are pushed hard.",
        "https://image.tmdb.org/t/p/w185/7fn624j5lj3xTme2SgiLCeuedmO.jpg",
        poster_whiplash, poster_whiplash_len
    },
    // 18. Blade Runner 2049 (2017)
    {
        "Blade Runner 2049", 2017, "Denis Villeneuve",
        "Ciencia Ficcion", "Neo-Noir / Sci-Fi",
        4.2f, THEME_LETTERBOXD_TOP,
        "TOP LETTERBOXD", "TOP LETTERBOXD",
        "\"Todas las mejores memorias son de alguien mas.\"", "\"All the best memories are hers.\"",
        "Apenas se uso pantalla verde: Villeneuve exigio construir decorados gigantes y niebla practica real.",
        "Almost zero green screen was used: Villeneuve demanded massive real-scale sets and practical fog.",
        "El descubrimiento de un secreto oculto lleva al oficial K a localizar al antiguo blade runner Rick Deckard.",
        "Young Blade Runner K unearths a long-buried secret that leads him to track down former runner Rick Deckard.",
        "https://image.tmdb.org/t/p/w185/gajva2L0rPYkEWjzgFlBXCAVBE5.jpg",
        poster_blade_runner_2049, poster_blade_runner_2049_len
    },
    // 19. Fight Club (1999)
    {
        "Fight Club", 1999, "David Fincher",
        "Drama Psicologico", "Psychological Thriller",
        4.3f, THEME_LETTERBOXD_TOP,
        "TOP LETTERBOXD", "TOP LETTERBOXD",
        "\"La primera regla del Club es: no hablas del Club de la Lucha.\"", "\"The first rule of Fight Club is: you do not talk about Fight Club.\"",
        "Brad Pitt y Edward Norton aprendieron a hacer jabon real y se rompieron piezas dentales para el film.",
        "Brad Pitt and Edward Norton took soapmaking classes and had a dentist chip their front teeth.",
        "Un oficinista insomne y un carismatico fabricante de jabon forman un club clandestino que se descontrola.",
        "An insomniac office worker and a devil-may-care soap maker form an underground fight club.",
        "https://image.tmdb.org/t/p/w185/jSziioSwPVrOy9Yow3XhWIBDjq1.jpg",
        poster_fight_club, poster_fight_club_len
    },
    // 20. The Godfather (1972)
    {
        "The Godfather", 1972, "Francis Ford Coppola",
        "Drama / Crimen", "Crime / Drama",
        4.5f, THEME_LETTERBOXD_TOP,
        "TOP LETTERBOXD", "TOP LETTERBOXD",
        "\"Le voy a hacer una oferta que no podra rechazar.\"", "\"Im gonna make him an offer he cant refuse.\"",
        "El gato de Don Corleone era un callejero que Marlon Brando encontro en el set y metio en su regazo.",
        "The cat held by Don Corleone was a stray that Marlon Brando found on set and placed on his lap.",
        "El patriarca de una dinastia del crimen organizado transfiere el control de su imperio a su reacio hijo menor.",
        "The aging patriarch of an organized crime dynasty transfers control of his empire to his reluctant son.",
        "https://image.tmdb.org/t/p/w185/3bhkrj58Vtu7enYsRolD1fZdja1.jpg",
        poster_the_godfather, poster_the_godfather_len
    }
};

static const size_t TOTAL_EMBEDDED_MOVIES = sizeof(EMBEDDED_MOVIES) / sizeof(EMBEDDED_MOVIES[0]);
