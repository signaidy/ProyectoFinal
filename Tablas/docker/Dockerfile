FROM ubuntu:24.04


ENV DEBIAN_FRONTEND=noninteractive


# Herramientas de compilación + SQLite + Python (para matplotlib)
RUN apt-get update && apt-get install -y \
build-essential cmake curl git \
libsqlite3-dev \
python3 python3-dev python3-matplotlib python3-numpy python3-pip sqlite3 \
&& rm -rf /var/lib/apt/lists/*

# App
WORKDIR /app
COPY CMakeLists.txt ./
COPY third_party ./third_party
COPY src ./src
COPY data ./data


# Construcción & Salidas
RUN mkdir -p build && cd build && \
cmake -DPython3_ROOT_DIR=/usr -DPython3_EXECUTABLE=/usr/bin/python3 .. && \
cmake --build . -j && \
mkdir -p /app/outputs

# Comando por defecto: ejecuta el binario (carga CSV, crea índices y genera PNG)
CMD ["/app/build/incidents_analytics"]

USER root