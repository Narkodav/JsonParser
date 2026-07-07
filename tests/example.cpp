#include "JsonParser/Value.h"
#include "JsonParser/Utils/MappedFile.h"
#include "JsonParser/SchemaParsing.h"

#include <iostream>
#include <vector>
#include <variant>
#include <chrono>
#include <string>
#include <stdexcept>

#define assert(x) if(!(x)) throw std::runtime_error("Assertion failed: " #x);

int main() {

	// Creating JSON values using initializer lists
	{
		Json::Value val = Json::ObjectList{
			{"pi", 3.141},
			{"happy", true},
			{"name", "Niels"},
			{"nothing", nullptr},
			{"answer", Json::ObjectList{
				{"everything", 42}
			}},
			{"list", {1, 0, 2}},
			{"object", Json::ObjectList{
				{"currency", "USD"},
				{"value", 42.99}
				}
			},
			{"different", {"abc", 1, true}},
		};
		Json::Value val1 = Json::ObjectList{ {"name", "Niels"} };

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
	try
	{
        // Parse JSON file
        doc1 = Json::Value::fromFile("tests/test.json");

        // Parse using memory-mapped file for better performance
        MappedFile file("tests/test.json");
        doc2 = Json::Value::parse(file);

        // Verify both parsing methods produce the same result
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

    // Strict parsing (enforces complete JSON compliance, use for better performance)
    Json::Value val = Json::Value::fromFileStrict("tests/standard_compliant.json");
    std::cout << "val: " << val.stringify() << std::endl;

    // Creating error response with timestamps
    Json::Value errorResponse = Json::ObjectList{
        {"error", Json::ObjectList{
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
	Json::Value s1 = Json::ObjectList{ {"abc", str } };

	// Schema based struct conversion
    struct Vec3 {
        float x, y, z;
		using Schema = Json::StructSchema<
        Json::StructMemberSchema<"x", &Vec3::x, Json::PrimitiveSchema>,
        Json::StructMemberSchema<"y", &Vec3::y, Json::PrimitiveSchema>,
        Json::StructMemberSchema<"z", &Vec3::z, Json::PrimitiveSchema>
    >;
    };

    using ArraySchema = Json::ContainerSchema<Json::PrimitiveSchema>;
    using NestedArraySchema = Json::ContainerSchema<ArraySchema>;

    using VecArraySchema = Json::ContainerSchema<Vec3::Schema>;
    using NestedVecArraySchema = Json::ContainerSchema<VecArraySchema>;

    struct TestStruct {
        Vec3 vec;
        std::vector<int> a1;
        std::vector<std::vector<int>> a2;
        std::vector<Vec3> a3;
        std::vector<std::vector<Vec3>> a4;
        int val;
		using Schema = Json::StructSchema<
			Json::StructMemberSchema<"vec", &TestStruct::vec, Vec3::Schema>,
			Json::StructMemberSchema<"a1", &TestStruct::a1, ArraySchema>,
			Json::StructMemberSchema<"a2", &TestStruct::a2, NestedArraySchema>,
			Json::StructMemberSchema<"a3", &TestStruct::a3, VecArraySchema>,
			Json::StructMemberSchema<"a4", &TestStruct::a4, NestedVecArraySchema>,
			Json::StructMemberSchema<"val", &TestStruct::val, Json::PrimitiveSchema>
    >;
    };

	// Construct test Value
	Json::Value val2 = Json::ObjectList{
		{ "vec", Json::ObjectList{
			{ "x", 1.0f },
			{ "y", 1.0f },
			{ "z", 1.0f },
		}},
		{ 	
			"a1", 
			{0, 1, 2, 3, 4, 5}
		},
		{
			"a2", 
			{{0, 1, 2}, {0, 1, 2}, {0, 1, 2}}
		},
		{
			"a3", {
			Json::ObjectList{
				{ "x", 2.0f },
				{ "y", 2.0f },
				{ "z", 2.0f },
			}
		}},
		{
			"a4", {
			{Json::ObjectList{
				{ "x", 2.0f },
				{ "y", 2.0f },
				{ "z", 2.0f },
			},Json::ObjectList{
				{ "x", 2.0f },
				{ "y", 2.0f },
				{ "z", 2.0f },
			}},
			{Json::ObjectList{
				{ "x", 2.0f },
				{ "y", 2.0f },
				{ "z", 2.0f },
			},Json::ObjectList{
				{ "x", 2.0f },
				{ "y", 2.0f },
				{ "z", 2.0f },
			}}
		}},
		{
			"val", 1
		}
	};

	TestStruct s = val2.toStruct<TestStruct::Schema, TestStruct>();

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

	// Write Struct to a Value
	Json::Value val3;
	val3.fromStruct<TestStruct::Schema>(s);
	assert(val2 == val3);

	return 0;
}