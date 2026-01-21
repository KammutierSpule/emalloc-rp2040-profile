FROM debian:trixie-slim

RUN export DEBIAN_FRONTEND=noninteractive \
&& apt-get -y update \
&& apt-get -y upgrade \
&& apt-get install -y --no-install-recommends -qq \
    sudo wget curl ca-certificates git ssh \
    build-essential make cmake ninja-build \
    clangd clang-format \
    cmake-format \
    python3 python3-pip \
    clang clang-tidy gdb \
    locales \
    gcc-arm-none-eabi binutils-arm-none-eabi \
    libnewlib-dev \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-dev libstdc++-arm-none-eabi-newlib \
&& apt-get purge --autoremove -qq \
&& rm -rf /var/lib/apt/lists/*

RUN update-ca-certificates

# Setup user and group names
ARG USER=vscode
ARG GROUP=${USER}_group
ARG USER_HOME_DIR=/home/${USER}

RUN groupadd -r ${GROUP} && \
    useradd -m -r -g ${GROUP} -s /bin/bash ${USER}

RUN echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USER} && \
    chmod 0440 /etc/sudoers.d/${USER}

RUN localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

ENV LANG en_US.utf8
ENV LANGUAGE en_US.utf8
ENV LC_ALL en_US.utf8

# Switch to user
USER ${USER}
WORKDIR ${USER_HOME_DIR}

# Install Raspberry Pi PICO SDK
ENV PICO_SDK_PATH=/home/${USER}/pico-sdk

RUN git clone https://github.com/raspberrypi/pico-sdk ${PICO_SDK_PATH} \
&& cd ${PICO_SDK_PATH} \
&& git submodule update --init
