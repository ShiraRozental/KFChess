cmake_minimum_required(VERSION 3.15)
project(KFChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# איסוף קבצי הלוגיקה מתוך src (כולל תת-תיקיות), חוץ מ-src/view -
# הקבצים שם תלויים ב-OpenCV ומיועדים רק ל-exe הראשי, לא לטסטים
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")
file(GLOB_RECURSE VIEW_SOURCES "src/view/*.cpp")
set(PURE_VIEW_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/view/GameScreenLayout.cpp")
list(REMOVE_ITEM VIEW_SOURCES ${PURE_VIEW_SOURCES})
set(CORE_SOURCES ${ALL_SOURCES})
list(REMOVE_ITEM CORE_SOURCES ${VIEW_SOURCES})

# --- OpenCV (מגיע ארוז בפרויקט תחת third_party/, בלי צורך בהתקנת מערכת) ---
set(OPENCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/OpenCV_451")
set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
set(OPENCV_LIB_DIR "${OPENCV_DIR}/bin")

# הבינאריים ב-third_party/ הם .lib בפורמט MSVC. הבדיקה כאן ולא ברמת הפרויקט
# כדי ש-KFChessTests, שלא נוגע ב-OpenCV, ימשיך להיבנות בכל מהדר.
function(add_opencv_executable target entry_point)
    if(NOT MSVC)
        message(FATAL_ERROR
            "Target '${target}' links the OpenCV binaries bundled in third_party/, "
            "which are MSVC-format .lib files and cannot be linked by ${CMAKE_CXX_COMPILER_ID}. "
            "Configure with the MSVC toolchain, e.g. "
            "cmake -S . -B build -G \"Visual Studio 17 2022\" -A x64, "
            "or pass -DBUILD_APP=OFF to build the tests alone.")
    endif()
    add_executable(${target} ${entry_point} ${CORE_SOURCES} ${VIEW_SOURCES})
    target_include_directories(${target} PRIVATE include third_party ${OPENCV_INCLUDE_DIR})
    target_link_directories(${target} PRIVATE ${OPENCV_LIB_DIR})
    target_link_libraries(${target} PRIVATE
        $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.lib>
        $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.lib>
        $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.lib>
        $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.lib>
    )
    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world451d.dll>
            $<$<CONFIG:Release>:${OPENCV_LIB_DIR}/opencv_world451.dll>
            $<$<CONFIG:RelWithDebInfo>:${OPENCV_LIB_DIR}/opencv_world451.dll>
            $<$<CONFIG:MinSizeRel>:${OPENCV_LIB_DIR}/opencv_world451.dll>
            $<TARGET_FILE_DIR:${target}>)
    endif()
endfunction()

# נקודת הכניסה של האפליקציה יושבת מחוץ ל-src/ (מקביל ל-app.py)
option(BUILD_APP "Build the OpenCV-backed executables (requires the MSVC toolchain)" ON)

if(BUILD_APP)
    add_opencv_executable(KFChess app/KFChessApp.cpp)
    add_opencv_executable(KFChessScript Main.cpp)
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
