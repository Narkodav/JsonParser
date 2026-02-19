cmake_minimum_required(VERSION 3.20)
project(JsonParser VERSION 1.0.0 LANGUAGES CXX)

add_library(JsonParser INTERFACE)
add_library(JsonParser::JsonParser ALIAS ${PROJECT_NAME})

# Include directories
target_include_directories(JsonParser
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# C++ standard
target_compile_features(JsonParser INTERFACE cxx_std_20)

# SIMD detection (optional)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
if(COMPILER_SUPPORTS_AVX2)
#    target_compile_definitions(JsonParser INTERFACE HAS_AVX2)
    target_compile_options(JsonParser INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:-mavx>")
endif()

check_cxx_compiler_flag("-msse2" COMPILER_SUPPORTS_SSE2)
if(COMPILER_SUPPORTS_SSE2)
#    target_compile_definitions(JsonParser INTERFACE HAS_SSE2)
endif()

# =========================
# Benchmarks
# =========================
option(JSONPARSER_BUILD_BENCHMARKS "Build JsonParser benchmarks" ON)

if(JSONPARSER_BUILD_BENCHMARKS)
    file(GLOB TEST_SOURCES "tests/benchmark.cpp")

    add_executable(JsonParserBenchmark ${TEST_SOURCES})
    target_link_libraries(JsonParserBenchmark PRIVATE JsonParser)
    target_include_directories(JsonParserBenchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # Copy JSON test files
    file(GLOB TEST_FILES "tests/*.json")
    foreach(TEST_FILE ${TEST_FILES})
        get_filename_component(FILENAME ${TEST_FILE} NAME)
        configure_file(${TEST_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME} COPYONLY)
    endforeach()
endif()

# =========================
# Installation
# =========================
install(TARGETS ${PROJECT_NAME}
    EXPORT JsonParserTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

install(EXPORT JsonParserTargets
    FILE JsonParserTargets.cmake
    NAMESPACE JsonParser::
    DESTINATION lib/cmake/JsonParser
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/install/JsonParserConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/JsonParserConfig.cmake
    INSTALL_DESTINATION lib/cmake/JsonParser
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/JsonParserConfig.cmake
    DESTINATION lib/cmake/JsonParser
)