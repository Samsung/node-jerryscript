cmake_minimum_required(VERSION 3.10)
project(v8jerry C CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG")

add_library(jerry STATIC
    deps/jerry/jerryscript.c
    deps/jerry/jerryscript-config.h
    deps/jerry/jerryscript.h
    deps/jerry/jerryscript-libm.c
    deps/jerry/jerryscript-port-default.c
    deps/jerry/jerryscript-port-default.h
)

add_library(v8jerry
    v8jerry.cpp
)
target_include_directories(v8jerry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/jerry)
target_include_directories(v8jerry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/v8-headers)
target_link_libraries(v8jerry jerry)


add_executable(demo test_hello.cpp)
target_link_libraries(demo v8jerry)
