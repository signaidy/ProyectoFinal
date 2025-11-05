#include "Graph.h"
#include <iostream>
#include <algorithm>
#include <fstream>

// Agrega un actor (nodo) al grafo
void Graph::addActor(int id, const std::string& name) {
    actors.emplace(id, name);
}

// Agrega una colaboración (arista) entre dos actores con la película más reciente
void Graph::addCollaboration(int id1, int id2, const std::string& movieTitle, int movieYear) {
    if (id1 == id2) return; // no loops
    int a = std::min(id1, id2);
    int b = std::max(id1, id2);

    // Actualizar arista solo si es nueva o si la película es más reciente
    auto key = std::make_pair(a, b);
    auto it = edgeLabels.find(key);
    if (it == edgeLabels.end() || movieYear > it->second.movieYear) {
        edgeLabels[key] = {movieTitle, movieYear};
        adjacency[a].insert(b);
        adjacency[b].insert(a);
    }
}

// Devuelve el número de actores (nodos) en el grafo
size_t Graph::numActors() const {
    return actors.size();
}

// Devuelve el número de colaboraciones (aristas) en el grafo
size_t Graph::numCollaborations() const {
    return edgeLabels.size();
}

// Exporta el grafo al formato DOT para visualización con Graphviz
bool Graph::exportToDot(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    out << "graph Collaborations {\n";
    out << "  node [shape=ellipse, style=filled, color=lightblue];\n";

    // Nodos
    for (const auto& [id, name] : actors) {
        out << "  \"" << id << "\" [label=\"" << name << "\"]" << ";\n";
    }

    // Aristas con etiquetas
    for (const auto& [pair, info] : edgeLabels) {
        int a = pair.first;
        int b = pair.second;
        std::string safeTitle = info.movieTitle;
        size_t pos = 0;
        while ((pos = safeTitle.find("\"", pos)) != std::string::npos) {
            safeTitle.replace(pos, 1, "\\\"");
            pos += 2; // avanzar después del carácter escapado
        }
        out << "  \"" << a << "\" -- \"" << b << "\" [label=\"" << safeTitle << "\"];\n";
    }

    out << "}\n";
    return true;
}