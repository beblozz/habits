cmake_minimum_required(VERSION 3.16)
project(habits CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(habits_lib
    src/date.cpp
    src/storage.cpp
    src/stats.cpp
    src/export.cpp
    src/commands.cpp
)
target_include_directories(habits_lib PUBLIC src)

add_executable(habits src/main.cpp)
target_link_libraries(habits PRIVATE habits_lib)

add_executable(run_tests tests/run_tests.cpp)
target_link_libraries(run_tests PRIVATE habits_lib)

enable_testing()
add_test(NAME habits_tests COMMAND run_tests)
