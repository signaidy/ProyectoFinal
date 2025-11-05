#pragma once

#include <string>
#include <vector>

struct MovieData {
    int id;
    std::string title;
    std::string release_date;
};

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
    static constexpr const char* api_key = "<TU_API_KEY_HERE>";
    static constexpr const char* base_url = "https://api.themoviedb.org/3";

    static std::string buildURL(const std::string& endpoint, const std::string& query = "");
};