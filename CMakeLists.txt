cmake_minimum_required(VERSION ${CMAKE_VERSION})
project(cpp_hashmap_tests)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)
include_directories(tests)

add_executable(basic_test
        hashmap.hpp
        tests/basic.cpp
)

add_executable(advanced_test
        hashmap.hpp
        tests/advanced.cpp
)

add_executable(extreme_test
        hashmap.hpp
        tests/extreme.cpp
)