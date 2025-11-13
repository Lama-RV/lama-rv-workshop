FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    curl build-essential \
    gcc-13 g++-13 libstdc++-13-dev \
    make cmake pkg-config \
    libc6-dev libncurses5-dev \
    libgoogle-glog-dev \
    ocaml-dune qemu-user gcc-riscv64-linux-gnu diffutils bash git \
    openssh-server \
 || true \
 && apt-get -f install -y \
 && ln -sf /usr/bin/riscv64-linux-gnu-gcc /usr/local/bin/riscv64-unknown-linux-gnu-gcc \
 && ln -sf /usr/bin/riscv64-linux-gnu-ar  /usr/local/bin/riscv64-unknown-linux-gnu-ar \
 && rm -rf /var/lib/apt/lists/*

RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.27.8/cmake-3.27.8-linux-x86_64.sh \
    -o /tmp/cmake.sh && chmod +x /tmp/cmake.sh \
    && /tmp/cmake.sh --skip-license --prefix=/usr/local \
    && rm /tmp/cmake.sh

RUN mkdir -p /var/run/sshd /root/.ssh && chmod 700 /root/.ssh && \
    echo 'root:root' | chpasswd && \
    sed -i 's/#PubkeyAuthentication.*/PubkeyAuthentication yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication.*/PasswordAuthentication no/' /etc/ssh/sshd_config && \
    sed -i 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' /etc/pam.d/sshd

EXPOSE 22

WORKDIR /workspace
COPY . .

RUN cmake -S comp -B comp-build \
    -DCMAKE_C_COMPILER=/usr/bin/gcc-13 \
    -DCMAKE_CXX_COMPILER=/usr/bin/g++-13 \
    -DCMAKE_CXX_STANDARD=20 \
 && cmake --build comp-build

CMD ["/usr/sbin/sshd", "-D"]