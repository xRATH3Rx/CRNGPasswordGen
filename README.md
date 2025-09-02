
# pwtool — Windows CLI Password Generator (C++17 + OpenSSL)

`pwtool` is a tiny, fast command‑line password generator for Windows. It uses OpenSSL’s CSPRNG to create strong passwords that meet a sensible default policy, and it can export to TXT and Excel‑friendly CSV.

> **Default policy**
> - Length ≥ 16
> - At least 2 uppercase, 2 lowercase, 2 digits
> - Specials optional (≥ 2 if enabled). You can disable or override the special set.

---

## Features

- Cryptographically secure randomness via `RAND_bytes` (OpenSSL).
- Bias‑free selection & Fisher–Yates shuffle.
- Sensible defaults; configurable length/sets.
- Windows‑friendly line endings for TXT and UTF‑8 BOM for CSV (plays nicely with Excel).
- Quiet mode for piping.
- Helpful errors and exit codes.

---

## Requirements

- **C++17** compiler (MSVC or MinGW).
- **OpenSSL** (library + headers).

### Get OpenSSL (options)
- **MSVC + vcpkg**
```
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install openssl
# Then integrate or point your include/lib paths to vcpkg\installed\<triplet>\{include,lib}
```
- **MSYS2 / MinGW-w64**
```
pacman -S mingw-w64-x86_64-openssl
```
- **Chocolatey (prebuilt OpenSSL for MSVC)**
```
choco install openssl
```

> Make sure `libcrypto` is available to your linker, and runtime DLLs (e.g., `libcrypto-*.dll`) are on `PATH` or next to the executable.

---

## Build

### MSVC (Developer Command Prompt)
```
cl /std:c++17 pwtool.cpp libcrypto.lib /EHsc
```

If you’re using vcpkg, add the library path, for example:
```
cl /std:c++17 pwtool.cpp /I <vcpkg>\installedd-windows\include ^
   /link /LIBPATH:<vcpkg>\installedd-windows\lib libcrypto.lib
```

### MinGW-w64 (MSYS2)
```
g++ -std=c++17 pwtool.cpp -lcrypto -o pwtool.exe
```

---

## Usage

```
pwtool [options]

Options:
  -n <N>          Number of passwords (default 1)
  -l <L>          Password length (>=16, default 16)
  --specials s    Override special characters set
  --nospecial     Exclude special characters entirely
  -txt <file>     Save passwords to a .txt file (one per line)
  -csv <file>     Save passwords to a CSV (Excel-friendly, numbered)
  -q              Quiet mode (only print passwords to stdout)
  -h              Show help
```

> The tool guarantees at least two characters from each selected category (U/L/D and, unless `--nospecial`, specials). The remainder is filled from the union and shuffled.

---

## Examples

Generate one 20‑char password and print it:
```
pwtool -l 20
```

Generate 50 passwords of length 24, **no** special characters:
```
pwtool -n 50 -l 24 --nospecial
```

Use a custom special set and export to TXT + CSV:
```
pwtool -n 25 -l 18 --specials "^~!?" -txt out.txt -csv out.csv
```

Quiet mode (prints only passwords — great for piping):
```
pwtool -q -n 3 -l 20 > passwords.txt
```

---

## Output files

- **TXT**: One password per line, CRLF line endings (`
`) for Windows tools.
- **CSV**: UTF‑8 with BOM, header `#,Password`, 1‑based numbering. Values are minimally CSV‑escaped, and each row ends with `
`.

---

## Security notes

- Random bytes are produced by OpenSSL `RAND_bytes`. If the call fails, the program exits with an error.
- Index selection uses rejection sampling to avoid modulo bias.
- A Fisher–Yates shuffle (with CSPRNG indices) randomizes character placement.
- The generator enforces a minimum length (16) and per‑category minimums to avoid weak outputs when categories are enabled.

> **Operational hygiene**: Treat generated passwords as secrets. When exporting to files, ensure the output paths are secured and clean up files if not needed.

---

## Exit codes

- `0` — Success
- `1` — Error (invalid arguments, file I/O, or RNG failure).

---

## FAQ

**Q: Why do my CSVs open with correct columns in Excel?**  
A: The file starts with a UTF‑8 BOM and uses CRLF line endings; values are quoted/escaped when needed.

**Q: Can I use it on Linux/macOS?**  
A: The code is portable C++17 and only needs OpenSSL. The build commands above target Windows, but you can compile similarly on other platforms.

**Q: How do I change the allowed special characters?**  
A: Use `--specials "<chars>"`. To disable them entirely, pass `--nospecial`.

---

## License

MIT

---

## Source layout

This repo contains a single source file:

- `pwtool.cpp` — all logic (CSPRNG helpers, generator, CLI, TXT/CSV writers).

---

## Acknowledgements

- OpenSSL for CSPRNG (`RAND_bytes`).
- Classic Fisher–Yates shuffle.
