cmake_minimum_required(VERSION 3.10)
project(JsonParser VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Set build directory
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/builds)

# Check for SIMD support
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
if(COMPILER_SUPPORTS_AVX2)
    add_compile_definitions(HAS_AVX2)
endif()

check_cxx_compiler_flag("-msse2" COMPILER_SUPPORTS_SSE2)
if(COMPILER_SUPPORTS_SSE2)
    add_compile_definitions(HAS_SSE2)
endif()

# Library sources
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

file(GLOB_RECURSE HEADERS 
    "include/JsonParser/*.h"
)

# Create static library
add_library(JsonParser STATIC ${SOURCES} ${HEADERS})
target_include_directories(JsonParser 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Create shared library
add_library(JsonParser_shared SHARED ${SOURCES} ${HEADERS})
set_target_properties(JsonParser_shared PROPERTIES 
    OUTPUT_NAME JsonParser
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_include_directories(JsonParser_shared
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Tests
set(TEST_SOURCES "tests/benchmark.cpp")

# Main benchmark executable
add_executable(JsonParserBenchmark ${TEST_SOURCES})
target_link_libraries(JsonParserBenchmark PRIVATE JsonParser)
target_include_directories(JsonParserBenchmark PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Shared library benchmark executable
add_executable(JsonParserBenchmark_shared ${TEST_SOURCES})
target_link_libraries(JsonParserBenchmark_shared PRIVATE JsonParser_shared)
target_include_directories(JsonParserBenchmark_shared PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set_target_properties(JsonParserBenchmark_shared PROPERTIES OUTPUT_NAME JsonParserBenchmark_shared)

# Copy test files to bin directory for benchmarking
file(GLOB TEST_FILES "tests/*.json")
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(FILENAME ${TEST_FILE} NAME)
    configure_file(${TEST_FILE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${FILENAME} COPYONLY)
endforeach()

# Installation
install(TARGETS JsonParser JsonParser_shared
    EXPORT JsonParserTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

# Export targets
install(EXPORT JsonParserTargets
    FILE JsonParserTargets.cmake
    NAMESPACE JsonParser::
    DESTINATION lib/cmake/JsonParser
)

# Create version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/JsonParserConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Configure package
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/JsonParserConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/JsonParserConfig.cmake"
    @ONLY
)

# Install package configuration
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/JsonParserConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/JsonParserConfigVersion.cmake"
    DESTINATION lib/cmake/JsonParser
)