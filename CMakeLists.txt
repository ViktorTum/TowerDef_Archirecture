cmake_minimum_required(VERSION 3.20)
project(TowerDef VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TD_BUILD_TESTS "Build unit tests" ON)
option(TD_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(TdModule)

if(TD_BUILD_TESTS)
    enable_testing()
    include(TdDependencies)
endif()

add_subdirectory(modules)
