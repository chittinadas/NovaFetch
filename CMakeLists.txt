
cmake_minimum_required(VERSION 3.24)

project(NovaFetch
    VERSION 0.2.0
    DESCRIPTION "Native Windows Download Manager"
    LANGUAGES CXX RC
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

option(NOVAFETCH_BUILD_TESTS "Build tests" OFF)

add_subdirectory(src)
add_subdirectory(app)

if(NOVAFETCH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
