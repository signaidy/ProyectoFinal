// Graph.h
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <utility>

// Clase para representar un grafo de colaboraciones entre actores
class Graph {
public:
    // Agrega un actor (nodo) al grafo
    void addActor(int id, const std::string& name);
    // Agrega una colaboración (arista) entre dos actores con la película más reciente
    void addCollaboration(int id1, int id2, const std::string& movieTitle, int movieYear);

    // Devuelve el número de actores (nodos) en el grafo
    size_t numActors() const;
    // Devuelve el número de colaboraciones (aristas) en el grafo
    size_t numCollaborations() const;

    // Exporta el grafo al formato DOT para visualización con Graphviz
    bool exportToDot(const std::string& filename) const;

private:
    // Estructura para almacenar información de la arista
    struct EdgeInfo {
        std::string movieTitle;
        int movieYear;
    };

    std::unordered_map<int, std::string> actors; // id -> name
    std::unordered_map<int, std::unordered_set<int>> adjacency; // id -> set of connected actor ids
    std::map<std::pair<int, int>, EdgeInfo> edgeLabels; // (min(id1,id2), max(id1,id2)) -> movie info
};