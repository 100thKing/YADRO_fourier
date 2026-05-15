cmake_minimum_required(VERSION 3.14)
project(FFT_Project LANGUAGES CXX)

# ── C++ standard ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build type (defaults to Release if not specified) ─────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_Debug   "-O0 -g")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ── Target 1: fft (CSV generator) ────────────────────────────
add_executable(fft fft.cpp)

target_compile_options(fft PRIVATE
    -Wall -Wextra -Wpedantic
)

# ── Target 2: error_rate (CSV error analyser) ─────────────────
add_executable(error_rate error_rate.cpp)

target_compile_options(error_rate PRIVATE
    -Wall -Wextra -Wpedantic
)

# ── Print binary output directory after configuration ─────────
message(STATUS "Executables will be placed in: ${CMAKE_BINARY_DIR}")

# ── Custom target: run fft then error_rate in one step ────────
# Usage: cmake --build build --target run
add_custom_target(run
    COMMENT "=== Running fft (CSV generation) ==="
    COMMAND fft
    COMMENT "=== Running error_rate on all CSV files ==="
    COMMAND ${CMAKE_COMMAND} -E echo "--- error_rate ---"
    COMMAND error_rate
        fft_8.csv fft_16.csv fft_12.csv fft_18.csv fft_20.csv
        fft_30.csv fft_60.csv fft_120.csv fft_360.csv fft_1800.csv
    DEPENDS fft error_rate
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
