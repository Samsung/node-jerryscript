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

add_subdirectory(tests)

if(USE_V8)

# It is possible that not all of these are required, but for now it's ok
set(V8_LIBS
    ${USE_V8_LIB_DIR}/libv8_base.a
    ${USE_V8_LIB_DIR}/libv8_libsampler.a
    ${USE_V8_LIB_DIR}/libv8_builtins_setup.a
    ${USE_V8_LIB_DIR}/libv8_libplatform.a
    ${USE_V8_LIB_DIR}/libv8_nosnapshot.a
    ${USE_V8_LIB_DIR}/libv8_builtins_generators.a
    ${USE_V8_LIB_DIR}/libv8_libbase.a
)

add_executable(v8_demo test_hello.cpp)
target_include_directories(v8_demo PUBLIC ${USE_V8_HEADER_DIR})
target_link_libraries(v8_demo ${V8_LIBS} pthread)

endif()
