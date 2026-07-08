cmake_minimum_required(VERSION 3.15)
project(KFChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# איסוף כל קבצי ה-cpp מתוך src (כולל תת-תיקיות)
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(KFChess ${SOURCES})

target_include_directories(KFChess PRIVATE include)

# --- Unit tests ---
option(BUILD_TESTS "Build the doctest-based unit test suite" ON)
option(ENABLE_COVERAGE "Instrument the test build for gcov/gcovr coverage" OFF)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
    )
    # doctest's own CMakeLists.txt declares a cmake_minimum_required older
    # than modern CMake still supports; this raises the floor just for it.
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    FetchContent_MakeAvailable(doctest)
    unset(CMAKE_POLICY_VERSION_MINIMUM)

    # אותם קבצי לוגיקה כמו KFChess, חוץ מ-main.cpp (יש לו main משלו)
    set(GAME_SOURCES ${SOURCES})
    list(REMOVE_ITEM GAME_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

    file(GLOB TEST_SOURCES "tests/*.cpp")

    add_executable(KFChessTests ${TEST_SOURCES} ${GAME_SOURCES})
    target_include_directories(KFChessTests PRIVATE include)
    target_link_libraries(KFChessTests PRIVATE doctest::doctest)

    if(ENABLE_COVERAGE)
        target_compile_options(KFChessTests PRIVATE --coverage -O0 -g)
        target_link_options(KFChessTests PRIVATE --coverage)
    endif()

    enable_testing()
    add_test(NAME KFChessTests COMMAND KFChessTests)
endif()
