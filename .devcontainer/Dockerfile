ARG FROM_IMAGE=ubuntu:jammy
FROM $FROM_IMAGE

# setup environment
ENV LANG en_US.utf8
ENV LANGUAGE en_US.utf8
ENV LC_ALL C.UTF-8

ENV DEBIAN_FRONTEND=noninteractive

# install packages
RUN apt-get update && apt-get install -q -y --no-install-recommends \
    sudo  \
    build-essential \
    software-properties-common \
    git \
    curl \
    cmake \
    automake \
    gdb \
    wget \
    unzip \
    zip \
    clangd \
    clang-format \
    clang-tidy \
    && rm -rf /var/lib/apt/lists/*


RUN apt-get update && apt-get install -q -y --no-install-recommends \
    libasio-dev \
    libwebsocketpp-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libboost-all-dev \
    nlohmann-json3-dev \
    libgl1-mesa-glx \
    libxrandr-dev \
    libfreetype-dev \
    && rm -rf /var/lib/apt/lists/*


RUN git clone https://github.com/OpenSimulationInterface/open-simulation-interface.git && \
    cd open-simulation-interface && \
    git checkout v3.3.1 && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    sudo make install

RUN apt-get update && apt-get install -q -y --no-install-recommends \
    libqt5gui5 \
    libqt5widgets5 \
    libqwt-qt5-6 \
    libyaml-cpp0.7  \
    libhdf5-103-1 \
    libglm-dev \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://github.com/eclipse-ecal/ecal/releases/download/v5.11.0/ecal_5.11.0-jammy_amd64.deb -O ecal.deb && \
    dpkg -i ecal.deb


RUN groupadd --gid 1000 dev \
    && useradd --home-dir /home/dev --create-home --uid 1000 \
    --gid 1000 --shell /bin/bash dev --groups sudo

RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER dev

CMD ["bash"]