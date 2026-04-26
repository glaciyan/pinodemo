# CLAUDE.md — AI assistant instructions for pinodemo

## Build folder

Always use `cmake-build-claude` as the build directory, never `cmake-build-debug-visual-studio`.
The latter is the user's CLion-managed build and must not be touched.

## Tool paths (this machine)

```
cmake:  C:\Users\kevin\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe
ninja:  C:\Users\kevin\AppData\Local\Programs\CLion\bin\ninja\win\x64\ninja.exe
vcpkg:  C:\Users\kevin\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake
vcvars: C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
```

## Configure command

```powershell
$vcvars = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
$cmake  = "C:\Users\kevin\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe"
$ninja  = "C:\Users\kevin\AppData\Local\Programs\CLion\bin\ninja\win\x64\ninja.exe"
$vcpkg  = "C:\Users\kevin\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake"
$src    = "C:\dev\pinodemo"
$build  = "C:\dev\pinodemo\cmake-build-claude"

cmd /c "`"$vcvars`" && `"$cmake`" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=`"$ninja`" -DCMAKE_TOOLCHAIN_FILE=`"$vcpkg`" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md -G Ninja -S `"$src`" -B `"$build`" 2>&1"
```

## Build command

```powershell
$vcvars = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
$cmake  = "C:\Users\kevin\AppData\Local\Programs\CLion\bin\cmake\win\x64\bin\cmake.exe"
$build  = "C:\dev\pinodemo\cmake-build-claude"

cmd /c "`"$vcvars`" && `"$cmake`" --build `"$build`" --target MyProject -j 4 2>&1"
```

Always use `cmd /c` with backtick-escaped inner quotes to source vcvars before cmake.
vcvars must be sourced before every cmake invocation so MSVC is on PATH.
These commands run from PowerShell, not bash.

## vcpkg triplet

`x64-windows-static-md` — Boost and most dependencies are static; MSVC CRT is dynamic (`/MD`).
urdfdom is always dynamic in vcpkg regardless of triplet (4 DLLs).

## Linking strategy

- Pinocchio: built as shared libraries (`BUILD_SHARED_LIBS ON`) to avoid MSVC static export hacks
- Boost: static via vcpkg triplet
- urdfdom: always dynamic (vcpkg limitation); 4 DLLs shipped
- All 6 runtime DLLs are copied next to the executable via a `POST_BUILD` custom command

## DLL staging

`PINOCCHIO_RUNTIME_DLLS` in `CMakeLists.txt` is the explicit list of DLLs to stage.
Uses `$<TARGET_FILE:target>` generator expressions — handles Debug/Release paths automatically.
urdfdom targets (`urdfdom::urdfdom_model` etc.) must be in scope via `find_package(urdfdom CONFIG REQUIRED)` in the top-level CMakeLists before they can be referenced.

## Checking dependencies

Use Dependencies.exe to inspect binary dependency chains:
```
"C:\Users\kevin\Downloads\Dependencies_x64_Release\Dependencies.exe" -chain MyProject.exe
"C:\Users\kevin\Downloads\Dependencies_x64_Release\Dependencies.exe" -modules MyProject.exe
```
