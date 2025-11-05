#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <semaphore>       // C++20: std::counting_semaphore
#include "TMDBAPIUtils.h"  // (Interfaz para la API de TMDB)
#include "Graph.h"         // (Interfaz para la representación del grafo)

int main() {
    // 1. Inicialización: especificar actor principal y rango de años
    int mainActorId = 6384;                  // ID de Keanu Reeves en TMDB
    std::string mainActorName = "Keanu Reeves";
    int startYear = 1985, endYear = 2023;    // Rango de años a considerar

    std::cout << "Construyendo grafo de colaboraciones para " 
              << mainActorName << " (" << startYear << "-" << endYear << ")...\n";

    // Crear instancia del grafo de colaboraciones
    Graph graph;
    // Agregar el actor principal al grafo como nodo inicial
    graph.addActor(mainActorId, mainActorName);

    // 2. Obtener la lista de películas del actor principal en el rango dado
    //    (Función implementada en TMDBAPIUtils; devuelve, por ejemplo, vector de películas con id, título y año)
    std::vector<MovieData> filmography = TMDBAPIUtils::getMoviesForActor(mainActorId, startYear, endYear);
    // MovieData es una estructura definida en TMDBAPIUtils que contiene al menos:
    //    - id (int): ID de la película en TMDB
    //    - title (std::string): título de la película
    //    - year (int): año de estreno de la película

    // Semáforo para limitar a 5 las llamadas simultáneas a la API (por ejemplo)
    const int MAX_CONCURRENT_CALLS = 5;
    std::counting_semaphore<MAX_CONCURRENT_CALLS> apiSemaphore(MAX_CONCURRENT_CALLS);

    // Mutex para proteger el acceso concurrente al grafo
    std::mutex graphMutex;

    // 3. Recorrer cada película de la filmografía y lanzar un hilo para obtener su elenco
    std::vector<std::thread> threads;
    for (const MovieData& movie : filmography) {
        // Adquirir semáforo antes de lanzar un nuevo hilo (limita las llamadas concurrentes)
        apiSemaphore.acquire();
        // Copiar datos necesarios para el hilo (para evitar capturas por referencia inválidas)
        int movieId = movie.id;
        std::string movieTitle = movie.title;
        int movieYear = movie.year;

        // Lanzar thread para obtener el elenco de la película
        threads.emplace_back([&, movieId, movieTitle, movieYear]() {
            // 4. Dentro del hilo: obtener el reparto completo de la película usando TMDBAPIUtils
            std::vector<ActorData> cast = TMDBAPIUtils::getMovieCast(movieId);
            // ActorData es una estructura definida en TMDBAPIUtils que contiene al menos:
            //    - id (int): ID del actor en TMDB
            //    - name (std::string): nombre del actor

            // Bloquear el mutex antes de modificar el grafo compartido
            std::lock_guard<std::mutex> lock(graphMutex);
            // Agregar todos los actores de este reparto al grafo (nodos)
            for (const ActorData& actor : cast) {
                graph.addActor(actor.id, actor.name);
            }
            // Agregar aristas de colaboración entre cada par de actores de este reparto
            // (Esto incluye al actor principal, que también debería estar en `cast`)
            size_t n = cast.size();
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = i + 1; j < n; ++j) {
                    int actorId1 = cast[i].id;
                    int actorId2 = cast[j].id;
                    // Registrar colaboración entre actorId1 y actorId2 en esta película
                    graph.addCollaboration(actorId1, actorId2, movieTitle, movieYear);
                }
            }
            // Al salir del bloque, lock_guard libera el mutex automáticamente

            // Liberar el semáforo para permitir que otro hilo inicie su llamada a la API
            apiSemaphore.release();
        });
    }

    // Esperar a que todos los threads terminen (join)
    for (std::thread& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // 5. (Después de threads) Ahora el grafo contiene todos los actores y colaboraciones recopiladas.
    std::cout << "Películas procesadas: " << filmography.size() << ". ";
    std::cout << "Total de actores en grafo: " << graph.numActors() 
              << ", colaboraciones: " << graph.numCollaborations() << std::endl;

    // 6. Exportar el grafo a un archivo DOT para visualización con Graphviz
    std::string outputFile = "colaboraciones.dot";
    if (graph.exportToDot(outputFile)) {
        std::cout << "Grafo exportado a " << outputFile 
              << ". Generando imagen SVG con Graphviz...\n";
              
        // Ejecutar comando de sistema para generar SVG usando Graphviz
        int result = std::system("dot -Tsvg colaboraciones.dot -o grafo.svg");
        if (result == 0) {
            std::cout << "Imagen generada exitosamente: grafo.svg\n";
        } else {
            std::cerr << "Error al ejecutar Graphviz (dot). Código: " << result << "\n";
        }
    } else {
        std::cerr << "Error: no se pudo exportar el grafo a DOT.\n";
    }

    return 0;
}