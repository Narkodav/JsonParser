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

		bool isArray() const override { return false; }
		bool isBool() const override { return false; }
		bool isNumber() const override { return false; }
		bool isObject() const override { return true; }
		bool isString() const override { return false; }
		bool isNull() const override { return false; }

		const auto& data() const { return m_values; };
		auto& data() { return m_values; };
	};
}

