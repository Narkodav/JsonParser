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

		bool isArray() const override { return true; }
		bool isBool() const override { return false; }
		bool isNumber() const override { return false; }
		bool isObject() const override { return false; }
		bool isString() const override { return false; }
		bool isNull() const override { return false; }

		const auto& data() const { return m_values; };
		auto& data() { return m_values; };
	};
}

