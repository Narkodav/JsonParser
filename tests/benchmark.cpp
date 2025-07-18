#include "JsonParser/Parser.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>

void benchmarkFile(const std::string& filename, int iterations = 10000) {
    std::cout << "Benchmarking " << filename << " with " << iterations << " iterations..." << std::endl;
    
    // Read file content once
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Failed to open " << filename << std::endl;
        return;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    std::cout << "File size: " << content.size() << " bytes" << std::endl;
    
    // Benchmark parsing
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        try {
            auto doc = Json::Parser::parseFile(filename);
            // Prevent optimization from removing the parsing
            volatile size_t size = doc.roots().size();
            (void)size;
        } catch (const std::exception& e) {
            std::cout << "Parse error on iteration " << i << ": " << e.what() << std::endl;
            return;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double avgTime = static_cast<double>(duration.count()) / iterations;
    double throughput = (content.size() * iterations) / (1024.0 * 1024.0) / (duration.count() / 1000.0);
    
    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
    std::cout << "Average time per parse: " << avgTime << " ms" << std::endl;
    std::cout << "Throughput: " << throughput << " MB/s" << std::endl;
    std::cout << "Parses per second: " << (iterations * 1000.0 / duration.count()) << std::endl;
    std::cout << std::endl;
}

void benchmarkString(const std::string& json, const std::string& name, int iterations = 100000) {
    std::cout << "Benchmarking " << name << " with " << iterations << " iterations..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        try {
            auto doc = Json::Parser::parse(json);
            volatile size_t size = doc.roots().size();
            (void)size;
        } catch (const std::exception& e) {
            std::cout << "Parse error: " << e.what() << std::endl;
            return;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double avgTime = static_cast<double>(duration.count()) / iterations;
    
    std::cout << "Total time: " << duration.count() << " μs" << std::endl;
    std::cout << "Average time per parse: " << avgTime << " μs" << std::endl;
    std::cout << "Parses per second: " << (iterations * 1000000.0 / duration.count()) << std::endl;
    std::cout << std::endl;
}

int main() {
    std::cout << "=== JSON Parser Benchmark ===" << std::endl;
    
    // Benchmark files
    benchmarkFile("test.json", 5000);
    benchmarkFile("test_edge_cases.json", 3000);
    
    // Benchmark simple strings
    benchmarkString("{\"key\": \"value\"}", "simple object", 50000);
    benchmarkString("[1, 2, 3, 4, 5]", "simple array", 50000);
    benchmarkString("42.5e10", "number", 100000);
    benchmarkString("\"hello world\"", "string", 100000);
    
    // Benchmark complex string
    benchmarkString(R"({
        "name": "John",
        "age": 30,
        "scores": [95.5, 87, 92.3e-1],
        "address": {
            "street": "123 Main St",
            "city": "Anytown"
        }
    })", "complex object", 20000);
    
    std::cout << "Benchmark complete!" << std::endl;
    return 0;
}

//#include <chrono>
//#include <iostream>
//#include <string>
//#include <vector>
//#include "SIMDFunctions.h"
//
//int main() {
//    // Create test data with lots of whitespace
//    std::string data(1000000, ' ');
//    for (size_t i = 0; i < data.size(); i += 2) {
//        data[i] = 'X'; // Non-whitespace every 100 chars
//    }
//
//    for (int i = 0; i < data.size(); ++i) {
//        volatile size_t result = skipWhitespacesScalar(data, i);
//        i = result;
//    }
//
//#ifdef HAS_SSE2
//    for (int i = 0; i < data.size(); ++i) {
//        volatile size_t result = skipWhitespacesSIMD16(data, i);
//        i = result;
//    }
//#endif
//
//#ifdef HAS_AVX2
//    for (int i = 0; i < data.size(); ++i) {
//        volatile size_t result = skipWhitespacesSIMD32(data, i);
//        i = result;
//    }
//#endif
//
//    // Benchmark scalar
//    auto start = std::chrono::high_resolution_clock::now();
//    for (int i = 0; i < data.size(); ++i) {
//        volatile size_t result = skipWhitespacesScalar(data, i);
//        i = result;
//    }
//    auto end = std::chrono::high_resolution_clock::now();
//    auto scalar_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//
//#ifdef HAS_SSE2
//    // Benchmark SSE2
//    start = std::chrono::high_resolution_clock::now();
//    for (int i = 0; i < data.size(); ++i) {
//        volatile size_t result = skipWhitespacesSIMD16(data, i);
//        i = result;
//    }
//    end = std::chrono::high_resolution_clock::now();
//    auto sse2_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//#endif
//
//#ifdef HAS_AVX2
//    // Benchmark AVX2
//    start = std::chrono::high_resolution_clock::now();
//    for (int i = 0; i < data.size(); ++i) {
//        volatile size_t result = skipWhitespacesSIMD32(data, i);
//        i = result;
//    }
//    end = std::chrono::high_resolution_clock::now();
//    auto avx2_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//#endif
//
//    std::cout << "Scalar: " << scalar_time.count() << " μs\n";
//#ifdef HAS_SSE2
//    std::cout << "SSE2:   " << sse2_time.count() << " μs (";
//    std::cout << (double)scalar_time.count() / sse2_time.count() << "x faster)\n";
//#endif
//#ifdef HAS_AVX2
//    std::cout << "AVX2:   " << avx2_time.count() << " μs (";
//    std::cout << (double)scalar_time.count() / avx2_time.count() << "x faster)\n";
//#endif
//
//    return 0;
//}