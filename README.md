# JsonParser Library

A high-performance C++ JSON parser library with SIMD optimizations.

## Building with CMake

### Prerequisites
- CMake 3.10 or higher
- C++20 compatible compiler

### Build Instructions

1. Create a build directory:
```
mkdir -p builds
cd builds
```

2. Configure the project with Visual Studio:
```
cmake -G "Visual Studio 17 2022" -A x64 ..
```

3. Build all configurations:
```
cmake --build . --config Debug        # Dynamic Debug Runtime (MDd)
cmake --build . --config Release      # Dynamic Release Runtime (MD)
cmake --build . --config DebugStatic  # Static Debug Runtime (MTd)
cmake --build . --config ReleaseStatic # Static Release Runtime (MT)
```

4. Alternatively, open the solution in Visual Studio:
   - Open `builds/JsonParser.sln`
   - Select the desired configuration from the dropdown
   - Build the solution

5. Run the benchmarks:
```
cd ../bin/ReleaseDll
JsonParserBenchmark_MD.exe
```

### Output Directories
- `bin/DebugDll/` - Debug builds with dynamic runtime (MDd)
- `bin/ReleaseDll/` - Release builds with dynamic runtime (MD)
- `bin/DebugStatic/` - Debug builds with static runtime (MTd)
- `bin/ReleaseStatic/` - Release builds with static runtime (MT)

## Features
- Fast JSON parsing with SIMD optimizations (AVX2/SSE2 when available)
- Support for all JSON data types
- Support for comments in JSON
- Memory-mapped file support for efficient parsing of large files
- Support for JSON Lines format (multiple JSON documents)

## License
See the LICENSE.txt file for details.