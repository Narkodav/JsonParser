#pragma once
#include <stdint.h>
#include <array>
#include <stdexcept>
#include <memory>
#include <string>
#include <algorithm>
#include <fstream>

#include "MappedFile.h"

#include "Document.h"

#include "ValueTypes/Array.h"
#include "ValueTypes/Bool.h"
#include "ValueTypes/Null.h"
#include "ValueTypes/Number.h"
#include "ValueTypes/Object.h"
#include "ValueTypes/String.h"

namespace Json
{
	class Parser
	{
	public:
		static inline const uint8_t beginArray = '[';
		static inline const uint8_t endArray = ']';
		static inline const uint8_t beginObject = '{';
		static inline const uint8_t endObject = '}';
		static inline const uint8_t nameSeparator = ':';
		static inline const uint8_t valueSeparator = ',';

		static inline const std::array<uint8_t, 4> whitespaceCharacters = { ' ', '\t', '\r', '\n' };
		static inline const uint8_t rootSeparator = '\n';

		static inline const uint8_t commentStart = '/';
		static inline const uint8_t lineComment = '/';
		static inline const uint8_t blockCommentStart = '*';
		static inline const uint8_t blockCommentEnd = '*';

		static inline const std::string nullLiteral = "null";
		static inline const std::string trueLiteral = "true";
		static inline const std::string falseLiteral = "false";
		static inline const uint8_t decimalSeparator = '.';
		static inline const std::array<uint8_t, 15> numberCharacters =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '-', 'e', 'E', decimalSeparator };
		static inline const std::array<uint8_t, 12> numberStartCharacters =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '-' };

		static inline const uint8_t stringStart = '\"';
		static inline const uint8_t stringEnd = '\"';

		static inline const uint8_t escapedCharStart = '\\';
		//static inline const std::array<uint8_t, 15> escapeChars =
		//{ '"', '\\', '/', 'b', 'f', 'n', 'r', 't', 'u' };

	private:
		template<typename Container>
		static inline void handleEscapedChar(Container& input, size_t& i, std::string& string)
		{
			if (i >= input.size()) throw std::runtime_error("Unterminated escape sequence");
			char escaped = input[i];
			switch (escaped) {
			case '"':  string.push_back('"'); break;
			case '\\': string.push_back('\\'); break;
			case '/':  string.push_back('/'); break;
			case 'b':  string.push_back('\b'); break;
			case 'f':  string.push_back('\f'); break;
			case 'n':  string.push_back('\n'); break;
			case 'r':  string.push_back('\r'); break;
			case 't':  string.push_back('\t'); break;
			case 'u': {
				// Unicode escape \uXXXX
				i += 4;
				if (i >= input.size())
					throw std::runtime_error("Invalid unicode escape");

				std::string hex;
				hex.append(&input[i - 3], 4);

				// Convert hex to unicode
				unsigned int codepoint = std::stoul(hex, nullptr, 16);
				if (codepoint <= 0x7F) {
					string.push_back(static_cast<char>(codepoint));
				}
				else {
					// Convert Unicode codepoint to UTF-8
					if (codepoint <= 0x7FF) {
						// 2-byte UTF-8
						string.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
						string.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
					}
					else {
						// 3-byte UTF-8 (covers BMP)
						string.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
						string.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
						string.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
					}
				}
				break;
			}
			default:
				throw std::runtime_error(std::string("Invalid escape sequence: \\") + escaped);
			}
		}

	private:

		template<typename Container>
		static inline void skipComment(Container& input, size_t& i)
		{
			if (i >= input.size()) throw std::runtime_error("Invalid comment syntax");
			char c = input[i];
			++i;
			if (c == lineComment) {
				for (; i < input.size() && input[i] != '\n'; ++i); // skip to end of line
				return;
			}
			else if (c == blockCommentStart) {
				for (; i < input.size(); ++i)
				{
					if (input[i] == blockCommentEnd && ++i < input.size() && input[i] == commentStart) {
						return;
					}
				}
				throw std::runtime_error("Endless block comment");
			}
			throw std::runtime_error("Invalid comment syntax");
		}

		template<typename Container>
		static inline bool skipToNextValue(Container& input, size_t& i, uint8_t end)
		{
			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (c == commentStart)
					skipComment(input, ++i);
				else if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c)
					continue;
				else if (c == end)
					return true;
				else return false;
			}
			throw std::runtime_error("Could not find next value");
		}

		template<typename Container>
		static inline bool skipToNextSeparator(Container& input, size_t& i,
		uint8_t separator, uint8_t end)
		{
			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (c == commentStart)
					skipComment(input, ++i);
				else if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c)
					continue;
				else if (c == separator)
					return false;
				else if (c == end)
					return true;
				else throw std::runtime_error("Invalid JSON syntax");
			}
			throw std::runtime_error("Could not find separator or end of object");
		}

		template<typename Container>
		static void skipToNextRootSeparator(Container& input, size_t& i)
		{
			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (c == rootSeparator)
				{
					return;
				}
				else if (c == commentStart)
					skipComment(input, ++i);
				else if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c)
					throw std::runtime_error("Non whitespace character found");
			}
		}

		template<typename Container>
		static inline std::unique_ptr<Value> parseNumber(Container& input, size_t& i)
		{
			std::string string;
			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c 
					|| c == valueSeparator || c == endObject
					|| c == endArray || c == commentStart)
					return std::make_unique<Number>(std::stod(string));
				string.push_back(c);
			}
			return std::make_unique<Number>(std::stod(string));
		}

		template<typename Container>
		static inline std::unique_ptr<Value> parseString(Container& input, size_t& i)
		{
			std::string string;
			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (c == stringEnd)
				{
					++i;
					return std::make_unique<String>(std::move(string));
				}
				else if (c == escapedCharStart)
					handleEscapedChar(input, ++i, string);
				else string.push_back(c);
			}
			throw std::runtime_error("Invalid string syntax");
		}

		template<typename Container>
		static inline std::unique_ptr<Value> parseBoolTrue(Container& input, size_t& i)
		{
			for (size_t j = 1; j < trueLiteral.size() && i < input.size(); ++j, ++i) {
				if (input[i] != trueLiteral[j]) {
					throw std::runtime_error("Invalid bool true literal");
				}
			}
			return std::make_unique<Bool>(true);
		}

		template<typename Container>
		static inline std::unique_ptr<Value> parseBoolFalse(Container& input, size_t& i)
		{
			for (size_t j = 1; j < falseLiteral.size() && i < input.size(); ++j, ++i) {
				if (input[i] != falseLiteral[j]) {
					throw std::runtime_error("Invalid bool false literal");
				}
			}
			return std::make_unique<Bool>(false);
		}

		template<typename Container>
		static inline std::unique_ptr<Value> parseNull(Container& input, size_t& i)
		{
			for (size_t j = 1; j < nullLiteral.size() && i < input.size(); ++j, ++i) {
				if (input[i] != nullLiteral[j]) {
					throw std::runtime_error("Invalid null literal");
				}
			}
			return std::make_unique<Null>();
		}

		template<typename Container>
		static std::unique_ptr<Value> parseValue(Container& input, size_t& i)
		{
			char c = input[i];
			if (c == stringStart) {
				return parseString(input, ++i);
			}
			else if (c == trueLiteral[0]) {
				return parseBoolTrue(input, ++i);
			}
			else if (c == falseLiteral[0]) {
				return parseBoolFalse(input, ++i);
			}
			else if (c == nullLiteral[0]) {
				return parseNull(input, ++i);
			}
			else if (c == numberStartCharacters[11]
				|| c == numberStartCharacters[10]
				|| (c >= numberStartCharacters[0]
					&& c <= numberStartCharacters[9])) {
				return parseNumber(input, i);
			}
			else {
				throw std::runtime_error(std::string("Invalid value syntax: ") + c);
			}
		}

		template<typename Container>
		static std::unique_ptr<Value> parseArray(Container& input, size_t& i)
		{
			std::unique_ptr<Array> array = std::make_unique<Array>();
			char c;
			for (; i < input.size(); ++i)
			{
				try
				{
					if (skipToNextValue(input, i, endArray))
						return array;
				}
				catch (const std::exception& e) {
					throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
				}
				c = input[i];
				if (c == beginObject) {
					array->m_values.emplace_back(std::move(parseObject(input, ++i)));
					++i;
					try
					{
						if (skipToNextSeparator(input, i, valueSeparator, endArray))
							return array;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
					}
				}
				else if (c == beginArray) {
					array->m_values.emplace_back(std::move(parseArray(input, ++i)));
					++i;
					try
					{
						if (skipToNextSeparator(input, i, valueSeparator, endArray))
							return array;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
					}
				}
				else if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c) {
					continue;
				}
				else if (c == commentStart) {
					skipComment(input, ++i);
				}
				else {
					array->m_values.emplace_back(std::move(parseValue(input, i)));
					try
					{
						if (skipToNextSeparator(input, i, valueSeparator, endArray))
							return array;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
					}
				}
			}
			throw std::runtime_error("Endless array");
		}

		template<typename Container>
		static std::unique_ptr<Value> parseValueForPair(Container& input, size_t& i)
		{
			if (skipToNextSeparator(input, i, nameSeparator, endObject) ||
				skipToNextValue(input, ++i, endObject))
				throw std::runtime_error("No value for name tag");
			char c = input[i];
			if (c == beginObject) {
				auto object = parseObject(input, ++i);
				++i;
				return object;
			}
			else if (c == beginArray) {
				auto array = parseArray(input, ++i);
				++i;
				return array;
			}
			else return parseValue(input, i);
		}

		template<typename Container>
		static std::unique_ptr<Value> parseObject(Container& input, size_t& i)
		{
			std::unique_ptr<Object> object = std::make_unique<Object>();

			char c;
			for (; i < input.size(); ++i)
			{
				if (skipToNextValue(input, i, endObject))
					return object;
				c = input[i];
				if (c == stringStart) {
					std::unique_ptr<Value> name = std::move(parseString(input, ++i));
					if (object->m_values.find(name->asString().data()) != object->m_values.end())
						throw std::runtime_error(std::string("Duplicate key: ") + name->asString().data());
					auto value = parseValueForPair(input, i);
					object->m_values.emplace(name->asString().data(), std::move(value));
					try
					{
						if (skipToNextSeparator(input, i, valueSeparator, endObject))
							return object;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid object syntax: ") + e.what());
					}
				}
				else if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c) {
					continue;
				}
				else if (c == commentStart) {
					skipComment(input, ++i);
				}
				else {
					throw std::runtime_error("Invalid object syntax: " + std::to_string(c));
				}
			}
			throw std::runtime_error("Endless object");
		};

	public:
		template<typename Container>
		static Document parse(Container& input)
		{
			Document document;
			try {
				char c;
				for (size_t i = 0; i < input.size(); ++i)
				{
					c = input[i];
					if (c == beginObject) {
						document.m_roots.emplace_back(std::move(parseObject(input, ++i)));
						++i;
						skipToNextRootSeparator(input, i);
					}
					else if (c == beginArray) {
						document.m_roots.emplace_back(std::move(parseArray(input, ++i)));
						++i;
						skipToNextRootSeparator(input, i);
					}
					else if (c == rootSeparator 
						|| whitespaceCharacters[0] == c
						|| whitespaceCharacters[1] == c
						|| whitespaceCharacters[2] == c
						|| whitespaceCharacters[3] == c) {
						continue;
					}
					else if (c == commentStart) {
						skipComment(input, ++i);
					}
					else {
						document.m_roots.emplace_back(std::move(parseValue(input, i)));
						skipToNextRootSeparator(input, i);
					}
				}
			}
			catch (const std::exception& e) {
				throw std::runtime_error(std::string("JSON parsing failed: ") + e.what());
			}
			return document;
		}
	
		static Document parseFile(const std::string& filename) {
			MappedFile file(filename.c_str());
			if (!file.isMapped())
				throw std::runtime_error("File not found: " + filename);
			return parse(file);
		}
	};
}