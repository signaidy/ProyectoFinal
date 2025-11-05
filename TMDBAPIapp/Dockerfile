FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    unzip \
    graphviz \
    libcurl4-openssl-dev \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /deps
RUN git clone https://github.com/nlohmann/json.git && \
    cd json && mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr && \
    make -j && make install

RUN git clone https://github.com/libcpr/cpr.git && \
    cd cpr && mkdir build && cd build && \
    cmake .. -DCMAKE_USE_OPENSSL=ON -DCMAKE_INSTALL_PREFIX=/usr && \
    make -j && make install

WORKDIR /app
COPY . .

RUN g++ -std=c++20 -o grafo-cpp main.cpp TMDBAPIUtils.cpp Graph.cpp \
    -lcpr -lssl -lcrypto -lpthread

# Ejecuta el grafo y genera la imagen automáticamente
CMD ./grafo-cpp && dot -Tpng colaboraciones.dot -o grafo.png