#pragma once
#include <vector>
#include <string>

namespace Json
{
	class Array;
	class Bool;
	class Number;
	class Object;
	class String;

	class Value
	{
	public:
		Value() = default;
		virtual ~Value() = default;

		virtual bool isArray() = 0;
		virtual bool isBool() = 0;
		virtual bool isNumber() = 0;
		virtual bool isObject() = 0;
		virtual bool isString() = 0;
		virtual bool isNull() = 0;

		const Array& asArray();
		const Bool& asBool();
		const Number& asNumber();
		const Object& asObject();
		const String& asString();
	};
}

