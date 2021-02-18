project(simple-sandbox)

set(SOURCE_FILES main.cpp sandbox.cpp)
add_definitions(-D_GNU_SOURCE)

add_executable(simple-sandbox ${SOURCE_FILES})