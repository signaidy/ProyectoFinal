# Incidents Analytics (SQLite + C++ + Docker)

Este proyecto analiza datos de incidentes a partir de archivos CSV, utilizando C++, SQLite y visualizaciones con `matplotlib-cpp`. 

## Funcionalidades principales

1. Crea las tablas `incidents`, `details` y `outcomes` en una base de datos SQLite.
2. Carga los archivos CSV desde `./data/` en paralelo usando hilos y semáforos.
3. Limpia y normaliza los datos antes de insertarlos.
4. Crea índices para acelerar consultas (`report_id`).
5. Ejecuta una serie de análisis predefinidos:
   - (a) Total de incidentes por año (2018–2020)
   - (b) Medios de transporte más comunes con detección por inteligencia
   - (c) Promedio de arrestos por método de detección (top 10)
   - (d) Categorías con mayores sentencias (en días)
   - (e) Multas totales por año
6. Genera gráficas PNG en `./outputs/`
7. Modo diagnóstico opcional para revisar estructura y salud de la base de datos

---

## Requisitos

- Docker
- Docker Compose

---

## Instrucciones de uso

### 1. Construir el contenedor

```bash
docker compose build
```

> Si quieres reconstruir desde cero (sin usar caché):

```bash
docker compose build --no-cache
```

---

### 2. Ejecutar análisis normal

```bash
docker compose run --rm analytics
```

Esto realiza toda la carga, procesamiento y genera las gráficas. Verás en consola los resultados para los incisos (a) a (e).

---

### 3. Ejecutar en modo diagnóstico

```bash
docker compose run --rm analytics /app/build/incidents_analytics --diagnostico
```

Esto genera un reporte completo con:

- Conteo de registros en cada tabla
- Años mínimo y máximo
- Distribución de métodos de detección y medios de transporte
- Suma de arrestos y multas
- Diagnóstico de unidades de sentencia
- Primeras filas de la tabla `incidents`

El resultado se guarda también como archivo de texto en:

``` path
./outputs/images/diagnostico.txt
```

---

## Salidas del sistema

- 📊 **Gráficas PNG:** en `./outputs/`
- 🗃️ **Base de datos SQLite:** en `./outputs/incidents.db`
- 📄 **Diagnóstico (si se ejecuta):** en `./outputs/images/diagnostico.txt`

---

## Estructura del proyecto

``` Estructura
├── data/                   # CSVs de entrada (incidents.csv, details.csv, outcomes.csv)
├── outputs/                # Resultados: gráficos PNG y DB
│   └── images/             # Subcarpeta para gráficas y diagnósticos
├── src/                    # Código fuente en C++
├── docker/                 # Dockerfile y configuración
├── CMakeLists.txt          # Compilación con CMake
└── docker-compose.yml
```
