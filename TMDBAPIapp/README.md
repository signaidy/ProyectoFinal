# Proyecto: Grafo de Colaboración de Actores (TMDB + C++)

Este proyecto construye y visualiza un grafo de colaboración de actores utilizando datos de la API de The Movie Database (TMDB). El grafo muestra qué actores han trabajado juntos, etiquetando cada arista con la película más reciente en la que colaboraron. Se implementa en **C++20** y usa **Graphviz** para visualizar el grafo resultante.

---

## 🔧 Requisitos

- **Docker + Docker Compose** (recomendado para evitar instalar dependencias manualmente)
- O bien:
  - **Compilador:** Visual Studio 2019/2022 (MSVC) con soporte C++20.
  - **Dependencias externas:**
    - [`cpr`](https://github.com/libcpr/cpr) — para peticiones HTTP.
    - [`nlohmann/json`](https://github.com/nlohmann/json) — para parseo de JSON.
    - [`Graphviz`](https://graphviz.org/) — para visualizar el archivo `.dot` generado.

---

## 🚀 Uso con Docker

### 1. Agrega tu API Key en el código

Abre `TMDBAPIUtils.h` y reemplaza:

```cpp
static constexpr const char* api_key = "<TU_API_KEY_HERE>";
````

### 2. Ejecuta todo el flujo

```bash
docker compose up --build
```

Esto:

* Descargará dependencias (`cpr`, `nlohmann/json`, `graphviz`)
* Compilará el proyecto en C++
* Ejecutará el binario `grafo-cpp`
* Generará `colaboraciones.dot` y `grafo.png`

### 3. Visualiza el grafo generado

Abre `grafo.png` desde tu sistema operativo para ver el grafo de colaboraciones de actores.

---

## 🛠️ Uso sin Docker (instalación manual)

### 1. Instala las dependencias manualmente

#### En Windows (vía vcpkg)

```bash
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
vcpkg install cpr nlohmann-json
```

#### En Linux (apt)

```bash
sudo apt update
sudo apt install build-essential cmake libcurl4-openssl-dev libssl-dev graphviz
```

Luego instala las dependencias:

```bash
git clone https://github.com/libcpr/cpr.git
cd cpr && mkdir build && cd build
cmake .. -DCMAKE_USE_OPENSSL=ON
make -j && sudo make install
```

```bash
git clone https://github.com/nlohmann/json.git
cd json && mkdir build && cd build
cmake .. && make -j && sudo make install
```

---

### 2. Agrega tu API Key

Edita el archivo `TMDBAPIUtils.h` y reemplaza:

```cpp
static constexpr const char* api_key = "<TU_API_KEY_HERE>";
```

---

### 3. Compila el proyecto

```bash
g++ -std=c++20 main.cpp TMDBAPIUtils.cpp Graph.cpp -o grafo-cpp -lcpr -lssl -lcrypto -pthread
```

O si estás en Windows usando MSVC:

```bash
cl /std:c++20 main.cpp TMDBAPIUtils.cpp Graph.cpp /I\"<ruta a vcpkg>/installed/x64-windows/include\" /link /LIBPATH:\"<ruta a vcpkg>/installed/x64-windows/lib\" cpr.lib
```

---

### 4. Ejecuta y visualiza

```bash
./grafo-cpp
dot -Tpng colaboraciones.dot -o grafo.png
```

Abre `grafo.png` para ver el resultado.

---

## 🧠 Cómo funciona

1. Define el actor principal (Keanu Reeves) y un rango de años.
2. Obtiene la filmografía del actor usando la API de TMDB.
3. Lanza múltiples hilos (con límite usando semáforo) para obtener el elenco de cada película.
4. Construye un grafo con nodos = actores y aristas = colaboraciones etiquetadas con la película más reciente.
5. Exporta el grafo en formato DOT (`colaboraciones.dot`).

---

## 📈 Visualización con Graphviz (modo local)

Una vez generado el archivo `.dot`, si usas instalación local:

```bash
dot -Tpng colaboraciones.dot -o grafo.png
```

---

## 📝 Notas adicionales

* El contenedor ejecuta automáticamente Graphviz (`dot`) al finalizar.
* Se utilizan threads y semáforo (C++20) para optimizar llamadas HTTP.
* Puedes editar el actor base en `main.cpp` (por defecto: Keanu Reeves, ID 6384).

---

## 📬 Créditos

Proyecto académico UNIS — implementación en C++ por Carlos Solares.
