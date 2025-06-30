#pragma once
#include "Value.h"

namespace Json
{
	class Number : public Value
	{
	private:
		double m_value;
		
	public:
		Number() : Value() {};
		Number(double value) : Value(), m_value(value) {};

		bool isArray() override { return false; }
		bool isBool() override { return false; }
		bool isNumber() override { return true; }
		bool isObject() override { return false; }
		bool isString() override { return false; }
		bool isNull() override { return false; }

		const auto& data() const { return m_value; };
		auto& data() { return m_value; };
	};
}

