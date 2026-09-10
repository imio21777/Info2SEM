cmake_minimum_required(VERSION 3.10)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(lab
    LANGUAGES CXX C
    DESCRIPTION "Data structure course lab."
)

# -------------------------- miniSTL --------------------------

set(miniSTL_SOURCE_DIR miniSTL)
add_subdirectory(${miniSTL_SOURCE_DIR})

# -------------------------- Catch2 --------------------------

set(catch2_SOURCE_DIR Catch2)
add_subdirectory(${catch2_SOURCE_DIR})
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# -------------------------- Test --------------------------

file(GLOB test_sources test/*.cpp)

add_executable(TestSTL ${test_sources})
target_link_libraries(TestSTL PRIVATE miniSTL::miniSTL Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(TestSTL)
