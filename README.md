# Final Data Analysis Project

This repository contains a multi-module academic project that combines data engineering, graph analytics, and dimensionality reduction.

The work is organized into independent modules that can be run separately:

1. **TMDB Collaboration Graph (`TMDBAPIapp`)**
   Builds an actor collaboration graph from The Movie Database (TMDB) API using C++ and exports a Graphviz visualization.
   - Module README: [TMDBAPIapp/README.md](TMDBAPIapp/README.md)

2. **Incidents Analytics Pipeline (`Tablas`)**
   Loads incident CSV datasets into SQLite, runs analytical queries in C++, and generates charts.
   - Module README: [Tablas/README.md](Tablas/README.md)

3. **ISOMAP Dimensionality Reduction (`ISOMAP`)**
   Applies ISOMAP to high-dimensional biological data (ALL/AML leukemia gene expression) and produces 2D embeddings.
   - Module README: [ISOMAP/README.md](ISOMAP/README.md)

4. **Project Presentation (`Presentacion`)**
   Narrative and results summary for the full project.
   - Presentation file: [Presentacion/presentacion.md](Presentacion/presentacion.md)

## End-to-End Overview

At a high level, the project demonstrates a complete analytics workflow:

1. Extract and model relationships as graphs (TMDB module).
2. Build a reproducible analytics pipeline on structured datasets (Incidents module).
3. Explore hidden nonlinear structure in high-dimensional data (ISOMAP module).
4. Present methods, results, and interpretation in a single report (Presentation module).

## Notes

- Each module has its own dependencies and execution steps.
- Use the linked module README files for setup, run commands, and outputs.
