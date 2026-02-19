#pragma once
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <span>
#include <stdexcept>
#include <string_view>
#include <limits>
#include <cstdint>
#include <fstream>

#include "JsonParser/Concepts.h"
#include "JsonParser/Utils/Macros.h"
#include "JsonParser/ContainerParser.h"
#include "JsonParser/StreamParser.h"
#include "JsonParser/StrictContainerParser.h"
#include "JsonParser/StrictStreamParser.h"

namespace Json
{
    namespace Detail
    {
        template<auto T>
        struct EnumToTypeTrait : std::false_type {};
    }

	class Value
	{
	public:
		enum class Type
		{			
            Array,
			Object,
			String,
			Bool,
			Integer,
			Number,
			Null
		};


		struct TransparentObjectHash {
			using is_transparent = void;

			size_t operator()(std::string_view sv) const noexcept {
				return std::hash<std::string_view>{}(sv);
			}

			size_t operator()(const std::string& s) const noexcept {
				return std::hash<std::string_view>{}(s);
			}

			size_t operator()(const char* s) const noexcept {
				return std::hash<std::string_view>{}(s);
			}
		};

		struct TransparentObjectEqual {
			using is_transparent = void;

			bool operator()(std::string_view lhs,
							std::string_view rhs) const noexcept {
				return lhs == rhs;
			}
		};

		using Object = std::unordered_map<std::string, Value,
			TransparentObjectHash, TransparentObjectEqual>;

		using Storage = std::variant<std::vector<Value>*, Object*, std::string*, bool, int64_t, double, std::nullptr_t>;

	protected:
		Storage m_value;

	public:
		Value() : m_value(nullptr) {};

		template<typename T>
		Value(std::initializer_list<T> values) requires std::convertible_to<T, bool>
			|| std::convertible_to<T, double>
			|| std::convertible_to<T, int>
			|| std::convertible_to<T, std::string>
			|| std::convertible_to<T, std::string_view>
			|| std::convertible_to<T, const char*>
			|| std::convertible_to<T, std::nullptr_t> {
			m_value = new std::vector<Value>();
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			arr.reserve(values.size());
			for (auto& val : values) {
				arr.emplace_back(std::move(val));
			}
		};
		Value(std::string_view value) {
			m_value = new std::string(value);
		}
		Value(const std::string& value) {
			m_value = new std::string(value);
		}
		template<size_t N>
		Value(const char (&value)[N]) {
			m_value = new std::string(value);
		}

		template<typename T>
		Value(T value) requires std::floating_point<T>
		{
			m_value = value;
		}

		template<typename T>
		Value(T value) requires std::integral<T> && (!std::same_as<T, bool>)
		{
			m_value = value;
		}

		template<typename T>
		Value(T value) requires std::same_as<T, bool> {
			m_value = value;
		}
		Value(std::initializer_list<std::pair<std::string, Value>> values) {
			m_value = new Object();
			auto& map = *std::get<Object*>(m_value);
			map.reserve(values.size());
			for (auto& pair : values) {
				map.emplace(pair.first, pair.second);
			}
		}
		Value(std::nullptr_t) {
			m_value = nullptr;
		}
		Type getType() const { return static_cast<Type>(m_value.index()); }

		template<Type T>
		const auto& get() const {
#ifndef NODEBUG
			switch (getType()) {
			case T:
				return std::get<Detail::EnumToTypeTrait<T>::Type>(m_value);
			default:
				JSON_VERIFY(false, "Type mismatch");
			}
#else
            return std::get<Detail::EnumToTypeTrait<T>::Type>(m_value);
#endif
		}

		~Value() {
			switch (getType()) {
			case Type::Array:
				delete std::get<std::vector<Value>*>(m_value);
				break;
			case Type::Object:
				delete std::get<Object*>(m_value);
				break;
			case Type::String:
				delete std::get<std::string*>(m_value);
				break;
			default:
				break;
			};
		}

		Value(const Value& other) {
			switch (getType()) {
			case Type::Array:
				m_value = new std::vector<Value>(*std::get<std::vector<Value>*>(other.m_value));
				break;
			case Type::Object:
				m_value = new Object(*std::get<Object*>(other.m_value));
				break;
			case Type::String:
				m_value = new std::string(*std::get<std::string*>(other.m_value));
				break;
			default:
				m_value = other.m_value;
				break;
			}
		}

