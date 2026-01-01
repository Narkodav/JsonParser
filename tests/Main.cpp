#include "JsonParser/Value.h"
#include "JsonParser/Utils/MappedFile.h"
#include <iostream>
#define assert(x) if(!(x)) throw std::runtime_error("Assertion failed: " #x);
#include <vector>
#include <variant>


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
		std::cout << val.stringify();
	}

	//func({ { "a", false },  { "b", 1 } });

	Json::Value doc1;
	Json::Value doc2;
	try
	{
		doc1 = Json::Value::fromFile("tests/test.json");
		MappedFile file("tests/test.json");
		doc2 = Json::Value::parse(file);
		assert(doc1 == doc2);

		std::cout << "doc1: " << doc1.stringify() << std::endl;
		std::cout << "doc2: " << doc2.stringify() << std::endl;

		std::cout << "All tests passed!" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}