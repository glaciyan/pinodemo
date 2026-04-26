cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)

# Use the dynamic MSVC runtime (/MD, /MDd). This must match the vcpkg triplet
# (x64-windows-static-md): that triplet builds all third-party libs with /MD so
# they share one CRT heap. Mixing /MT and /MD across DLL boundaries causes
# crashes when memory allocated in one module is freed in another.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)

project(MyProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS filesystem serialization iostreams)
# urdfdom is always dynamic in vcpkg; find here so urdfdom:: targets exist for DLL staging
find_package(urdfdom CONFIG REQUIRED)

# eigen — add_subdirectory generates Eigen3Config.cmake into the build tree
set(EIGEN_BUILD_CMAKE_PACKAGE ON CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/eigen3" "${CMAKE_BINARY_DIR}/eigen3-build" EXCLUDE_FROM_ALL)
set(Eigen3_DIR "${CMAKE_BINARY_DIR}/eigen3-build" CACHE PATH "" FORCE)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/eigen3-build")

# tell pinocchio's ADD_PROJECT_DEPENDENCY that Boost and Eigen3 are already found
list(APPEND PROJECT_PACKAGES_IN_WORKSPACE Boost Eigen3)
set(Boost_VERSION_STRING "${Boost_VERSION}")
set(Boost_FOUND TRUE)

set(BUILD_SHARED_LIBS      ON  CACHE BOOL "" FORCE) # built as DLLs; avoids MSVC static export hacks
set(BUILD_TESTING          OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES         OFF CACHE BOOL "" FORCE)
set(BUILD_PYTHON_INTERFACE OFF CACHE BOOL "" FORCE)
add_compile_definitions(_WIN32_WINNT=0x0A00)

set(CMAKE_SKIP_INSTALL_RULES TRUE)
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/pinocchio" EXCLUDE_FROM_ALL)
set(CMAKE_SKIP_INSTALL_RULES FALSE)

# suppress C4716 ("not all paths return a value") in pinocchio's unreachable __assume(0) paths
target_compile_options(pinocchio_default PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4716>)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE pinocchio)
# /Zc:__cplusplus — MSVC otherwise lies about __cplusplus, breaking pinocchio's deprecation macros
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>)

# copy runtime DLLs next to the executable (Boost etc. are static; these six must ship)
if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
      $<TARGET_FILE:pinocchio_default>
      $<TARGET_FILE:pinocchio_parsers>
      $<TARGET_FILE:urdfdom::urdfdom_model>
      $<TARGET_FILE:urdfdom::urdfdom_model_state>
      $<TARGET_FILE:urdfdom::urdfdom_world>
      $<TARGET_FILE:urdfdom::urdfdom_sensor>
    COMMAND_EXPAND_LISTS
  )
endif()
