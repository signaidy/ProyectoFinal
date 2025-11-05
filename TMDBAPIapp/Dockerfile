FROM ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    git \
    graphviz \
    libcpr-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    nlohmann-json3-dev \
    pkg-config \
    unzip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

# Compilar y mover el binario a ruta segura fuera del volumen montado
RUN g++ -std=c++20 -o grafo-cpp main.cpp TMDBAPIUtils.cpp Graph.cpp \
    -lcpr -lssl -lcrypto -lpthread && \
    cp grafo-cpp /usr/local/bin/grafo-cpp

# El contenedor trabajará en la carpeta compartida para dejar los resultados
WORKDIR /output
CMD grafo-cpp && dot -Tpng colaboraciones.dot -o grafo.png