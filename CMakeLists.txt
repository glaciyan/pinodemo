cmake_minimum_required(VERSION 4.1)
project(pinodemo)

set(CMAKE_CXX_STANDARD 17)

set(BOOST_ROOT "${PROJECT_SOURCE_DIR}/lib/boost_1_91_0")
find_package(Boost CONFIG COMPONENTS filesystem)

add_executable(pinodemo main.cpp)
target_include_directories(pinodemo PRIVATE ${Boost_INCLUDE_DIR} )
target_link_libraries(pinodemo PRIVATE Boost::filesystem)
