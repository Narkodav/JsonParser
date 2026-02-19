#include "JsonParser/Value.h"
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
            auto doc = Json::Value::fromFile(filename);
            // Prevent optimization from removing the parsing
            volatile size_t size = doc.size();
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
            auto doc = Json::Value::parse(json);
            volatile size_t size = doc.size();
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
    benchmarkFile("tests/test.json", 5000);
    benchmarkFile("tests/test_edge_cases.json", 3000);
    
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