cmake_minimum_required(VERSION 3.10)
project(v8jerry C CXX)

add_library(v8jerry
    v8jerry.cpp

    v8jerry_callback.cpp
    v8jerry_handlescope.cpp
    v8jerry_templates.cpp
    v8jerry_isolate.cpp
    v8jerry_value.cpp
    v8jerry_value.hpp
    v8jerry_utils.cpp
    v8jerry_utils.hpp
)
target_link_libraries(v8jerry PRIVATE jerry)
target_link_libraries(v8jerry PUBLIC v8headers)
