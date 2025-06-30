#include "Parser.h"

#include <iostream>
#define assert(x) if(!(x)) throw std::runtime_error("Assertion failed: " #x);

int main()
{
	Json::Document doc;
	try
	{
		doc = Json::Parser::parseFile("test.json");
		std::cout << doc.roots().size() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	try
	{
		std::cout << doc.roots().size() << std::endl;
		assert(doc.roots().size() == 3)
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}