		Value& operator=(const Value& other) {
			if (this != &other) {
				this->~Value();
				switch (other.getType()) {
				case Type::Array:
					m_value = new std::vector<Value>(*std::get<std::vector<Value>*>(other.m_value));
					break;
				case Type::Object:
					m_value = new Object(*std::get<Object*>(other.m_value));
					break;
				case Type::String:
					m_value = new std::string(*std::get<std::string*>(other.m_value));
					break;
				default:
					m_value = other.m_value;
					break;
				}
			}
			return *this;
		}

		Value(Value&& other) noexcept {
			m_value = std::move(other.m_value);
			other.m_value = nullptr;
		}
		Value& operator=(Value&& other) noexcept {
			if (this != &other) {
				this->~Value();
				m_value = std::move(other.m_value);
				other.m_value = nullptr;
			}
			return *this;
		}

		static Value array(std::initializer_list<Value> values = {}) {
			Value val;
			val.m_value = new std::vector<Value>(std::move(values));
			return val;
		}

		static Value object(std::initializer_list<std::pair<std::string, Value>> values = {}) {
			Value val;
			val.m_value = new Object();
			auto& map = *std::get<Object*>(val.m_value);
			for (auto& pair : values) {
				map.emplace(pair);
			}
			return val;
		}

		void pushBack(const Value& value) {
			JSON_VERIFY(getType() == Type::Array, "Type mismatch");
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			arr.push_back(value);
		}

        template<typename... Args>
		void emplaceBack(Args&&... args) {
			JSON_VERIFY(getType() == Type::Array, "Type mismatch");
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			arr.emplace_back(std::forward<Args>(args)...);
		}

		Value& operator[](const std::string& key) {
			JSON_VERIFY(getType() == Type::Object, "Type mismatch");
			auto& map = *std::get<Object*>(m_value);
			return map[key];
		}
		Value& operator[](size_t index) {
			JSON_VERIFY(getType() == Type::Array, "Type mismatch");
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			return arr[index];
		}

		bool& asBool() {
			JSON_VERIFY(getType() == Type::Bool, "Type mismatch");
			return std::get<bool>(m_value);
		}
		double& asNumber() {
			JSON_VERIFY(getType() == Type::Number, "Type mismatch");
			return std::get<double>(m_value);
		}
		int64_t& asInteger() {
			JSON_VERIFY(getType() == Type::Integer, "Type mismatch");
			return std::get<int64_t>(m_value);
		}
		std::string& asString() {
			JSON_VERIFY(getType() == Type::String, "Type mismatch");
			return *std::get<std::string*>(m_value);
		}
		std::vector<Value>& asArray() {
            JSON_VERIFY(getType() == Type::Array, "Type mismatch");
			return *std::get<std::vector<Value>*>(m_value);
		}
		Object& asObject() {
            JSON_VERIFY(getType() == Type::Object, "Type mismatch");
			return *std::get<Object*>(m_value);
		}

		const Value& operator[](const std::string& key) const {
			return const_cast<const Value&>(const_cast<Value*>(this)->operator[](key));
		}
		const Value& operator[](size_t index) const {
			return const_cast<const Value&>(const_cast<Value*>(this)->operator[](index));
		}
		const bool& asBool() const {
			return const_cast<const bool&>(const_cast<Value*>(this)->asBool());
		}
		const double& asNumber() const {
			return const_cast<const double&>(const_cast<Value*>(this)->asNumber());
		}
		const int64_t& asInteger() const {
			return const_cast<const int64_t&>(const_cast<Value*>(this)->asInteger());
		}
		const std::string& asString() const {
			return const_cast<const std::string&>(const_cast<Value*>(this)->asString());
		}
		const std::vector<Value>& asArray() const {
			return const_cast<const std::vector<Value>&>(const_cast<Value*>(this)->asArray());
		}
		const Object& asObject() const {
			return const_cast<const Object&>(const_cast<Value*>(this)->asObject());
		}

		bool isNull() const { return getType() == Type::Null; }
		bool isBool() const { return getType() == Type::Bool; }
		bool isNumber() const { return getType() == Type::Number; }
		bool isInteger() const { return getType() == Type::Integer; }
		bool isString() const { return getType() == Type::String; }
		bool isArray() const { return getType() == Type::Array; }
		bool isObject() const { return getType() == Type::Object; }

