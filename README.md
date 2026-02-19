# JsonParser Library

A high-performance C++ JSON parser library with SIMD optimizations.

## Features
- Fast JSON parsing with SIMD optimizations (AVX2/SSE2 when available)
- Support for all JSON data types
- Support for comments in JSON
- Memory-mapped file support for efficient parsing of large files
- Support for JSON Lines format (multiple JSON documents)

## Building with CMake

### Prerequisites
- CMake 3.20 or higher
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2022+)
- Optional: AVX2 or SSE2 support for SIMD optimizations

### Build the library and benchmarks:
```
# Clone the repository (if not already done)
git clone <repository-url>
cd JsonParser

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Run benchmarks (if built)
./JsonParserBenchmark
```

### Build with benchmarks enabled (default):
```
cmake ..
```

### Build without benchmarks:
```
cmake .. -DJSONPARSER_BUILD_BENCHMARKS=OFF
```

### Build in Release mode with optimizations:
```
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Build with Debug symbols:
```
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### SIMD Optimizations
The library automatically detects and enables SIMD support based on your compiler and architecture:

AVX2: Automatically enabled if compiler supports -mavx2

SSE2: Automatically detected (typically available on x86_64)

To disable SIMD optimizations, you can set compiler flags explicitly:
```
cmake .. -DCMAKE_CXX_FLAGS="-mno-avx"
```

### Installation

Install the library system-wide:
```
cmake --install . --prefix /usr/local
# or with custom prefix
cmake --install . --prefix /path/to/install
```

### Using the Library in Other Projects

Method 1: find_package (after installation):
```
find_package(JsonParser REQUIRED)
target_link_libraries(MyProject PRIVATE JsonParser::JsonParser)
```

Method 2: add_subdirectory (if included in your project)
```
add_subdirectory(JsonParser)
target_link_libraries(MyProject PRIVATE JsonParser)
# or aliased:
target_link_libraries(MyProject PRIVATE JsonParser::JsonParser)
```

### Project Structure
```
JsonParser/
├── include/           # Public header files
├── tests/             # Benchmark and test files
│   ├── benchmark.cpp  # Benchmark executable
│   ├── example.cpp    # Usage example
│   └── *.json         # JSON test files
├── cmake/             # CMake configuration files
│   └── JsonParserConfig.cmake.in
└── CMakeLists.txt     # Main CMake configuration
```

## API Usage Examples

```
Basic usage example:
#include "JsonParser/Value.h"
#include "JsonParser/Utils/MappedFile.h"
#include <iostream>
#include <vector>
#include <variant>
#include <chrono>
#include <string>

#define assert(x) if(!(x)) throw std::runtime_error("Assertion failed: " #x);

int main() {
    // Creating JSON values using initializer lists
    {
        Json::Value val = {
            {"pi", 3.141},
            {"happy", true},
            {"name", "Niels"},
            {"nothing", nullptr},
            {"answer", {
                {"everything", 42}
            }},
            {"list", {1, 0, 2}},
            {"object", {
                {"currency", "USD"},
                {"value", 42.99}
            }},
            {"different", Json::Value::array({"abc", 1, true})},
        };
        
        Json::Value val1 = { {"name", "Niels"} };
        
        // Stringify JSON to string
        std::string stringified = val.stringify();        
        std::cout << val.stringify() << "\n\n-----\n" << std::endl;
        
        // Parse from string
        Json::Value other = Json::Value::parse(stringified)[0];        
        std::cout << other.stringify() << std::endl;
        
        // Value comparison
        assert(val.asObject().size() == other.asObject().size());
        assert(val == other);
    }

    // Reading from files
    std::vector<Json::Value> doc1;
    std::vector<Json::Value> doc2;
    
    try {
        // Parse JSON file
        doc1 = Json::Value::fromFile("tests/test.json");
        
        // Parse using memory-mapped file for better performance
        MappedFile file("tests/test.json");
        doc2 = Json::Value::parse(file);
        
        // Verify both parsing methods produce the same result
        assert(doc1 == doc2);
        
        for(size_t i = 0; i < doc1.size(); ++i) {
            assert(doc1[i] == doc2[i]);
            std::cout << "doc1: " << doc1[i].stringify() << std::endl;
            std::cout << "\n\n-----\n" << std::endl;
            std::cout << "doc2: " << doc2[i].stringify() << std::endl;
        }
        
        std::cout << "All tests passed!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Strict parsing (enforces complete JSON compliance, use for better performance)
    Json::Value val = Json::Value::fromFileStrict("tests/standard_compliant.json");
    std::cout << "val: " << val.stringify() << std::endl;

    // Creating error response with timestamps
    Json::Value errorResponse = {
        {"error", {
            {"code", 404},
            {"message", "Resource not found"},
            {"method", "GET"}
        }},
        {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
    };

    // Type checking examples
    Json::Value f = std::chrono::system_clock::now().time_since_epoch().count();
    Json::Value f1 = 123.03;
    Json::Value f2 = 123;
    Json::Value f3 = true;

    assert(f1.isNumber());
    assert(f2.isInteger());
    assert(f3.isBool());

    // String values
    std::string str = "asdsdf";
    Json::Value s1 = { {"abc", str } };

    return 0;
}
```

## Contributing
Contributions are welcome! Please feel free to submit a Pull Request. For major changes,
please open an issue first to discuss what you would like to change.

## License
See the LICENSE.txt file for details.
