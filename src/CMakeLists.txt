cmake_minimum_required(VERSION 3.10)
project(v8jerry C CXX)

add_library(v8jerry
    v8jerry.cpp
)
target_link_libraries(v8jerry PRIVATE jerry)
target_link_libraries(v8jerry PUBLIC v8headers)
