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
    # doctest מגיע ארוז בפרויקט (tests/doctest) כדי שהבנייה תעבוד
    # גם בלי גישה לאינטרנט (למשל בעת הגשה במודל).
    add_library(doctest INTERFACE)
    target_include_directories(doctest INTERFACE tests)
    add_library(doctest::doctest ALIAS doctest)

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
