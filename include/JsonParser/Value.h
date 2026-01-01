#pragma once
#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <span>
#include <stdexcept>
#include <string_view>

#include "Parser.h"

namespace Json
{
	class Value
	{
	public:
		enum class Type
		{
			Array,
			Bool,
			Integer,
			Number,
			Object,
			String,
			Null
		};
		using Storage = std::variant<std::vector<Value>*, bool, int, double,
			std::unordered_map<std::string, Value>*,
			std::string*>;

	private:
		Type m_type;
		Storage m_value;

	public:
		Value() : m_type(Type::Null) {};

		template<typename T>
		Value(std::initializer_list<T> values) requires std::same_as<T, bool>
			|| std::same_as<T, double>
			|| std::same_as<T, int>
			|| std::same_as<T, std::string>
			|| std::same_as<T, std::string_view>
			|| std::convertible_to<T, const char*> {
			m_type = Type::Array;
			m_value = new std::vector<Value>();
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			arr.reserve(values.size());
			for (auto& val : values) {
				arr.emplace_back(std::move(val));
			}
		};
		Value(std::string_view value) {
			m_type = Type::String;
			m_value = new std::string(std::move(value));
		}
		template<size_t N>
		Value(const char (&value)[N]) {
			m_type = Type::String;
			m_value = new std::string(value);
		}
		Value(double value) {
			m_type = Type::Number;
			m_value = value;
		}
		Value(int value) {
			m_type = Type::Integer;
			m_value = value;
		}
		template<typename T>
		Value(T value) requires std::same_as<T, bool> {
			m_type = Type::Bool;
			m_value = value;
		}
		Value(std::initializer_list<std::pair<std::string, Value>> values) {
			m_type = Type::Object;
			m_value = new std::unordered_map<std::string, Value>();
			auto& map = *std::get<std::unordered_map<std::string, Value>*>(m_value);
			map.reserve(values.size());
			for (auto& pair : values) {
				map.emplace(pair.first, pair.second);
			}
		}
		Value(std::nullptr_t) {
			m_type = Type::Null;
		}
		Type getType() const { return m_type; }
		template<Type T>
		const auto& get() const {
			switch (m_type) {
			case T:
				return std::get<T>(m_value);
			default:
				throw std::runtime_error("Type mismatch");
			}
		}

		~Value() {
			switch (m_type) {
			case Type::Array:
				delete std::get<std::vector<Value>*>(m_value);
				break;
			case Type::Object:
				delete std::get<std::unordered_map<std::string, Value>*>(m_value);
				break;
			case Type::String:
				delete std::get<std::string*>(m_value);
				break;
			default:
				break;
			};
		}

