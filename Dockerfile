FROM ubuntu:18.04 as builder
RUN apt-get update \
    && apt-get -y install \
    iproute2 \
    libtbb-dev \
    libboost-all-dev \
    cmake \
    ninja-build \
    gdb

WORKDIR /simple_P2P

COPY create_configs.sh CMakeLists.txt ./
COPY include include
COPY src src

RUN ./create_configs.sh \
    && cd build/Release \
    && make

FROM ubuntu:18.04
RUN apt-get update \
    && apt-get -y install \
    iproute2 \
    libtbb-dev \
    libboost-all-dev 

WORKDIR /simple_P2P
COPY --from=builder /simple_P2P/bin/Release/Simple_P2P .
ENTRYPOINT ./Simple_P2P \
    --my_ip $(hostname -I | awk '{print $1;}') \
    --broadcast_ip $(ip -o -f inet addr show | awk '/scope global/ {print $6}' | head -n 1)