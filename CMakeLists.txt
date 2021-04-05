cmake_minimum_required(VERSION 3.17)

project(UltimateTicTacToe)

set(CMAKE_CXX_STANDARD 11)

include_directories(PRIVATE src)

add_executable(main_minmax src/main_minmax.cpp)
add_executable(main_random src/main_random.cpp)
add_executable(main_mcts src/main_mcts.cpp)
