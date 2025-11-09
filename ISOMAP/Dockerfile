FROM ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive

# Instalar dependencias
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libeigen3-dev \
    && rm -rf /var/lib/apt/lists/*

# Crear carpeta de trabajo
WORKDIR /build

# Copiar código fuente
COPY . .

# Compilar el programa
RUN g++ -std=c++20 -O2 -o isomap src/main.cpp \
    -I/usr/include/eigen3

# Mover ejecutable a lugar accesible
RUN mkdir -p /output && cp isomap /output/

# Carpeta de ejecución
WORKDIR /output

# Ejecutar y dejar los resultados ahí
CMD ../build/isomap ../data/isomap.dat