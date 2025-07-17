#pragma once
#include "Value.h"

namespace Json
{
	class Null : public Value
	{
	public:
		Null() : Value() {};

		bool isArray() const override { return false; }
		bool isBool() const override { return false; }
		bool isNumber() const override { return false; }
		bool isObject() const override { return false; }
		bool isString() const override { return false; }
		bool isNull() const override { return true; }
	};
}

