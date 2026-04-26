# pinodemo

Minimal C++ demo of [Pinocchio 4](https://github.com/stack-of-tasks/pinocchio) on Windows.
Loads a UR5 URDF and runs forward kinematics.

## Dependencies

| Tool | Notes |
|------|-------|
| CMake >= 3.22 | [cmake.org](https://cmake.org) |
| Visual Studio 2022 | Desktop C++ workload required |
| Ninja | Bundled with CLion or available standalone |
| vcpkg | Dependencies installed automatically via `vcpkg.json` manifest |
| sccache (optional) | Speeds up incremental builds; auto-detected if on `PATH` |

Eigen3 and Pinocchio are included as git submodules.
Boost, urdfdom, and supporting libraries are fetched by vcpkg automatically on first configure.

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
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
```

Replace `<path-to-vcpkg>` with your vcpkg root, e.g. `C:\Users\you\.vcpkg-clion\vcpkg`.

vcpkg will automatically install all dependencies listed in `vcpkg.json` on first configure.

### CLion

Use the `debug-vs` preset defined in `CMakePresets.json`, or configure manually:

- Generator: `Ninja`
- Build type: `Debug`
- CMake options:
  ```
  -DCMAKE_TOOLCHAIN_FILE=C:\Users\you\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake
  -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
  ```

CLion handles the VS 2022 toolchain environment automatically when configured with a Visual Studio toolchain.

## Build

```bat
cmake --build build --target MyProject -j 4
```

## Run

```bat
build\MyProject.exe
```

Expected output (values vary — random configuration is sampled each run):

```
model name: ur5
q:  -6.26746  0.799037  -1.92703   3.87975   1.06826 -0.252923
universe                : 0.00 0.00 0.00
shoulder_pan_joint      : 0.00 0.00 0.09
shoulder_lift_joint     : -0.00 0.14  0.09
elbow_joint             :  0.30  0.02 -0.22
wrist_1_joint           :  0.46  0.02  0.14
wrist_2_joint           :  0.46  0.12  0.14
wrist_3_joint           :  0.43  0.12  0.23
```

## Linking strategy

| Library | How linked | Rationale |
|---------|-----------|-----------|
| Pinocchio | Shared (2 DLLs) | Avoids complex static export workarounds on MSVC |
| urdfdom | Shared (4 DLLs) | vcpkg does not support static urdfdom on Windows |
| Boost | Static | `x64-windows-static-md` triplet |
| Eigen3 | Header-only | — |
| MSVC CRT | Dynamic (`/MD`) | Required by `x64-windows-static-md` |

The 6 DLLs (`pinocchio_default.dll`, `pinocchio_parsers.dll`, `urdfdom_model.dll`,
`urdfdom_model_state.dll`, `urdfdom_world.dll`, `urdfdom_sensor.dll`) are automatically
copied next to the executable at build time. No PATH configuration is needed to run.

The MSVC runtime (`msvcp140.dll`, `vcruntime140.dll`) must be present on the target machine.
Ship `vc_redist.x64.exe` alongside your installer if targeting machines without Visual Studio.

## Project layout

```
CMakeLists.txt          build configuration
CMakePresets.json       CLion / IDE presets
vcpkg.json              vcpkg dependency manifest
src/main.cpp            demo: load UR5 URDF, run forward kinematics
lib/eigen3/             Eigen 3.4 (git submodule)
lib/pinocchio/          Pinocchio 4.0 (git submodule)
```
