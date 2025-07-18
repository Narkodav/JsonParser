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

		bool isArray() const override { return false; }
		bool isBool() const override { return false; }
		bool isNumber() const override { return true; }
		bool isObject() const override { return false; }
		bool isString() const override { return false; }
		bool isNull() const override { return false; }

		const auto& data() const { return m_value; };
		auto& data() { return m_value; };
	};
}

