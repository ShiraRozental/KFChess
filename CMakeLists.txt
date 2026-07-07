cmake_minimum_required(VERSION 3.15)
project(KFChess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# איסוף כל קבצי ה-cpp מתוך src (כולל תת-תיקיות)
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(KFChess ${SOURCES})

target_include_directories(KFChess PRIVATE include)