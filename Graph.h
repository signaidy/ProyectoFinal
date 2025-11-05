// Graph.h
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <utility>

class Graph {
public:
    void addActor(int id, const std::string& name);
    void addCollaboration(int id1, int id2, const std::string& movieTitle, int movieYear);

    size_t numActors() const;
    size_t numCollaborations() const;

    bool exportToDot(const std::string& filename) const;

private:
    struct EdgeInfo {
        std::string movieTitle;
        int movieYear;
    };

    std::unordered_map<int, std::string> actors; // id -> name
    std::unordered_map<int, std::unordered_set<int>> adjacency;
    std::map<std::pair<int, int>, EdgeInfo> edgeLabels; // (min(id1,id2), max(id1,id2)) -> movie info
};