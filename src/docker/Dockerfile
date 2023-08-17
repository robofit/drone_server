# Use a base image that provides necessary dependencies
FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Update and install required packages
RUN apt-get update && \
    apt-get install -y \
        make cmake yasm nasm pkg-config libssl-dev libboost-all-dev libx264-dev libavdevice-dev g++ libopencv-dev \
        git libmariadb-dev software-properties-common

# Build and install FFMPEG
RUN add-apt-repository ppa:ubuntuhandbook1/ffmpeg6 -y && \
    apt-get update && \
    apt-get install ffmpeg -y

# Install the newest libswscale version
RUN apt-get install -y libswscale-dev

#Build and install websocketpp
RUN git clone https://github.com/zaphoyd/websocketpp.git && \
    cd websocketpp && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make install

# Build and install mariadbpp
RUN git clone https://github.com/viaduck/mariadbpp.git && \
    cd mariadbpp && \
    git submodule update --init && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make install

# Clone and build the drone_server repository
RUN git clone https://github.com/robofit/drone_server.git && \
    cd drone_server/src && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

# Set up the working directory
WORKDIR /drone_server

# Run the application
ENTRYPOINT ["./src/build/DDSa"]

