# Lanfia
Mafia over LAN

# Build
1. Obtain ./resources directory and place in the repo
2. Bootstrap the build system
```sh
cc build.c -o build
# cc - compiler of your choice (gcc, clang, e.t.c.)
```
3. Build:
```sh
# Linux
./build -target linux
# Linux -> Windows example
./build -cc x86_64-w64-mingw32-gcc -target windows
```

# Tests
```sh
./build -run -- -tests
```
