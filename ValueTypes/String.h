#pragma once
#include "Value.h"

#include <string>
#include <utility>

namespace Json
{
	class String : public Value
	{
	private:
		std::string m_value;
	public:
		String() : Value() {};

		template <typename T>
		String(T&& value) : Value(), m_value(std::forward<T>(value)) {};

		bool isArray() const override { return false; }
		bool isBool() const override { return false; }
		bool isNumber() const override { return false; }
		bool isObject() const override { return false; }
		bool isString() const override { return true; }
		bool isNull() const override { return false; }

		const auto& data() const { return m_value; };
		auto& data() { return m_value; };
	};
}

