cmake_minimum_required(VERSION 3.15)
project(KFChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# איסוף קבצי הלוגיקה מתוך src (כולל תת-תיקיות), חוץ מ-src/view -
# הקבצים שם תלויים ב-OpenCV ומיועדים רק ל-exe הראשי, לא לטסטים
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
file(GLOB_RECURSE VIEW_SOURCES "src/view/*.cpp")
set(CORE_SOURCES ${ALL_SOURCES})
list(REMOVE_ITEM CORE_SOURCES ${VIEW_SOURCES})

# נקודת הכניסה של האפליקציה יושבת מחוץ ל-src/ (מקביל ל-app.py)
add_executable(KFChess Main.cpp ${CORE_SOURCES} ${VIEW_SOURCES})

target_include_directories(KFChess PRIVATE include third_party)

# --- OpenCV (מגיע ארוז בפרויקט תחת third_party/, בלי צורך בהתקנת מערכת) ---
set(OPENCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenCV_451")
set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
set(OPENCV_LIB_DIR "${OPENCV_DIR}/bin")

target_include_directories(KFChess PRIVATE ${OPENCV_INCLUDE_DIR})
target_link_directories(KFChess PRIVATE ${OPENCV_LIB_DIR})
target_link_libraries(KFChess PRIVATE
    $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.lib>
    $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.lib>
)

if(WIN32)
    add_custom_command(TARGET KFChess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.dll>
        $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.dll>
        $<TARGET_FILE_DIR:KFChess>)
endif()

# --- Unit tests ---
option(BUILD_TESTS "Build the doctest-based unit test suite" ON)
option(ENABLE_COVERAGE "Instrument the test build for gcov/gcovr coverage" OFF)

if(BUILD_TESTS)
    # doctest מגיע ארוז בפרויקט (tests/doctest) כדי שהבנייה תעבוד
    # גם בלי גישה לאינטרנט (למשל בעת הגשה במודל).
    add_library(doctest INTERFACE)
    target_include_directories(doctest INTERFACE tests)
    add_library(doctest::doctest ALIAS doctest)

    # אותם קבצי לוגיקה כמו KFChess (Main.cpp לא נכלל: יש לו main משלו,
    # ובלאו הכי הוא לא חלק מ-CORE_SOURCES כי הוא יושב מחוץ ל-src/).
    # CORE_SOURCES בלבד, לא VIEW_SOURCES - כדי שהטסטים לעולם לא ידרשו
    # OpenCV; ה-Renderer המופשט כבר מבודד אותם מכל מימוש view קונקרטי.
    file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

    add_executable(KFChessTests ${TEST_SOURCES} ${CORE_SOURCES})
    target_include_directories(KFChessTests PRIVATE include third_party)
    target_link_libraries(KFChessTests PRIVATE doctest::doctest)

    # נתיב מוחלט לקבצי ה-fixtures, כדי שהטסטים ימצאו אותם בלי תלות בתיקיית
    # העבודה שממנה מריצים את KFChessTests (build/ מול build_tests/ וכו').
    target_compile_definitions(KFChessTests PRIVATE
        TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")

    if(ENABLE_COVERAGE)
        target_compile_options(KFChessTests PRIVATE --coverage -O0 -g)
        target_link_options(KFChessTests PRIVATE --coverage)
    endif()

    enable_testing()
    add_test(NAME KFChessTests COMMAND KFChessTests)
endif()
