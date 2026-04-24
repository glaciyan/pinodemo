cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0144 NEW)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------
# paths
# ---------------------------------------------------------------
set(EIGEN3_ROOT     "${CMAKE_SOURCE_DIR}/lib/eigen3")
set(PINOCCHIO_ROOT  "${CMAKE_SOURCE_DIR}/lib/pinocchio")

# ---------------------------------------------------------------
# boost
# ---------------------------------------------------------------
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES filesystem serialization)

add_subdirectory("${CMAKE_SOURCE_DIR}/lib/boost-1.91.0-1" EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------
# eigen - add_subdirectory generates Eigen3Config.cmake into the build tree
# we then tell cmake to look there via CMAKE_PREFIX_PATH
# ---------------------------------------------------------------
add_subdirectory("${EIGEN3_ROOT}" "${CMAKE_BINARY_DIR}/eigen3-build" EXCLUDE_FROM_ALL)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/eigen3-build")

# ---------------------------------------------------------------
# pinocchio via its own cmake
# ---------------------------------------------------------------
set(BUILD_TESTING             OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARK           OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES            OFF CACHE BOOL "" FORCE)
set(BUILD_PYTHON_INTERFACE    OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_COLLISION_SUPPORT  OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_CASADI_SUPPORT     OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_AUTODIFF_SUPPORT   OFF CACHE BOOL "" FORCE)
set(BUILD_WITH_CODEGEN_SUPPORT    OFF CACHE BOOL "" FORCE)

add_subdirectory("${PINOCCHIO_ROOT}" EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------
# your executable
# ---------------------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE pinocchio)