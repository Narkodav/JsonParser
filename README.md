# JsonParser Library

A high-performance C++ JSON parser library with SIMD optimizations.

## Building with CMake

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler

### Build Instructions

1. Create a build directory:
```
mkdir -p builds
cd builds
```

2. Configure the project:
```
cmake ..
```

3. Build the library and tests:
```
cmake --build .
```

4. Run the tests:
```
cd ../bin
./JsonParserTests
```

## Features
- Fast JSON parsing with SIMD optimizations (AVX2/SSE2 when available)
- Support for all JSON data types
- Support for comments in JSON
- Memory-mapped file support for efficient parsing of large files
- Support for JSON Lines format (multiple JSON documents)

## License
See the LICENSE.txt file for details.