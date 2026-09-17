cmake_minimum_required(VERSION 3.16)
project(chess_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(chess_engine_lib
    src/chess_engine.cpp
)
target_include_directories(chess_engine_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(chess_engine main.cpp)
target_link_libraries(chess_engine PRIVATE chess_engine_lib)

add_executable(chess_engine_tests tests/test_chess.cpp)
target_link_libraries(chess_engine_tests PRIVATE chess_engine_lib)
