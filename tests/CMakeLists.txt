function(add_test FILENAME)
    get_filename_component(FILENAME_WITHOUT_EXT ${FILENAME} NAME_WE)
    add_executable(${FILENAME_WITHOUT_EXT} ${FILENAME})
    target_link_libraries(${FILENAME_WITHOUT_EXT} v8jerry)

    if(USE_V8)
        add_executable(v8_${FILENAME_WITHOUT_EXT} ${FILENAME})
        target_include_directories(v8_${FILENAME_WITHOUT_EXT} PUBLIC ${USE_V8_HEADER_DIR})
        target_link_libraries(v8_${FILENAME_WITHOUT_EXT} ${V8_LIBS})
    endif()
endfunction(add_test)

add_test(test_hello.cpp)

# ArrayBuffer
add_test(arraybuffer.cpp)
