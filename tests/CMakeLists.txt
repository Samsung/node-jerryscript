# ArrayBuffer
add_executable(arraybuffer arraybuffer.cpp)
target_link_libraries(arraybuffer v8jerry)
target_include_directories(arraybuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
