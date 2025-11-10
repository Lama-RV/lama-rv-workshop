FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    git \
    ocaml \
    dune \
    make \
    gcc \
    pkg-config \
    qemu-user \
    gcc-riscv64-linux-gnu \
    diffutils \
    bash \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
COPY . .
RUN chmod +x ./run_tests.sh

CMD ["make", "-C", "regression", "check"]