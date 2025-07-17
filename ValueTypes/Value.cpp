#include "Value.h"
#include "Array.h"
#include "Bool.h"
#include "Number.h"
#include "Object.h"
#include "String.h"
#include <stdexcept>

namespace Json
{
	const Array& Value::asArray() const
	{
		const Array* ptr = dynamic_cast<const Array*>(this);
		if (!ptr) throw std::runtime_error("Value is not an Array");
		return *ptr;
	}

	const Bool& Value::asBool() const
	{
		const Bool* ptr = dynamic_cast<const Bool*>(this);
		if (!ptr) throw std::runtime_error("Value is not a Bool");
		return *ptr;
	}

	const Number& Value::asNumber() const
	{
		const Number* ptr = dynamic_cast<const Number*>(this);
		if (!ptr) throw std::runtime_error("Value is not a Number");
		return *ptr;
	}

	const Object& Value::asObject() const
	{
		const Object* ptr = dynamic_cast<const Object*>(this);
		if (!ptr) throw std::runtime_error("Value is not an Object");
		return *ptr;
	}

	const String& Value::asString() const
	{
		const String* ptr = dynamic_cast<const String*>(this);
		if (!ptr) throw std::runtime_error("Value is not a String");
		return *ptr;
	}
}
