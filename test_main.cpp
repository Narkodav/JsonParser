#include "Parser.h"
#include <iostream>
#include <fstream>
#include <sstream>

void testFile(const std::string& filename, bool shouldSucceed = true) {
	std::cout << "Testing " << filename << "... ";
	try {
		auto doc = Json::Parser::parseFile(filename);
		if (shouldSucceed) {
			std::cout << "PASSED (" << doc.roots().size() << " documents)" << std::endl;
		} else {
			std::cout << "FAILED (should have thrown error)" << std::endl;
		}
	} catch (const std::exception& e) {
		if (shouldSucceed) {
			std::cout << "FAILED: " << e.what() << std::endl;
		} else {
			std::cout << "PASSED (correctly threw)" << std::endl;
		}
	}
}

void testString(const std::string& json, const std::string& testName, bool shouldSucceed = true) {
	std::cout << "Testing " << testName << "... ";
	try {
		auto doc = Json::Parser::parse(json);
		if (shouldSucceed) {
			std::cout << "PASSED" << std::endl;
		} else {
			std::cout << "FAILED (should have thrown error)" << std::endl;
		}
	} catch (const std::exception& e) {
		if (shouldSucceed) {
			std::cout << "FAILED: " << e.what() << std::endl;
		} else {
			std::cout << "PASSED (correctly threw)" << std::endl;
		}
	}
}

int main() {
	std::cout << "=== JSON Parser Tests ===" << std::endl;
	
	testFile("test.json", true);
	testFile("test_edge_cases.json", true);
	testFile("test_invalid.json", false);
	
	testString("{\"key\": \"value\"}", "simple object");
	testString("[1, 2, 3]", "simple array");
	testString("42", "simple number");
	testString("true", "boolean");
	testString("null", "null value");
	testString("1.23e10", "scientific notation");
	testString("{\"key\": \"value\"} // comment", "line comment");
	testString("{/* comment */\"key\"   : \"value\"}", "block comment");
	testString("[1 , 2  , 3,]", "trailing comma");

	testString("{key  : \"value\"}", "unquoted key", false);
	testString("{\"key\"  : True}", "invalid boolean", false);
	
	
	std::cout << "Tests Complete" << std::endl;
	return 0;
}