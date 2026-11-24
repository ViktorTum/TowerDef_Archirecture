# Build and test

Needs: a C++17 compiler (GCC, Clang or MSVC), CMake 3.20+, and internet access
on the first configure (CMake downloads Catch2 once).

## Windows, MSYS2 UCRT64 (as used by the team)

```bash
cd /d/Progs/Develope/TowerDefence/TowerDef_Archirecture
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Without Ninja use `-G "MinGW Makefiles"`. With Visual Studio:

```
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Options

| Option | Default | Meaning |
|---|---|---|
| `TD_BUILD_TESTS` | ON | build the unit tests (needs Catch2) |
| `TD_WARNINGS_AS_ERRORS` | OFF | turn compiler warnings into errors |

## Tests

Each module builds one test executable `td_<module>_tests`, registered as one
ctest entry named `td_<module>`. To see individual test cases:

```
build/modules/core/td_core_tests --list-tests
build/modules/core/td_core_tests "[hash]"
```

Test names are in English on purpose: non-ASCII names get mangled by some
Windows consoles.
