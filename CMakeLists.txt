# Build definition for the IBAN Calculator project.
# Targets: the program `iban` and the unit test executable `iban_unit_tests`.
# CMake does not compile; it writes a build plan for Ninja, and Ninja calls
# clang++ (macOS), g++ (Linux) or cl.exe (Windows).

# 3.31 is required for CMakePresets.json schema version 10 ($comment support).
cmake_minimum_required(VERSION 3.31)

project(IbanCalculator VERSION 0.1.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Language standard
# ---------------------------------------------------------------------------
# C++20 (std::atomic_flag::test, std::span, defaulted comparisons). Must be set
# before any target is created, because targets copy these defaults.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # fail instead of silently using an older standard
set(CMAKE_CXX_EXTENSIONS OFF)        # standard C++ only, no compiler extensions (gnu++20)

# ---------------------------------------------------------------------------
# Options and dependencies
# ---------------------------------------------------------------------------
option(IBAN_BUILD_TESTS "Build the test suites" ON)

# Threads::Threads links the correct threading library for each platform.
find_package(Threads REQUIRED)

# Strict warnings for project code only (rubric: code quality).
function(iban_set_warnings target)
    if(MSVC)
        # /Zc:__cplusplus makes MSVC report the real language version.
        target_compile_options(${target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    else()
        target_compile_options(${target} PRIVATE
                -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Core library
# ---------------------------------------------------------------------------
# All project code except main() lives in one static library, shared by the
# program and the tests, so the tests exercise exactly the code that ships.
add_library(iban_core STATIC
        src/domain/MCheck.cpp
)
# PUBLIC: everything that links iban_core can #include "iban/...".
target_include_directories(iban_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(iban_core PUBLIC Threads::Threads)
iban_set_warnings(iban_core)

# ---------------------------------------------------------------------------
# Program
# ---------------------------------------------------------------------------
add_executable(iban app/main.cpp)
target_link_libraries(iban PRIVATE iban_core)
iban_set_warnings(iban)

# ---------------------------------------------------------------------------
# Tests (GoogleTest is downloaded at a pinned version on first configure)
# ---------------------------------------------------------------------------
if(IBAN_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    # SYSTEM: GoogleTest headers are treated as system headers, so warnings
    # inside them do not show up when our tests include them.
    FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
            GIT_SHALLOW TRUE
            SYSTEM)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # required for MSVC
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)

    # Third-party code is not ours to fix: silence warnings while compiling
    # GoogleTest itself, so the build output only shows warnings in our code.
    foreach(gtest_target gtest gtest_main gmock gmock_main)
        if(TARGET ${gtest_target})
            if(MSVC)
                target_compile_options(${gtest_target} PRIVATE /w)
            else()
                target_compile_options(${gtest_target} PRIVATE -w)
            endif()
        endif()
    endforeach()

    add_executable(iban_unit_tests
            tests/unit/SetupTest.cpp
            tests/unit/MCheckTest.cpp
    )
    target_link_libraries(iban_unit_tests PRIVATE iban_core GTest::gtest_main)
    iban_set_warnings(iban_unit_tests)

    # Registers every test case with CTest so CI and the IDE can run and
    # report them individually.
    gtest_discover_tests(iban_unit_tests PROPERTIES LABELS unit)
endif()