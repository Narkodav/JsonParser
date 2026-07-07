#include "JsonParser/Value.h"
#include "JsonParser/Utils/MappedFile.h"
#include <iostream>
#define assert(x) if(!(x)) throw std::runtime_error("Assertion failed: " #x);
#include <vector>
#include <variant>
#include <chrono>
#include <string>
#include <stdexcept>

#include "JsonParser/SchemaParser.h"

int main() {

	// {
	// 	Json::Value val = {
	// 		{"pi", 3.141},
	// 		{"happy", true},
	// 		{"name", "Niels"},
	// 		{"nothing", nullptr},
	// 		{"answer", {
	// 			{"everything", 42}
	// 		}},
	// 		{"list", {1, 0, 2}},
	// 		{"object", {
	// 			{"currency", "USD"},
	// 			{"value", 42.99}
	// 			}
	// 		},
	// 		{"different", Json::Value::array({"abc", 1, true})},
	// 	};
	// 	Json::Value val1 = { {"name", "Niels"} };
	// 	std::string stringified = val.stringify();		

	// 	Json::Value other = Json::Value::parse(stringified)[0];		
	// 	std::cout << val.stringify();
	// 	std::cout << "\n\n-----" << std::endl;
	// 	std::cout << other.stringify();

	// 	assert(val.asObject().size() == other.asObject().size());

	// 	assert(val == other);
	// }

	// std::vector<Json::Value> doc1;
	// std::vector<Json::Value> doc2;
	// try
	// {
	// 	doc1 = Json::Value::fromFile("tests/test.json");
	// 	MappedFile file("tests/test.json");
	// 	doc2 = Json::Value::parse(file);
	// 	assert(doc1 == doc2);

	// 	for(size_t i = 0; i < doc1.size(); ++i)
	// 	{
	// 		assert(doc1[i] == doc2[i]);
	// 		std::cout << "doc1: " << doc1[i].stringify() << std::endl;
	// 		std::cout << "\n\n-----\n" << std::endl;
	// 		std::cout << "doc2: " << doc2[i].stringify() << std::endl;
	// 	}

	// 	std::cout << "All tests passed!" << std::endl;
	// }
	// catch (const std::exception& e)
	// {
	// 	std::cerr << e.what() << std::endl;
	// }

	// Json::Value val = Json::Value::fromFileStrict("tests/standard_compliant.json");

	// std::cout << "val: " << val.stringify() << std::endl;

	// Json::Value errorResponse = {
	// 	{"error", {
	// 		{"code", 404},
	// 		{"message", "Resource not found"},
	// 		//{"path", req.getUri()},
	// 		{"method", "GET"}
	// 	}
	// 	},
	// 	{"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
	// };

	// Json::Value f = std::chrono::system_clock::now().time_since_epoch().count();

	// auto data = std::chrono::system_clock::now().time_since_epoch().count();

	// Json::Value f1 = 123.03;
	// Json::Value f2 = 123;
	// Json::Value f3 = true;

	// assert(f1.isNumber());
	// assert(f2.isInteger());
	// assert(f3.isBool());

	// std::string str = "asdsdf";

	// Json::Value s1 = { {"abc", str } };


    struct Vec3 {
        float x, y, z;
		using Schema = Json::MemberList<
        Json::Member<"x", &Vec3::x, Json::Primitive>,
        Json::Member<"y", &Vec3::y, Json::Primitive>,
        Json::Member<"z", &Vec3::z, Json::Primitive>
    >;
    };

    using ArraySchema = Json::ArrayList<Json::Primitive>;
    using NestedArraySchema = Json::ArrayList<ArraySchema>;

    using VecArraySchema = Json::ArrayList<Vec3::Schema>;
    using NestedVecArraySchema = Json::ArrayList<VecArraySchema>;

    struct TestStruct {
        Vec3 vec;
        std::vector<int> a1;
        std::vector<std::vector<int>> a2;
        std::vector<Vec3> a3;
        std::vector<std::vector<Vec3>> a4;
        int val;
		using Schema = Json::MemberList<
			Json::Member<"vec", &TestStruct::vec, Vec3::Schema>,
			Json::Member<"a1", &TestStruct::a1, ArraySchema>,
			Json::Member<"a2", &TestStruct::a2, NestedArraySchema>,
			Json::Member<"a3", &TestStruct::a3, VecArraySchema>,
			Json::Member<"a4", &TestStruct::a4, NestedVecArraySchema>,
			Json::Member<"val", &TestStruct::val, Json::Primitive>
    >;
    };

	Json::Value val = {
		Json::Pair{ "vec", {
			Json::Pair{ "x", 1.0f },
			Json::Pair{ "y", 1.0f },
			Json::Pair{ "z", 1.0f },
		}},
		Json::Pair{ 	
			"a1", 
			{0, 1, 2, 3, 4, 5}
		},
		Json::Pair{
			"a2", 
			{{0, 1, 2}, {0, 1, 2}, {0, 1, 2}}
		},
		Json::Pair{
			"a3", {
			{
				Json::Pair{ "x", 2.0f },
				Json::Pair{ "y", 2.0f },
				Json::Pair{ "z", 2.0f },
			}
		}},
		Json::Pair{
			"a4", {
			{{
				Json::Pair{ "x", 2.0f },
				Json::Pair{ "y", 2.0f },
				Json::Pair{ "z", 2.0f },
			},{
				Json::Pair{ "x", 2.0f },
				Json::Pair{ "y", 2.0f },
				Json::Pair{ "z", 2.0f },
			}},
			{{
				Json::Pair{ "x", 2.0f },
				Json::Pair{ "y", 2.0f },
				Json::Pair{ "z", 2.0f },
			},{
				Json::Pair{ "x", 2.0f },
				Json::Pair{ "y", 2.0f },
				Json::Pair{ "z", 2.0f },
			}}
		}},
		Json::Pair{
			"val", 1
		}
	};

	TestStruct s = val.toStruct<TestStruct::Schema, TestStruct>();

	std::cout << "x: " << s.vec.x << ", y: " << s.vec.y << ", z: " << s.vec.z << std::endl;
	for(size_t i = 0; i < s.a1.size(); ++i) std::cout << s.a1[i] << " ";
	std::cout << std::endl;

	for(size_t i = 0; i < s.a2.size(); ++i) {
		for(size_t j = 0; j < s.a2[i].size(); ++j) {
			std::cout << s.a2[i][j] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;

	for(size_t i = 0; i < s.a3.size(); ++i) std::cout << "x: " << s.a3[i].x << ", y: " << s.a3[i].y << ", z: " << s.a3[i].z << " ";
	std::cout << std::endl;

	for(size_t i = 0; i < s.a4.size(); ++i) {
		for(size_t j = 0; j < s.a4[i].size(); ++j) {
			std::cout << "x: " << s.a4[i][j].x << ", y: " << s.a4[i][j].y << ", z: " << s.a4[i][j].z << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	
	std::cout << "val: " << s.val << std::endl;

	return 0;
}