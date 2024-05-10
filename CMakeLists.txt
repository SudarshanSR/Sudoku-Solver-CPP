cmake_minimum_required(VERSION 3.27)
project(sudoku)

set(CMAKE_CXX_STANDARD 23)

add_library(solver src/solver.hpp src/solver.cpp)

add_executable(sudoku src/main.cpp)
target_link_libraries(sudoku solver)
