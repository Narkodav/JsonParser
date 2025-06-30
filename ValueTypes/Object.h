#pragma once
#include "Value.h"

#include <unordered_map>
#include <memory>
#include <string>

namespace Json
{
	class Parser;
	class Object : public Value
	{
		friend class Parser;
	private:
		std::unordered_map<std::string, std::unique_ptr<Value>> m_values;

	public:
		Object() : Value() {};

		bool isArray() override { return false; }
		bool isBool() override { return false; }
		bool isNumber() override { return false; }
		bool isObject() override { return true; }
		bool isString() override { return false; }
		bool isNull() override { return false; }

		const auto& data() const { return m_values; };
		auto& data() { return m_values; };
	};
}

