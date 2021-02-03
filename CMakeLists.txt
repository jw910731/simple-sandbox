project(simple-sandbox-root)
cmake_minimum_required( VERSION 3.19 )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")

add_subdirectory(src)
