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

		virtual bool isArray() const = 0;
		virtual bool isBool() const = 0;
		virtual bool isNumber() const = 0;
		virtual bool isObject() const = 0;
		virtual bool isString() const = 0;
		virtual bool isNull() const = 0;

		const Array& asArray() const;
		const Bool& asBool() const;
		const Number& asNumber() const;
		const Object& asObject() const;
		const String& asString() const;
	};
}

