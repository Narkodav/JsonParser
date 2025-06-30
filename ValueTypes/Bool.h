#pragma once
#include "Value.h"

namespace Json
{
	class Bool : public Value
	{
	private:
		bool m_value;
		
	public:
		Bool() : Value() {};
		Bool(bool value) : Value(), m_value(value) {};

		bool isArray() override { return false; }
		bool isBool() override { return true; }
		bool isNumber() override { return false; }
		bool isObject() override { return false; }
		bool isString() override { return false; }
		bool isNull() override { return false; }

		const auto& data() const { return m_value; };
		auto& data() { return m_value; };
	};
}

