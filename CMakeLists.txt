cmake_minimum_required(VERSION 3.10)
project(JsonParser VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define output directories with configuration-specific subdirectories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/DebugDll)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/DebugDll)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/DebugDll)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/ReleaseDll)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/ReleaseDll)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/ReleaseDll)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUGSTATIC ${CMAKE_SOURCE_DIR}/bin/DebugStatic)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUGSTATIC ${CMAKE_SOURCE_DIR}/bin/DebugStatic)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUGSTATIC ${CMAKE_SOURCE_DIR}/bin/DebugStatic)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASESTATIC ${CMAKE_SOURCE_DIR}/bin/ReleaseStatic)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASESTATIC ${CMAKE_SOURCE_DIR}/bin/ReleaseStatic)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASESTATIC ${CMAKE_SOURCE_DIR}/bin/ReleaseStatic)

# Set build directory
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/builds)

# Check for SIMD support
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
if(COMPILER_SUPPORTS_AVX2)
    add_compile_definitions(HAS_AVX2)
endif()

# Configure Visual Studio configurations
if(MSVC)
    # Set up the four configurations
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;DebugStatic;ReleaseStatic" CACHE STRING "" FORCE)
    
    # Create custom configurations for static runtime
    set(CMAKE_CXX_FLAGS_DEBUGSTATIC ${CMAKE_CXX_FLAGS_DEBUG})
    set(CMAKE_C_FLAGS_DEBUGSTATIC ${CMAKE_C_FLAGS_DEBUG})
    set(CMAKE_EXE_LINKER_FLAGS_DEBUGSTATIC ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUGSTATIC ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
    
    set(CMAKE_CXX_FLAGS_RELEASESTATIC ${CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_C_FLAGS_RELEASESTATIC ${CMAKE_C_FLAGS_RELEASE})
    set(CMAKE_EXE_LINKER_FLAGS_RELEASESTATIC ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASESTATIC ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
    
    # Set runtime libraries
    # Debug and Release use dynamic runtime (/MDd and /MD)
    string(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/MT" "/MD" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    
    # DebugStatic and ReleaseStatic use static runtime (/MTd and /MT)
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUGSTATIC "${CMAKE_CXX_FLAGS_DEBUGSTATIC}")
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASESTATIC "${CMAKE_CXX_FLAGS_RELEASESTATIC}")
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

# Set output names based on configuration
if(MSVC)
    # Static library output names
    set_target_properties(JsonParser PROPERTIES 
        OUTPUT_NAME_DEBUG JsonParser_MDd
        OUTPUT_NAME_RELEASE JsonParser_MD
        OUTPUT_NAME_DEBUGSTATIC JsonParser_MTd
        OUTPUT_NAME_RELEASESTATIC JsonParser_MT
    )
    
    # Shared library output names
    set_target_properties(JsonParser_shared PROPERTIES 
        OUTPUT_NAME_DEBUG JsonParser_MDd
        OUTPUT_NAME_RELEASE JsonParser_MD
        OUTPUT_NAME_DEBUGSTATIC JsonParser_MTd
        OUTPUT_NAME_RELEASESTATIC JsonParser_MT
    )
endif()

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

# Set output names based on configuration
if(MSVC)
    # Static benchmark output names
    set_target_properties(JsonParserBenchmark PROPERTIES 
        OUTPUT_NAME_DEBUG JsonParserBenchmark_MDd
        OUTPUT_NAME_RELEASE JsonParserBenchmark_MD
        OUTPUT_NAME_DEBUGSTATIC JsonParserBenchmark_MTd
        OUTPUT_NAME_RELEASESTATIC JsonParserBenchmark_MT
    )
    
    # Shared benchmark output names
    set_target_properties(JsonParserBenchmark_shared PROPERTIES 
        OUTPUT_NAME_DEBUG JsonParserBenchmark_shared_MDd
        OUTPUT_NAME_RELEASE JsonParserBenchmark_shared_MD
        OUTPUT_NAME_DEBUGSTATIC JsonParserBenchmark_shared_MTd
        OUTPUT_NAME_RELEASESTATIC JsonParserBenchmark_shared_MT
    )
endif()

# Copy test files to bin directory for benchmarking
file(GLOB TEST_FILES "tests/*.json")
foreach(TEST_FILE ${TEST_FILES})
    get_filename_component(FILENAME ${TEST_FILE} NAME)
    configure_file(${TEST_FILE} ${CMAKE_SOURCE_DIR}/bin/DebugDll/${FILENAME} COPYONLY)
    configure_file(${TEST_FILE} ${CMAKE_SOURCE_DIR}/bin/ReleaseDll/${FILENAME} COPYONLY)
    configure_file(${TEST_FILE} ${CMAKE_SOURCE_DIR}/bin/DebugStatic/${FILENAME} COPYONLY)
    configure_file(${TEST_FILE} ${CMAKE_SOURCE_DIR}/bin/ReleaseStatic/${FILENAME} COPYONLY)
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