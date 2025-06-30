#pragma once
#include "Value.h"

#include <vector>
#include <memory>

namespace Json
{
	class Parser;
	class Array : public Value
	{
		friend class Parser;
	private:
		std::vector<std::unique_ptr<Value>> m_values;
		
	public:
		Array() : Value() {};

		bool isArray() override { return true; }
		bool isBool() override { return false; }
		bool isNumber() override { return false; }
		bool isObject() override { return false; }
		bool isString() override { return false; }
		bool isNull() override { return false; }

		const auto& data() const { return m_values; };
		auto& data() { return m_values; };
	};
}

