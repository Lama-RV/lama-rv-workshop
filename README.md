# RISC-V backend for Lama bytecode
## Build
```bash
make build
```
## Regression tests
```bash
make regression
```
## Performance tests
```bash
make -C performance
```

## Getting environment
The environment for the development of this project is described via nix.

To install nix you can follow [this link](https://nixos.org/download/).
In addition, it is necessary to enable support for experimental functions,
the easiest way to do this is with the following command.
```
echo 'experimental-features = nix-command flakes' >> ~/.config/nix/nix.conf
```

After this, you are ready to enter environment with command:
```
nix develop
```
In first time it can take some time to download and build prerequisites.
