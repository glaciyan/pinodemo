# pinodemo

Minimal C++ demo of [Pinocchio 4](https://github.com/stack-of-tasks/pinocchio) on Windows.
Builds a manipulator model and runs the Recursive Newton-Euler Algorithm (RNEA).

## Dependencies

| Tool | Notes |
|------|-------|
| CMake >= 3.22 | [cmake.org](https://cmake.org) |
| Visual Studio 2022 | Desktop C++ workload required |
| vcpkg | Boost is installed automatically via `vcpkg.json` manifest |
| sccache (optional) | Speeds up incremental builds; auto-detected if on `PATH` |

Eigen3 is included as a git submodule. Boost is fetched by vcpkg automatically on first configure.

## Clone

```bat
git clone --recurse-submodules <repo-url>
cd pinodemo
```

If you already cloned without `--recurse-submodules`:

```bat
git submodule update --init --recursive
```

## Configure

Run from a **VS 2022 x64 Developer Command Prompt** (required for the MSVC compiler environment).

```bat
cmake -B build -S . ^
  -G Ninja ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows-static
```

Replace `<path-to-vcpkg>` with your vcpkg root, e.g. `C:\Users\you\.vcpkg-clion\vcpkg`.

vcpkg will automatically install all Boost dependencies listed in `vcpkg.json` on first configure.

### CLion

Add the following to your CMake profile's **CMake options**:

```
-DCMAKE_TOOLCHAIN_FILE=C:\Users\you\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake
-DVCPKG_TARGET_TRIPLET=x64-windows-static
```

Set the build type to `RelWithDebInfo` and the generator to `Ninja`.
Builds must be launched from a VS 2022 x64 developer shell (CLion handles this automatically when configured with a Visual Studio toolchain).

## Build

```bat
cmake --build build --target MyProject -j 4
```

## Run

```bat
build\MyProject.exe
```

Expected output:

```
tau = 0 0 0 0 0 0
```

## Static linking

Everything is statically linked into `MyProject.exe`:

- Boost libraries (`x64-windows-static` vcpkg triplet)
- Pinocchio (`BUILD_SHARED_LIBS=OFF`)
- MSVC runtime (`/MT`)

No DLLs are required at runtime.

## Project layout

```
CMakeLists.txt          build configuration
vcpkg.json              Boost dependency manifest
src/main.cpp            demo: RNEA on a 6-DOF manipulator
lib/eigen3/             Eigen 3.4 (git submodule)
lib/pinocchio/          Pinocchio 4.0 (git submodule)
```
