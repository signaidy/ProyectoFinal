UnU# Proyecto: Grafo de Colaboración de Actores (TMDB + C++)

Este proyecto construye y visualiza un grafo de colaboración de actores utilizando datos de la API de The Movie Database (TMDB). El grafo muestra qué actores han trabajado juntos, etiquetando cada arista con la película más reciente en la que colaboraron. Se implementa en **C++20** y usa **Graphviz** para visualizar el grafo resultante.

---

## 🔧 Requisitos

* **Compilador:** Visual Studio 2019/2022 (MSVC) con soporte C++20.
* **Dependencias externas:**

  * [`cpr`](https://github.com/libcpr/cpr) — para peticiones HTTP.
  * [`nlohmann/json`](https://github.com/nlohmann/json) — para parseo de JSON.
  * [`Graphviz`](https://graphviz.org/) — para visualizar el archivo `.dot` generado.

### Instalación de dependencias vía vcpkg (recomendado)

```bash
# Instala vcpkg si no lo tienes ya
https://github.com/microsoft/vcpkg#quick-start

# Instala las bibliotecas necesarias
vcpkg install cpr nlohmann-json
```

---

## 🚀 Compilación (usando MSVC + vcpkg)

### 1. Clona el repositorio y navega al directorio del proyecto

```
git clone <este-proyecto>
cd tmdb-grafo-cpp
```

### 2. Compila con el siguiente comando

```bash
cl /std:c++20 /I"<ruta-a-vcpkg>\installed\x64-windows\include" \
    main.cpp TMDBAPIUtils.cpp Graph.cpp \
    /link /LIBPATH:"<ruta-a-vcpkg>\installed\x64-windows\lib" cpr.lib
```

Reemplaza `<ruta-a-vcpkg>` con el path correcto en tu máquina.

---

## 🧠 Cómo funciona

1. Define el actor principal (Keanu Reeves) y un rango de años.
2. Obtiene la filmografía del actor usando la API de TMDB.
3. Lanza múltiples hilos (con límite usando semáforo) para obtener el elenco de cada película.
4. Construye un grafo con nodos = actores y aristas = colaboraciones etiquetadas con la película más reciente.
5. Exporta el grafo en formato DOT (`colaboraciones.dot`).

---

## 📈 Visualización con Graphviz

Una vez generado el archivo `.dot`, puedes renderizarlo con:

```bash
dot -Tpng colaboraciones.dot -o grafo.png
```

O si prefieres SVG:

```bash
dot -Tsvg colaboraciones.dot -o grafo.svg
```

---

## 📝 Notas adicionales

* La API Key de TMDB debe ser insertada manualmente en `TMDBAPIUtils.h`.
* Se utilizan threads y semáforo (C++20) para mejorar rendimiento de llamadas HTTP.
* Graphviz solo se usa para visualización, no es una dependencia de compilación.

---

## 📬 Créditos

Proyecto académico UNIS — implementación en C++ por Carlos Solares.
