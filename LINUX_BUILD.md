# Pwtool (Linux Build Guide)
## Requirements

- A C++ compiler (g++)
- OpenSSL development headers

### Install dependencies (Debian/Ubuntu/Kali)

```bash
sudo apt update
sudo apt install g++ libssl-dev
```

> If you’re on Fedora:
> ```bash
> sudo dnf install gcc-c++ openssl-devel
> ```
> On Arch:
> ```bash
> sudo pacman -S gcc openssl
> ```

## Build

From the project root (adjust the filename if yours differs, e.g. `pwtool.cpp` vs `PasswordGen.cpp`):

```bash
# Basic build
g++ -O2 -Wall -o pwtool pwtool.cpp -lssl -lcrypto

# Or, if your file is named PasswordGen.cpp
g++ -O2 -Wall -o PasswordGen PasswordGen.cpp -lssl -lcrypto
```

## Run

```bash
./pwtool
# or
./PasswordGen
```

## Troubleshooting

### `fatal error: openssl/rand.h: No such file or directory`
You’re missing OpenSSL headers. Install them and rebuild:

```bash
sudo apt install libssl-dev
# then rebuild:
g++ -O2 -Wall -o pwtool pwtool.cpp -lssl -lcrypto
```

### Linker errors about `-lssl` / `-lcrypto`
Ensure `libssl-dev` (or your distro’s equivalent) is installed and that you included both `-lssl -lcrypto` in the compile command.

## Optional: Makefile

Create a simple `Makefile` for convenience (adjust the source filename):

```makefile
# Makefile
CXX := g++
SRC := pwtool.cpp
BIN := pwtool
CXXFLAGS := -O2 -Wall
LDLIBS := -lssl -lcrypto

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(BIN)
```

Then build with:
```bash
make
```
---
