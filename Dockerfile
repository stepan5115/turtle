FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++-13 gcc-13 make build-essential default-jdk wget git cmake \
    libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev pkg-config \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100 \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

CMD ["bash"]