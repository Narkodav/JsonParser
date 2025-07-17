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

		bool isArray() const override { return false; }
		bool isBool() const override { return true; }
		bool isNumber() const override { return false; }
		bool isObject() const override { return false; }
		bool isString() const override { return false; }
		bool isNull() const override { return false; }

		const auto& data() const { return m_value; };
		auto& data() { return m_value; };
	};
}

