# Build definition for the IBAN Calculator project.

# 3.31 is required for CMakePresets.json schema version 10 ($comment support).
cmake_minimum_required(VERSION 3.31)

project(IbanCalculator VERSION 0.1.0 LANGUAGES CXX)

# Use C++20 without compiler-specific language extensions.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # fail instead of silently using an older standard
set(CMAKE_CXX_EXTENSIONS OFF)        # standard C++ only, no compiler extensions (gnu++20)

# Build unit tests by default. Can be disabled with -DIBAN_BUILD_TESTS=OFF.
option(IBAN_BUILD_TESTS "Build the test suites" ON)

# Portable thread library (adds -pthread where the platform needs it).
find_package(Threads REQUIRED)

# Apply consistent compiler warnings to project targets.
# Third-party dependencies are configured separately.
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
# Version
# ---------------------------------------------------------------------------
# The version in project() is the single source of truth: CMake writes it into
# a generated header, so the code never repeats the number.
# LEARN: configure_file copies the template and replaces @PROJECT_VERSION@;
# LEARN: @ONLY leaves ${...} in the file untouched.
configure_file(cmake/Version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/iban/Version.h @ONLY)

# ---------------------------------------------------------------------------
# Core library
# ---------------------------------------------------------------------------
# All logic except main() lives in one library, linked by both the program and
# the tests, so the tests exercise exactly the code that ships.
# LEARN: STATIC: the code is copied into each program that links it (a .a file).
add_library(iban_core STATIC
        src/domain/MCheck.cpp
)
# Only include/ is public: code that links iban_core can include these headers.
# LEARN: PUBLIC: iban_core itself AND everything that links it get this path.
target_include_directories(iban_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
iban_set_warnings(iban_core)

# ---------------------------------------------------------------------------
# Console application
# ---------------------------------------------------------------------------
# LEARN: add_executable(name sources...) builds a program from these files.
add_executable(iban app/main.cpp)
# Generated headers live in the build folder, not in the source tree.
target_include_directories(iban PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(iban PRIVATE iban_core Threads::Threads)
iban_set_warnings(iban)

# ---------------------------------------------------------------------------
# Code formatting
# ---------------------------------------------------------------------------
# `format` rewrites all sources, `format-check` only reports deviations (CI).
# Each clang-format release formats slightly differently, so one version is
# pinned for developers and CI; a different version makes format-check fail
# with a clear message instead of reporting false deviations.
set(IBAN_CLANG_FORMAT_VERSION "21.1.8")
find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

if(CLANG_FORMAT_EXECUTABLE)
    execute_process(COMMAND ${CLANG_FORMAT_EXECUTABLE} --version
            OUTPUT_VARIABLE clang_format_output OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" clang_format_found "${clang_format_output}")

    # A glob is acceptable here: it only feeds a tool, not the build itself.
    # CONFIGURE_DEPENDS re-runs the glob when files are added or removed.
    file(GLOB_RECURSE IBAN_FORMAT_FILES CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h)

    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${IBAN_FORMAT_FILES}
            COMMENT "Formatting sources with clang-format ${clang_format_found}"
            VERBATIM)

    if(clang_format_found VERSION_EQUAL IBAN_CLANG_FORMAT_VERSION)
        add_custom_target(format-check
                COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror ${IBAN_FORMAT_FILES}
                COMMENT "Checking formatting with clang-format ${clang_format_found}"
                VERBATIM)
    else()
        add_custom_target(format-check
                COMMAND ${CMAKE_COMMAND} -E echo
                "format-check needs clang-format ${IBAN_CLANG_FORMAT_VERSION}, found ${clang_format_found}."
                "Install it with: pipx install clang-format==${IBAN_CLANG_FORMAT_VERSION}"
                COMMAND ${CMAKE_COMMAND} -E false
                VERBATIM)
    endif()
endif()

# ---------------------------------------------------------------------------
# Unit tests
# ---------------------------------------------------------------------------
if(IBAN_BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    # Fetch a fixed GoogleTest version to ensure reproducible test builds.
    # SYSTEM: warnings in GoogleTest headers are not reported in our tests.
    FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
            GIT_SHALLOW TRUE
            SYSTEM)
    # Use the same C++ runtime library for GoogleTest and the project on MSVC.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # Do not generate installation targets for GoogleTest.
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)

    # Suppress warnings while building GoogleTest itself; SYSTEM only covers its
    # headers in our code. Apple clang 21 reports -Wcharacter-conversion in
    # gtest-printers.h.
    foreach(gtest_target gtest gtest_main gmock gmock_main)
        if(TARGET ${gtest_target})
            if(MSVC)
                target_compile_options(${gtest_target} PRIVATE /w)
            else()
                target_compile_options(${gtest_target} PRIVATE -w)
            endif()
        endif()
    endforeach()

    # LEARN: gtest_main provides a main() that runs every TEST(...) in the files.
    add_executable(iban_unit_tests
            tests/unit/SetupTest.cpp
            tests/unit/MCheckTest.cpp
    )
    # Links the same iban_core as the program, so the tests check the shipped code.
    target_link_libraries(iban_unit_tests PRIVATE iban_core GTest::gtest_main Threads::Threads)
    iban_set_warnings(iban_unit_tests)

    # Register individual GoogleTest cases with CTest.
    gtest_discover_tests(iban_unit_tests PROPERTIES LABELS unit)
endif()
