cmake_minimum_required(VERSION 3.10)
project(v8jerry C CXX)

#set(CMAKE_BUILD_TYPE Debug)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG")
endif()

add_subdirectory(deps)
add_subdirectory(src)

if((NOT DEFINED USE_V8) AND (DEFINED ENV{USE_V8}))
    set(USE_V8 $ENV{USE_V8})
endif()

if((NOT DEFINED USE_V8_LIB_DIR) AND (DEFINED ENV{USE_V8_LIB_DIR}))
    set(USE_V8_LIB_DIR $ENV{USE_V8_LIB_DIR})
endif()

if((NOT DEFINED USE_V8_HEADER_DIR) AND (DEFINED ENV{USE_V8_HEADER_DIR}))
    set(USE_V8_HEADER_DIR $ENV{USE_V8_HEADER_DIR})
endif()

message("-- Configured USE_V8: ${USE_V8}")
message("-- Configured USE_V8_LIB_DIR: ${USE_V8_LIB_DIR}")
message("-- Configured USE_V8_HEADER_DIR: ${USE_V8_HEADER_DIR}")

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
        pthread
    )

endif()

add_subdirectory(tests)
