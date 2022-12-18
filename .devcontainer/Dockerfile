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







RUN groupadd --gid 1000 dev \
    && useradd --home-dir /home/dev --create-home --uid 1000 \
    --gid 1000 --shell /bin/bash dev --groups sudo

RUN sed -i -e '$a%dev ALL=(ALL)" NOPASSWD:ALL' /etc/sudoers


USER dev

CMD ["bash"]