		Value(const Value& other) {
			m_type = other.m_type;
			switch (m_type) {
			case Type::Array:
				m_value = new std::vector<Value>(*std::get<std::vector<Value>*>(other.m_value));
				break;
			case Type::Object:
				m_value = new std::unordered_map<std::string, Value>(*std::get<std::unordered_map<std::string, Value>*>(other.m_value));
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
				m_type = other.m_type;
				switch (m_type) {
				case Type::Array:
					m_value = new std::vector<Value>(*std::get<std::vector<Value>*>(other.m_value));
					break;
				case Type::Object:
					m_value = new std::unordered_map<std::string, Value>(*std::get<std::unordered_map<std::string, Value>*>(other.m_value));
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
			m_type = other.m_type;
			m_value = std::move(other.m_value);
			other.m_type = Type::Null;
		}
		Value& operator=(Value&& other) noexcept {
			if (this != &other) {
				this->~Value();
				m_type = other.m_type;
				m_value = std::move(other.m_value);
				other.m_type = Type::Null;
			}
			return *this;
		}

		static Value array(std::initializer_list<Value> values = {}) {
			Value val;
			val.m_type = Type::Array;
			val.m_value = new std::vector<Value>(std::move(values));
			return val;
		}

		static Value object(std::initializer_list<std::pair<std::string, Value>> values = {}) {
			Value val;
			val.m_type = Type::Object;
			val.m_value = new std::unordered_map<std::string, Value>();
			auto& map = *std::get<std::unordered_map<std::string, Value>*>(val.m_value);
			for (auto& pair : values) {
				map.emplace(pair);
			}
			return val;
		}

		std::string stringify(size_t indent = 0) const {
			switch (m_type) {
			case Type::Array: {
				std::string result = std::string(indent, ' ') + "[\n";
				const auto& arr = *std::get<std::vector<Value>*>(m_value);
				for (size_t i = 0; i < arr.size(); ++i) {
					result += arr[i].stringify(indent + 2) + ",\n";
				}
				result += std::string(indent, ' ') + "]";
				return result;
			}
			case Type::Object: {
				std::string result = std::string(indent, ' ') + "{\n";
				const auto& map = *std::get<std::unordered_map<std::string, Value>*>(m_value);
				for (const auto& [key, val] : map) {
					if (val.m_type == Type::Array || val.m_type == Type::Object)
						result += std::string(indent + 2, ' ') + "\"" + key + "\":\n"
						+ val.stringify(indent + 2) + ",\n";
					else result += std::string(indent + 2, ' ') + "\"" + key + "\": "
						+ val.stringify() + ",\n";
				}
				result += std::string(indent, ' ') + "}";
				return result;
			}				
			case Type::String:
				return std::string(indent, ' ') + "\"" + *std::get<std::string*>(m_value) + "\"";
			case Type::Bool:
				return std::string(indent, ' ') + (std::get<bool>(m_value) ? "true" : "false");
			case Type::Integer:
				return std::string(indent, ' ') + std::to_string(std::get<int>(m_value));
			case Type::Number:
				return std::string(indent, ' ') + std::to_string(std::get<double>(m_value));
			case Type::Null:
				return std::string(indent, ' ') + "null";
			default:
				throw std::runtime_error("Unknown type");
			}
		}

		void pushBack(const Value& value) {
			if (m_type != Type::Array) {
				throw std::runtime_error("Type mismatch");
			}
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			arr.push_back(value);
		}

		void emplaceBack(Value&& value) {
			if (m_type != Type::Array) {
				throw std::runtime_error("Type mismatch");
			}
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			arr.emplace_back(std::move(value));
		}

		Value& operator[](const std::string& key) {
			if (m_type != Type::Object) {
				throw std::runtime_error("Type mismatch");
			}
			auto& map = *std::get<std::unordered_map<std::string, Value>*>(m_value);
			return map[key];
		}
		Value& operator[](size_t index) {
			if (m_type != Type::Array) {
				throw std::runtime_error("Type mismatch");
			}
			auto& arr = *std::get<std::vector<Value>*>(m_value);
			return arr[index];
		}

		bool& asBool() {
			if (m_type != Type::Bool) {
				throw std::runtime_error("Type mismatch");
			}
			return std::get<bool>(m_value);
		}
		double& asNumber() {
			if (m_type != Type::Number) {
				throw std::runtime_error("Type mismatch");
			}
			return std::get<double>(m_value);
		}
		int& asInteger() {
			if (m_type != Type::Integer) {
				throw std::runtime_error("Type mismatch");
			}
			return std::get<int>(m_value);
		}
		std::string& asString() {
			if (m_type != Type::String) {
				throw std::runtime_error("Type mismatch");
			}
			return *std::get<std::string*>(m_value);
		}
		std::vector<Value>& asArray() {
			if (m_type != Type::Array) {
				throw std::runtime_error("Type mismatch");
			}
			return *std::get<std::vector<Value>*>(m_value);
		}
		std::unordered_map<std::string, Value>& asObject() {
			if (m_type != Type::Object) {
				throw std::runtime_error("Type mismatch");
			}
			return *std::get<std::unordered_map<std::string, Value>*>(m_value);
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
		const int& asInteger() const {
			return const_cast<const int&>(const_cast<Value*>(this)->asInteger());
		}
		const std::string& asString() const {
			return const_cast<const std::string&>(const_cast<Value*>(this)->asString());
		}
		const std::vector<Value>& asArray() const {
			return const_cast<const std::vector<Value>&>(const_cast<Value*>(this)->asArray());
		}
		const std::unordered_map<std::string, Value>& asObject() const {
			return const_cast<const std::unordered_map<std::string, Value>&>(const_cast<Value*>(this)->asObject());
		}

		bool isNull() const { return m_type == Type::Null; }
		bool isBool() const { return m_type == Type::Bool; }
		bool isNumber() const { return m_type == Type::Number; }
		bool isInteger() const { return m_type == Type::Integer; }
		bool isString() const { return m_type == Type::String; }
		bool isArray() const { return m_type == Type::Array; }
		bool isObject() const { return m_type == Type::Object; }

		static Value fromFile(const std::string& filename) {
			return Detail::Parser::parseFile(filename);
		}

		template<typename Container>
		static Value fromString(Container& input) {
			return Detail::Parser::parse(input);
		}
	};
}

