FROM ubuntu:24.04


ENV DEBIAN_FRONTEND=noninteractive


# Herramientas de compilación + SQLite + Python (para matplotlib)
RUN apt-get update && apt-get install -y \
build-essential cmake git curl \
libsqlite3-dev sqlite3 \
python3 python3-dev python3-pip \
&& rm -rf /var/lib/apt/lists/*


# Libs Python para gráficos
RUN pip3 install --no-cache-dir matplotlib numpy


# App
WORKDIR /app
COPY CMakeLists.txt ./
COPY third_party ./third_party
COPY src ./src
COPY data ./data


# Construcción
RUN mkdir -p build && cd build && \
cmake -DPython3_ROOT_DIR=/usr -DPython3_EXECUTABLE=/usr/bin/python3 .. && \
cmake --build . -j


# Salidas
RUN mkdir -p /app/outputs


# Comando por defecto: ejecuta el binario (carga CSV, crea índices y genera PNG)
CMD ["/app/build/incidents_analytics"]