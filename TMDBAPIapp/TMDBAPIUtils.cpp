#include "TMDBAPIUtils.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
// Para simplificar el uso de nlohmann::json
using json = nlohmann::json;

// Construye la URL completa para una solicitud a la API
std::string TMDBAPIUtils::buildURL(const std::string& endpoint, const std::string& query) {
    std::ostringstream oss;
    oss << base_url << endpoint << "?api_key=" << api_key;
    if (!query.empty()) {
        oss << "&" << query;
    }
    return oss.str();
}

// Obtiene películas en las que participó un actor en un rango de fechas
std::vector<MovieData> TMDBAPIUtils::getMoviesForActor(int personId, int startYear, int endYear) {
    std::vector<MovieData> movies;
    std::string endpoint = "/person/" + std::to_string(personId) + "/movie_credits";
    std::string url = buildURL(endpoint);

    // Realizar la solicitud GET
    cpr::Response r = cpr::Get(cpr::Url{url});
    if (r.status_code != 200) {
        std::cerr << "Error al obtener películas del actor. Código: " << r.status_code << std::endl;
        return movies;
    }

    // Parsear la respuesta JSON
    json j = json::parse(r.text);
    // Recorrer las películas y filtrar por rango de años
    for (const auto& item : j["cast"]) {
        if (!item.contains("release_date") || !item["release_date"].is_string()) continue;

        std::string release_date = item["release_date"];
        if (release_date.length() < 4) continue;

        int year = 0;
        // Extraer el año de la fecha de lanzamiento
        try {
            year = std::stoi(release_date.substr(0, 4));
        } catch (const std::exception& e) {
            std::cerr << "Error parsing year from release_date: '" << release_date << "' - " << e.what() << std::endl;
            continue;
        }
        // Filtrar por rango de años
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

// Obtiene el elenco de una película
std::vector<ActorData> TMDBAPIUtils::getMovieCast(int movieId, int limit, const std::vector<int>& excludeIds) {
    std::vector<ActorData> cast; // Vector para almacenar el elenco
    std::string endpoint = "/movie/" + std::to_string(movieId) + "/credits"; // Endpoint de la API para obtener créditos de la película
    std::string url = buildURL(endpoint); // Construir la URL completa

    // Realizar la solicitud GET
    cpr::Response r = cpr::Get(cpr::Url{url});
    if (r.status_code != 200) {
        std::cerr << "Error al obtener elenco de la película. Código: " << r.status_code << std::endl;
        return cast;
    }

    // Parsear la respuesta JSON
    json j = json::parse(r.text);
    // Recorrer el elenco y agregar actores al vector
    for (const auto& member : j["cast"]) {
        if (member["order"] >= limit) continue; // Limitar al número especificado
        int id = member["id"];
        if (std::find(excludeIds.begin(), excludeIds.end(), id) != excludeIds.end()) continue; // Excluir IDs especificados

        // Agregar actor al elenco
        cast.push_back(ActorData{
            id,
            member["name"]
        });
    }
    return cast;
}