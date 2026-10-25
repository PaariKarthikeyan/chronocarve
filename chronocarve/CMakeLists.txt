cmake_minimum_required(VERSION 3.20)
project(ChronoCarve VERSION 1.0.0 LANGUAGES CXX C)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization Flags
if(MSVC)
    add_compile_options(/O2 /arch:AVX2 /W3 /utf-8 /EHsc)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-O3 -march=native -Wall -Wextra -pthread)
endif()

# Find OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV found: ${OpenCV_VERSION} at ${OpenCV_INCLUDE_DIRS}")

# Find OpenMP (optional for multi-threading acceleration)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found, enabling parallel acceleration")
endif()

# Dependencies: cpp-httplib & nlohmann/json
include(FetchContent)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/httplib.h")
    message(STATUS "Fetching cpp-httplib...")
    FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.15.3
    )
    FetchContent_MakeAvailable(httplib)
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json.hpp")
    message(STATUS "Fetching nlohmann/json...")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Boykov-Kolmogorov Max-Flow library
set(MAXFLOW_SOURCES
    third_party/maxflow/graph.cpp
    third_party/maxflow/maxflow.cpp
)

# ChronoCarve core image reframing sources
set(CHRONOCARVE_SOURCES
    src/main.cpp
    src/server.cpp
    src/energy.cpp
    src/image_io.cpp
    src/greedy_carver.cpp
    src/backward_dp_carver.cpp
    src/forward_dp_carver.cpp
    src/graph_cut_carver.cpp
    src/benchmark.cpp
    ${MAXFLOW_SOURCES}
)

add_executable(chronocarve ${CHRONOCARVE_SOURCES})

target_include_directories(chronocarve PRIVATE
    include
    third_party
    third_party/maxflow
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(chronocarve PRIVATE
    ${OpenCV_LIBS}
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(chronocarve PRIVATE OpenMP::OpenMP_CXX)
endif()

if(WIN32)
    target_link_libraries(chronocarve PRIVATE ws2_32 psapi)
endif()

# Copy web dashboard directory to build output if different
if(NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    add_custom_command(TARGET chronocarve POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/web
        $<TARGET_FILE_DIR:chronocarve>/web
    )
endif()
