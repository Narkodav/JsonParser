#include "JsonParser/Value.h"
#include "JsonParser/Utils/MappedFile.h"
#include <iostream>
#define assert(x) if(!(x)) throw std::runtime_error("Assertion failed: " #x);
#include <vector>
#include <variant>
#include <chrono>
#include <string>
#include <stdexcept>

int main() {

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
				}
			},
			{"different", Json::Value::array({"abc", 1, true})},
		};
		Json::Value val1 = { {"name", "Niels"} };
		std::string stringified = val.stringify();		

		Json::Value other = Json::Value::parse(stringified)[0];		
		std::cout << val.stringify();
		std::cout << "\n\n-----" << std::endl;
		std::cout << other.stringify();

		assert(val.asObject().size() == other.asObject().size());

		assert(val == other);
	}

	std::vector<Json::Value> doc1;
	std::vector<Json::Value> doc2;
	try
	{
		doc1 = Json::Value::fromFile("tests/test.json");
		MappedFile file("tests/test.json");
		doc2 = Json::Value::parse(file);
		assert(doc1 == doc2);

		for(size_t i = 0; i < doc1.size(); ++i)
		{
			assert(doc1[i] == doc2[i]);
			std::cout << "doc1: " << doc1[i].stringify() << std::endl;
			std::cout << "\n\n-----\n" << std::endl;
			std::cout << "doc2: " << doc2[i].stringify() << std::endl;
		}

		std::cout << "All tests passed!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	Json::Value val = Json::Value::fromFileStrict("tests/standard_compliant.json");

	std::cout << "val: " << val.stringify() << std::endl;

	Json::Value errorResponse = {
		{"error", {
			{"code", 404},
			{"message", "Resource not found"},
			//{"path", req.getUri()},
			{"method", "GET"}
		}
		},
		{"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
	};

	Json::Value f = std::chrono::system_clock::now().time_since_epoch().count();

	auto data = std::chrono::system_clock::now().time_since_epoch().count();

	Json::Value f1 = 123.03;
	Json::Value f2 = 123;
	Json::Value f3 = true;

	assert(f1.isNumber());
	assert(f2.isInteger());
	assert(f3.isBool());

	std::string str = "asdsdf";

	Json::Value s1 = { {"abc", str } };


	return 0;
}