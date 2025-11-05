#pragma once

#include <string>
#include <vector>

// Estructura para datos de películas
struct MovieData {
    int id;
    int year;
    std::string title;
    std::string release_date;
};

// Estructura para datos de actores
struct ActorData {
    int id;
    std::string name;
};

class TMDBAPIUtils {
public:
    // Obtiene películas en las que participó un actor en un rango de fechas
    static std::vector<MovieData> getMoviesForActor(int personId, int startYear, int endYear);

    // Obtiene el elenco de una película
    static std::vector<ActorData> getMovieCast(int movieId, int limit = 10, const std::vector<int>& excludeIds = {});

private:
    // Clave de API y URL base de TMDB
    static constexpr const char* api_key = "a1bb56cdefd8d5a9d0eb8c4928eb530d";
    static constexpr const char* base_url = "https://api.themoviedb.org/3";

    // Construye la URL completa para una solicitud a la API
    static std::string buildURL(const std::string& endpoint, const std::string& query = "");
};