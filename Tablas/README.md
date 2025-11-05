UnU

# Incidents Analytics (SQLite + C++ + Docker)

Este proyecto:

1) Crea tablas `incidents`, `details`, `outcomes` en SQLite.
2) Carga **paralelamente** (hilos + semáforo) los CSV del directorio `data/`.
3) Crea índices en `report_id`.
4) Ejecuta consultas para los incisos (a)-(e).
5) Genera **gráficas PNG** en `outputs/` usando `matplotlib-cpp`.


## Requisitos
- Docker y Docker Compose.


## Instrucciones rápidas

```bash
docker compose build
docker compose up --remove-orphans --force-recreate
```

# Resultados

- Base de datos: ./outputs/incidents.db
- Gráficas: ./outputs/*.png