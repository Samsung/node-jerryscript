# Export "jerry" and "v8headers" libraries

set(JERRY_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/)

set(JERRY_SOURCES
    ${JERRY_GEN_DIR}/jerryscript.c
    ${JERRY_GEN_DIR}/jerryscript-config.h
    ${JERRY_GEN_DIR}/jerryscript.h
    ${JERRY_GEN_DIR}/jerryscript-port-default.c
    ${JERRY_GEN_DIR}/jerryscript-port-default.h
)

add_custom_command(OUTPUT ${JERRY_SOURCES}
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/jerryscript/tools/srcgenerator.py
            --output-dir=${JERRY_GEN_DIR}
            --jerry-core
            --jerry-port-default
)

add_library(jerry STATIC ${JERRY_SOURCES})
target_include_directories(jerry PUBLIC ${JERRY_GEN_DIR})
target_compile_definitions(jerry PUBLIC
    JERRY_ERROR_MESSAGES=1
    JERRY_LINE_INFO=1
    JERRY_CPOINTER_32_BIT=1
    JERRY_VM_EXEC_STOP=1
)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message("-- Configured JerryScript for: 32 bit")
    target_compile_definitions(jerry PUBLIC
        JERRY_SYSTEM_ALLOCATOR=1
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message("-- Configured JerryScript for: 64 bit")
    target_compile_definitions(jerry PUBLIC
        JERRY_GLOBAL_HEAP_SIZE=200*1024
        JERRY_SYSTEM_ALLOCATOR=0
    )
else()
    message(FATAL_ERROR "-- Incorrect sizeof(void*)")
endif()

add_library(v8headers INTERFACE)
target_include_directories(v8headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/v8-headers)
