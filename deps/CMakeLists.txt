# Export "jerry" and "v8headers" libraries

add_library(jerry STATIC
    ${CMAKE_SOURCE_DIR}/deps/jerry/jerryscript.c
    ${CMAKE_SOURCE_DIR}/deps/jerry/jerryscript-config.h
    ${CMAKE_SOURCE_DIR}/deps/jerry/jerryscript.h
    ${CMAKE_SOURCE_DIR}/deps/jerry/jerryscript-libm.c
    ${CMAKE_SOURCE_DIR}/deps/jerry/jerryscript-port-default.c
    ${CMAKE_SOURCE_DIR}/deps/jerry/jerryscript-port-default.h
)
target_include_directories(jerry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/jerry)

add_library(v8headers INTERFACE)
target_include_directories(v8headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/v8-headers)
