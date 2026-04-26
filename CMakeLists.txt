cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

# ---------------------------------------------------------------
# sccache - must be set before project() / enable_language()
# ---------------------------------------------------------------
find_program(SCCACHE sccache)
if(SCCACHE)
  set(CMAKE_C_COMPILER_LAUNCHER   "${SCCACHE}" CACHE STRING "")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${SCCACHE}" CACHE STRING "")
endif()

# ---------------------------------------------------------------
# dynamic MSVC runtime (/MD) — third-party libs are static via x64-windows-static-md,
# only the MSVC redistributable DLLs are required at runtime
# ---------------------------------------------------------------
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)

# sccache can't handle /Zi (shared PDB) with parallel builds; use /Z7 instead
# so debug info goes into each .obj rather than a shared PDB file.
if(SCCACHE)
  foreach(_lang C CXX)
    foreach(_cfg DEBUG RELWITHDEBINFO)
      string(REPLACE "/Zi" "/Z7"
        CMAKE_${_lang}_FLAGS_${_cfg} "${CMAKE_${_lang}_FLAGS_${_cfg}}")
    endforeach()
  endforeach()
endif()

project(MyProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------
# paths
# ---------------------------------------------------------------
set(EIGEN3_ROOT     "${CMAKE_SOURCE_DIR}/lib/eigen3")
set(PINOCCHIO_ROOT  "${CMAKE_SOURCE_DIR}/lib/pinocchio")

# ---------------------------------------------------------------
# boost (via vcpkg - x64-windows-static-md triplet: static libs, dynamic CRT)
# ---------------------------------------------------------------
find_package(Boost REQUIRED COMPONENTS filesystem serialization iostreams)

# urdfdom is always dynamic in vcpkg; find it here so its imported targets
# (urdfdom::urdfdom_model etc.) are available for DLL staging below.
find_package(urdfdom CONFIG REQUIRED)

# ---------------------------------------------------------------
# eigen - add_subdirectory generates Eigen3Config.cmake into the build tree
# we then tell cmake to look there via Eigen3_DIR
# ---------------------------------------------------------------
set(EIGEN_BUILD_CMAKE_PACKAGE ON CACHE BOOL "" FORCE)
add_subdirectory("${EIGEN3_ROOT}" "${CMAKE_BINARY_DIR}/eigen3-build" EXCLUDE_FROM_ALL)
set(Eigen3_DIR "${CMAKE_BINARY_DIR}/eigen3-build" CACHE PATH "" FORCE)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/eigen3-build")

# ---------------------------------------------------------------
# pinocchio via its own cmake
# ---------------------------------------------------------------

# Tell pinocchio's ADD_PROJECT_DEPENDENCY macro that Boost and Eigen3
# are already in the workspace so it skips find_package.
list(APPEND PROJECT_PACKAGES_IN_WORKSPACE Boost Eigen3)
# Provide the version variables that pinocchio inspects after finding Boost.
set(Boost_VERSION_STRING "${Boost_VERSION}")
set(Boost_FOUND TRUE)

set(BUILD_SHARED_LIBS         ON  CACHE BOOL "" FORCE)
set(BUILD_TESTING             OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARK           OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES            OFF CACHE BOOL "" FORCE)
set(BUILD_PYTHON_INTERFACE    OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_URDF_SUPPORT       ON  CACHE BOOL "" FORCE)
set(BUILD_WITH_COLLISION_SUPPORT  OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_CASADI_SUPPORT     OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_AUTODIFF_SUPPORT   OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_CODEGEN_SUPPORT    OFF CACHE BOOL "" FORCE)

add_compile_definitions(_WIN32_WINNT=0x0A00)

set(CMAKE_SKIP_INSTALL_RULES TRUE)
add_subdirectory("${PINOCCHIO_ROOT}" EXCLUDE_FROM_ALL)
set(CMAKE_SKIP_INSTALL_RULES FALSE)

# MSVC treats C4716 ("must return a value") as an error even when __assume(0)
# marks code as unreachable — suppress it for the pinocchio library only.
if(TARGET pinocchio_default)
  target_compile_options(pinocchio_default PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/wd4716>)
endif()

# ---------------------------------------------------------------
# your executable
# ---------------------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE pinocchio)
# /Zc:__cplusplus makes MSVC report the real __cplusplus value so pinocchio's
# PINOCCHIO_DEPRECATED_MESSAGE macro expands to [[deprecated(...)]] not __declspec.
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)

# ---------------------------------------------------------------
# DLL staging — copy runtime DLLs next to the executable.
# Boost and most vcpkg deps are static (x64-windows-static-md).
# Pinocchio is built shared (two DLLs from this build).
# urdfdom cannot be built static in vcpkg regardless of triplet,
# so its four DLLs must also be shipped.
# ---------------------------------------------------------------
set(PINOCCHIO_RUNTIME_DLLS
    # pinocchio — built as shared libs in this tree
    $<TARGET_FILE:pinocchio_default>
    $<TARGET_FILE:pinocchio_parsers>
    # urdfdom — always dynamic from vcpkg; target names handle debug/release paths
    $<TARGET_FILE:urdfdom::urdfdom_model>
    $<TARGET_FILE:urdfdom::urdfdom_model_state>
    $<TARGET_FILE:urdfdom::urdfdom_world>
    $<TARGET_FILE:urdfdom::urdfdom_sensor>
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            ${PINOCCHIO_RUNTIME_DLLS}
        COMMAND_EXPAND_LISTS
        COMMENT "Staging pinocchio DLLs next to executable"
    )
endif()
