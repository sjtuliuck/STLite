cmake_minimum_required(VERSION 3.14)
project(deque)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(data)

add_executable(deque
#        data/five/code.cpp
#        data/four/code.cpp
#        data/four.memcheck/code.cpp
#        data/one/code.cpp
#        data/one.memcheck/code.cpp
        data/six/code.cpp
#        data/three/code.cpp
#        data/three.memcheck/code.cpp
#        data/two/code.cpp
#        data/two.memcheck/code.cpp
        data/class-bint.hpp
        data/class-integer.hpp
        data/class-matrix.hpp
        deque.hpp
        exceptions.hpp
        utility.hpp
        )