        std::string stringify(size_t indent = 0) const {
			switch (getType()) {
			case Type::Array: {
				const auto& arr = this->asArray();
				if (arr.empty()) return std::string(indent, ' ') + "[]";
				std::string result = std::string(indent, ' ') + "[\n" +
					arr[0].stringify(indent + 2);
				
				for (size_t i = 1; i < arr.size(); ++i) {
					result += ",\n" + arr[i].stringify(indent + 2);
				}
				result += "\n" + std::string(indent, ' ') + "]";
				return result;
			}
			case Type::Object: {
				const auto& map = this->asObject();
				std::string result = std::string(indent, ' ') + "{\n";				
				for (const auto& [key, val] : map) {
					if (val.getType() == Type::Array || val.getType() == Type::Object)
						result += std::string(indent + 2, ' ') + "\"" + key + "\":\n"
						+ val.stringify(indent + 2) + ",\n";
					else result += std::string(indent + 2, ' ') + "\"" + key + "\": "
						+ val.stringify() + ",\n";
				}
				if (!map.empty())
				{
					result.pop_back();
					result.pop_back();
					result += "\n";
				}
				result += std::string(indent, ' ') + "}";
				return result;
			}				
			case Type::String:
				return std::string(indent, ' ') + "\"" + *std::get<std::string*>(m_value) + "\"";
			case Type::Bool:
				return std::string(indent, ' ') + (std::get<bool>(m_value) ? "true" : "false");
			case Type::Integer:
				return std::string(indent, ' ') + std::to_string(std::get<int64_t>(m_value));
			case Type::Number:
				return std::string(indent, ' ') + std::to_string(std::get<double>(m_value));
			case Type::Null:
				return std::string(indent, ' ') + "null";
			default:
				throw std::runtime_error("Unknown type");
			}
		}

		std::string stringifyLean(size_t indent = 0) const {
			switch (getType()) {
			case Type::Array: {
				const auto& arr = this->asArray();
				if (arr.empty()) return std::string(indent, ' ') + "[]";
				std::string result = std::string(indent, ' ') + "[" +
					arr[0].stringify(indent + 2);

				for (size_t i = 1; i < arr.size(); ++i) {
					result += "," + arr[i].stringify(indent + 2);
				}
				result += std::string(indent, ' ') + "]";
				return result;
			}
			case Type::Object: {
				const auto& map = this->asObject();
				std::string result = std::string(indent, ' ') + "{";
				for (const auto& [key, val] : map) {
					if (val.getType() == Type::Array || val.getType() == Type::Object)
						result += std::string(indent + 2, ' ') + "\"" + key + "\":"
						+ val.stringify(indent + 2) + ",";
					else result += std::string(indent + 2, ' ') + "\"" + key + "\": "
						+ val.stringify() + ",";
				}
				if (!map.empty())
					result.pop_back();
				result += std::string(indent, ' ') + "}";
				return result;
			}
			case Type::String:
				return std::string(indent, ' ') + "\"" + *std::get<std::string*>(m_value) + "\"";
			case Type::Bool:
				return std::string(indent, ' ') + (std::get<bool>(m_value) ? "true" : "false");
			case Type::Integer:
				return std::string(indent, ' ') + std::to_string(std::get<int64_t>(m_value));
			case Type::Number:
				return std::string(indent, ' ') + std::to_string(std::get<double>(m_value));
			case Type::Null:
				return std::string(indent, ' ') + "null";
			default:
				throw std::runtime_error("Unknown type");
			}
		}

		static auto parse(std::string_view input) {
			return ContainerParser<Value>::parse(input);
		}

		template<Container C>
		static auto parse(C& input) {
			return ContainerParser<Value>::parse(input);
		}

		template<Stream S>
		static auto parse(S& input) {
			return StreamParser<Value>::parse(input);
		}

		static auto fromFile(std::string_view path) {
			std::ifstream file(path.data(), std::ios::in);
			return StreamParser<Value>::parse(file);
		}

		// Strict parser follows the json spec exactly, no comment, trailing comma or multiple root parsing
		// Use when perfomance matters more than utility
		static auto parseStrict(std::string_view input) {
			return StrictContainerParser<Value>::parse(input);
		}

		// Strict parser follows the json spec exactly, no comment, trailing comma or multiple root parsing
		// Use when perfomance matters more than utility
		template<Container C>
		static auto parseStrict(C& input) {
			return StrictContainerParser<Value>::parse(input);
		}

