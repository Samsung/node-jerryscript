cmake_minimum_required(VERSION 3.5)

# Set
set(DEVICEAPI_ROOT ${PROJECT_SOURCE_DIR}/deps/tizen-device-api)
set(JERRYV8_ROOT ${PROJECT_SOURCE_DIR})
set(DEVICEAPI_OUTPUT tizen_device_api)

# Source Files
file(GLOB_RECURSE DEVICEAPI_SRC_FILES ${DEVICEAPI_ROOT}/src/*.cpp)


# Include Paths
set(DEVICEAPI_INC_PATHS
    ${DEVICEAPI_ROOT}/src
    ${JERRYV8_ROOT}/deps/v8-headers
)

# Defines
set(DEVICEAPI_DEFINES
)

# Options
set(DEVICEAPI_FLAGS
)

set(DEVICEAPI_LDFLAGS
)

# Libraries
set(DEVICEAPI_STATIC_LIBS
)

# Target
message("***** Build Tizen Device API *****")
message("*Defines: ${DEVICEAPI_DEFINES}")
message("*Flags: ${DEVICEAPI_FLAGS}")
message("*Include Paths: ${DEVICEAPI_INC_PATHS}")

add_library(${DEVICEAPI_OUTPUT} STATIC ${DEVICEAPI_SRC_FILES})
target_include_directories(${DEVICEAPI_OUTPUT} PRIVATE ${DEVICEAPI_INC_PATHS})
target_compile_definitions(${DEVICEAPI_OUTPUT} PRIVATE ${DEVICEAPI_DEFINES})
target_compile_options(${DEVICEAPI_OUTPUT} PRIVATE ${DEVICEAPI_FLAGS})
target_link_libraries(${DEVICEAPI_OUTPUT} PRIVATE ${DEVICEAPI_STATIC_LIBS} ${DEVICEAPI_LDFLAGS})
set_target_properties(${DEVICEAPI_OUTPUT} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_ROOT}"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_ROOT}"
)
