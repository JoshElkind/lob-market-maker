cmake_minimum_required(VERSION 3.16)
project(lob_market_maker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lob
  src/book.cpp
)
target_include_directories(lob PUBLIC include)

add_executable(lob_tests tests/test_book.cpp)
target_link_libraries(lob_tests PRIVATE lob)
