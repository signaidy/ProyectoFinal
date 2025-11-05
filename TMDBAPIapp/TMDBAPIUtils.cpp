#include "TMDBAPIUtils.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

std::string TMDBAPIUtils::buildURL(const std::string& endpoint, const std::string& query) {
    std::ostringstream oss;
    oss << base_url << endpoint << "?api_key=" << api_key;
    if (!query.empty()) {
        oss << "&" << query;
    }
    return oss.str();
}

std::vector<MovieData> TMDBAPIUtils::getMoviesForActor(int personId, int startYear, int endYear) {
    std::vector<MovieData> movies;
    std::string endpoint = "/person/" + std::to_string(personId) + "/movie_credits";
    std::string url = buildURL(endpoint);

    cpr::Response r = cpr::Get(cpr::Url{url});
    if (r.status_code != 200) {
        std::cerr << "Error al obtener películas del actor. Código: " << r.status_code << std::endl;
        return movies;
    }

    json j = json::parse(r.text);
    for (const auto& item : j["cast"]) {
        if (!item.contains("release_date") || !item["release_date"].is_string()) continue;

        std::string release_date = item["release_date"];
        if (release_date.length() < 4) continue;

        int year = 0;
        try {
            year = std::stoi(release_date.substr(0, 4));
        } catch (const std::exception& e) {
            std::cerr << "Error parsing year from release_date: '" << release_date << "' - " << e.what() << std::endl;
            continue;
        }

        if (year >= startYear && year <= endYear) {
            movies.push_back(MovieData{
                item["id"].get<int>(),
                year,
                item["title"].get<std::string>(),
                release_date
            });
        }
    }
    return movies;
}

std::vector<ActorData> TMDBAPIUtils::getMovieCast(int movieId, int limit, const std::vector<int>& excludeIds) {
    std::vector<ActorData> cast;
    std::string endpoint = "/movie/" + std::to_string(movieId) + "/credits";
    std::string url = buildURL(endpoint);

    cpr::Response r = cpr::Get(cpr::Url{url});
    if (r.status_code != 200) {
        std::cerr << "Error al obtener elenco de la película. Código: " << r.status_code << std::endl;
        return cast;
    }

    json j = json::parse(r.text);
    for (const auto& member : j["cast"]) {
        if (member["order"] >= limit) continue;
        int id = member["id"];
        if (std::find(excludeIds.begin(), excludeIds.end(), id) != excludeIds.end()) continue;

        cast.push_back(ActorData{
            id,
            member["name"]
        });
    }
    return cast;
}