		// Strict parser follows the json spec exactly, no comment, trailing comma or multiple root parsing
		// Use when perfomance matters more than utility
		template<Stream S>
		static auto parseStrict(S& input) {
			return StrictStreamParser<Value>::parse(input);
		}

		// Strict parser follows the json spec exactly, no comment, trailing comma or multiple root parsing
		// Use when perfomance matters more than utility
		static auto fromFileStrict(std::string_view path) {
			std::ifstream file(path.data(), std::ios::in);
			return StrictStreamParser<Value>::parse(file);
		}

		bool operator==(const Value& other) const {
			switch (getType()) {
			case Type::Array:
				return asArray() == other.asArray();
			case Type::Object:
				return asObject() == other.asObject();
			case Type::String:
				return asString() == other.asString();
			default:
				return m_value == other.m_value;
			}
		}

		bool operator!=(const Value& other) const {
			return !(*this == other);
		}

		// enum class Type
		// {			
        //     Array,
		// 	Object,
		// 	String,
		// 	Bool,
		// 	Integer,
		// 	Number,
		// 	Null
		// };

		bool operator<(const Value& other) const {
			JSON_VERIFY(getType() == other.getType(), "Comparing Values containing different types");
			switch (getType()) {
			case Type::Array:
				return asArray() < other.asArray();
			case Type::Object:
				JSON_VERIFY(false, "Object does not have operator < defined");
			case Type::String:
				return asString() < other.asString();
			case Type::Bool:
				return asBool() < other.asBool();
			case Type::Integer:
				return asInteger() < other.asInteger();
			case Type::Number:
				return asNumber() < other.asNumber();
			case Type::Null:
				return false;
			default:
				return false;
			}
		}

		bool operator>(const Value& other) const {
			JSON_VERIFY(getType() == other.getType(), "Comparing Values containing different types");
			switch (getType()) {
			case Type::Array:
				return asArray() > other.asArray();
			case Type::Object:
				JSON_VERIFY(false, "Object does not have operator > defined");
			case Type::String:
				return asString() > other.asString();
			case Type::Bool:
				return asBool() > other.asBool();
			case Type::Integer:
				return asInteger() > other.asInteger();
			case Type::Number:
				return asNumber() > other.asNumber();
			case Type::Null:
				return false;
			default:
				return false;
			}
		}

		bool operator<=(const Value& other) const {
			JSON_VERIFY(getType() == other.getType(), "Comparing Values containing different types");
			switch (getType()) {
			case Type::Array:
				return asArray() <= other.asArray();
			case Type::Object:
				JSON_VERIFY(false, "Object does not have operator <= defined");
			case Type::String:
				return asString() <= other.asString();
			case Type::Bool:
				return asBool() <= other.asBool();
			case Type::Integer:
				return asInteger() <= other.asInteger();
			case Type::Number:
				return asNumber() <= other.asNumber();
			case Type::Null:
				return false;
			default:
				return false;
			}
		}

		bool operator>=(const Value& other) const {
			JSON_VERIFY(getType() == other.getType(), "Comparing Values containing different types");
			switch (getType()) {
			case Type::Array:
				return asArray() >= other.asArray();
			case Type::Object:
				JSON_VERIFY(false, "Object does not have operator >= defined");
			case Type::String:
				return asString() >= other.asString();
			case Type::Bool:
				return asBool() >= other.asBool();
			case Type::Integer:
				return asInteger() >= other.asInteger();
			case Type::Number:
				return asNumber() >= other.asNumber();
			case Type::Null:
				return false;
			default:
				return false;
			}
		}
	};

    namespace Detail
    {
        template<>
        struct EnumToTypeTrait<Value::Type::Array> {
            using Type = std::vector<Value>*;
        };

        template<>
        struct EnumToTypeTrait<Value::Type::Bool> {
            using Type = bool;
        };

        template<>
        struct EnumToTypeTrait<Value::Type::Integer> {
            using Type = int64_t;
        };

        template<>
        struct EnumToTypeTrait<Value::Type::Number> {
            using Type = double;
        };

        template<>
        struct EnumToTypeTrait<Value::Type::Object> {
            using Type = Value::Object*;
        };

        template<>
        struct EnumToTypeTrait<Value::Type::String> {
            using Type = std::string*;
        };

        template<>
        struct EnumToTypeTrait<Value::Type::Null> {
            using Type = std::nullptr_t;
        };
    }
}

