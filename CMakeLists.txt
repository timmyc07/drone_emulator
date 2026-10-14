cmake_minimum_required(VERSION 3.20)
project(UniversalUAVSimulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_executable(uavsim_info src/uavsim_info.cpp)
target_include_directories(uavsim_info PRIVATE include)

add_executable(uavsim_smoke tests/smoke_test.cpp)
target_include_directories(uavsim_smoke PRIVATE include)
add_test(NAME uavsim_smoke COMMAND uavsim_smoke)
