# Build definition for the IBAN Calculator project.
# The assignment does not require a build system; CMake is used for portable
# builds on three platforms, Debug/Release presets, tests, and formatting.
# The full rationale is in README.md, section "Build system: why CMake".

# 3.31 is required for CMakePresets.json schema version 10 ($comment support).
cmake_minimum_required(VERSION 3.31)

project(IbanCalculator VERSION 0.1.0 LANGUAGES CXX)

# Use C++20 without compiler-specific language extensions.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # fail instead of silently using an older standard
set(CMAKE_CXX_EXTENSIONS OFF)        # standard C++ only, no compiler extensions (gnu++20)

# Build unit tests by default. Can be disabled with -DIBAN_BUILD_TESTS=OFF.
option(IBAN_BUILD_TESTS "Build the test suites" ON)

# Off by default: CI and normal builds do not need Google Benchmark. Enable it
# for measurements with: cmake --preset release -DIBAN_BUILD_BENCHMARKS=ON
option(IBAN_BUILD_BENCHMARKS "Build the benchmark program" OFF)

# Off by default: the end-to-end tests need Java 25 and Maven. Enable them with
# -DIBAN_BUILD_E2E_TESTS=ON and run them with: ctest --preset debug -L e2e
option(IBAN_BUILD_E2E_TESTS "Register the Cucumber end-to-end tests with CTest" OFF)

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
        src/application/CountVariants.cpp
        src/application/ListVariants.cpp
        src/application/ParallelCount.cpp
        src/application/ParallelList.cpp
        src/domain/Count.cpp
        src/domain/MCheck.cpp
        src/domain/RangeSplit.cpp
        src/presentation/ConsoleInput.cpp
        src/presentation/InputParser.cpp
)
# Only include/ is public: code that links iban_core can include these headers.
# LEARN: PUBLIC: iban_core itself AND everything that links it get this path.
target_include_directories(iban_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# The library starts threads (parallel count). PUBLIC: every target that links
# iban_core also gets the thread library, so it does not have to repeat it.
target_link_libraries(iban_core PUBLIC Threads::Threads)
iban_set_warnings(iban_core)

# ---------------------------------------------------------------------------
# Console application
# ---------------------------------------------------------------------------
# LEARN: add_executable(name sources...) builds a program from these files.
add_executable(iban app/main.cpp)
# Generated headers live in the build folder, not in the source tree.
target_include_directories(iban PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(iban PRIVATE iban_core)
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
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp
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
            tests/unit/InputParserTest.cpp
            tests/unit/ConsoleInputTest.cpp
            tests/unit/RangeSplitTest.cpp
            tests/unit/CountTest.cpp
            tests/unit/ParallelCountTest.cpp
            tests/unit/SharedCounterTest.cpp
            tests/unit/CountVariantsTest.cpp
            tests/unit/ForEachValidTest.cpp
            tests/unit/BoundedBufferTest.cpp
            tests/unit/SequencedWriterTest.cpp
            tests/unit/ListVariantsTest.cpp
    )
    # Links the same iban_core as the program, so the tests check the shipped code.
    target_link_libraries(iban_unit_tests PRIVATE iban_core GTest::gtest_main Threads::Threads)
    iban_set_warnings(iban_unit_tests)

    # Register individual GoogleTest cases with CTest.
    gtest_discover_tests(iban_unit_tests PROPERTIES LABELS unit)

    # Demonstration of the data race of a shared counter without a lock. A
    # program, not a test: a race is not reproducible, so it cannot pass or fail
    # reliably. Run it in the debug or tsan build (see tests/race/RaceDemo.cpp).
    add_executable(iban_race_demo tests/race/RaceDemo.cpp)
    target_link_libraries(iban_race_demo PRIVATE iban_core)
    iban_set_warnings(iban_race_demo)

    # Demonstration of a race condition without a data race in list mode: the
    # sequence number and the line in two separate critical sections.
    add_executable(iban_list_order_demo tests/race/ListOrderDemo.cpp)
    target_link_libraries(iban_list_order_demo PRIVATE iban_core)
    iban_set_warnings(iban_list_order_demo)
endif()

# ---------------------------------------------------------------------------
# End-to-end tests
# ---------------------------------------------------------------------------
# Cucumber scenarios (tests/e2e) start the compiled program as a separate
# process and check its output and exit code. Maven runs them; CTest only
# starts Maven, so `ctest -L e2e` runs them like every other test.
if(IBAN_BUILD_E2E_TESTS)
    enable_testing()
    # mvn.cmd is the name of the Maven launcher on Windows.
    find_program(MAVEN_EXECUTABLE NAMES mvn mvn.cmd REQUIRED)
    # $<TARGET_FILE:iban> is the full path of the built program, including
    # .exe on Windows, so the scenarios test exactly this build.
    add_test(
            NAME e2e.cucumber
            COMMAND ${MAVEN_EXECUTABLE} --batch-mode
            --file ${CMAKE_CURRENT_SOURCE_DIR}/tests/e2e/pom.xml
            test -Diban.executable=$<TARGET_FILE:iban>)
    # The first run downloads the Cucumber and JUnit libraries.
    set_tests_properties(e2e.cucumber PROPERTIES LABELS e2e TIMEOUT 600)
endif()

# ---------------------------------------------------------------------------
# Benchmark
# ---------------------------------------------------------------------------
# Measures the count mode with 1 to 12, 16 and 32 threads. A separate program,
# not a test: it takes seconds and its output is a time, not pass or fail.
if(IBAN_BUILD_BENCHMARKS)
    include(FetchContent)
    # Pinned version, fetched like GoogleTest. SYSTEM: its headers do not
    # trigger our strict warnings.
    FetchContent_Declare(googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.10.0
            GIT_SHALLOW TRUE
            SYSTEM)
    # Only the library is needed: no tests, no install rules, and no -Werror
    # on its own sources.
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googlebenchmark)

    add_executable(iban_benchmark benchmark/CountBenchmark.cpp)
    # benchmark::benchmark_main is not used: the file defines BENCHMARK_MAIN.
    target_link_libraries(iban_benchmark PRIVATE iban_core benchmark::benchmark)
    iban_set_warnings(iban_benchmark)

    # Compares the ways of combining the thread counts (shared counter, locks,
    # atomic, result slots). A separate program, so the speedup measurement and
    # its CI summary stay unchanged.
    add_executable(iban_benchmark_variants benchmark/CountVariantsBenchmark.cpp)
    target_link_libraries(iban_benchmark_variants PRIVATE iban_core benchmark::benchmark)
    iban_set_warnings(iban_benchmark_variants)
endif()
