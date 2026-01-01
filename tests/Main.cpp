#include "JsonParser/Value.h"

#include <iostream>
#define assert(x) if(!(x)) throw std::runtime_error("Assertion failed: " #x);
#include <vector>
#include <variant>


int main() {

	//{
	//	Json::Value val = {
	//			  {"pi", 3.141},
	//			  {"happy", true},
	//			  {"name", "Niels"},
	//			  {"nothing", nullptr},
	//			  {"answer", {
	//				{"everything", 42}
	//			  }},
	//			  {"list", {1, 0, 2}},
	//			  {"object", {
	//				{"currency", "USD"},
	//				{"value", 42.99}
	//				}
	//			  },
	//			  {"different", Json::Value::array({"abc", 1, true})},
	//	};
	//	std::cout << val.stringify();
	//}




	//func({ { "a", false },  { "b", 1 } });

	Json::Value doc;
	try
	{
		doc = Json::Value::fromFile("tests/test.json");
		std::cout << doc.stringify() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}


	return 0;
}