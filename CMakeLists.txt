cmake_minimum_required(VERSION 3.16)
project(lob_market_maker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lob
  src/book.cpp
  src/sim.cpp
)
target_include_directories(lob PUBLIC include)
target_compile_options(lob PRIVATE -Wall -Wextra -Wpedantic)

add_executable(lob_sim src/main.cpp)
target_link_libraries(lob_sim PRIVATE lob)
target_compile_options(lob_sim PRIVATE -Wall -Wextra -Wpedantic)

add_executable(lob_tests tests/test_book.cpp)
target_link_libraries(lob_tests PRIVATE lob)
target_compile_options(lob_tests PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME book COMMAND lob_tests)
