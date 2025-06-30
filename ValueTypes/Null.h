#pragma once
#include "Value.h"

namespace Json
{
	class Null : public Value
	{
	public:
		Null() : Value() {};

		bool isArray() override { return false; }
		bool isBool() override { return false; }
		bool isNumber() override { return false; }
		bool isObject() override { return false; }
		bool isString() override { return false; }
		bool isNull() override { return true; }
	};
}

