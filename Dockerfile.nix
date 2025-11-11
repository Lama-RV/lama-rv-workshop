FROM nixos/nix:2.32.3

RUN mkdir -p /root/.config/nix \
 && echo "experimental-features = nix-command flakes" >> /root/.config/nix/nix.conf

WORKDIR /workspace
COPY flake.nix flake.lock /workspace/
COPY . /workspace/

ENTRYPOINT ["/bin/sh"]
CMD []