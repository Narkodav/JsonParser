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

//#include "SIMDFunctions.h"

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
		static inline size_t skipComment(Container& input, size_t i)
		{
			++i;
			if (i >= input.size()) throw std::runtime_error("Invalid comment syntax");
			char c = input[i];
			++i;
			if (c == lineComment) {
				for (; i < input.size() && input[i] != '\n'; ++i); // skip to end of line
				return i;
			}
			else if (c == blockCommentStart) {
				for (; i < input.size(); ++i)
				{
					if (input[i] == blockCommentEnd && ++i < input.size() && input[i] == commentStart) {
						return i;
					}
				}
				throw std::runtime_error("Endless block comment");
			}
			throw std::runtime_error("Invalid comment syntax");
		}

		//skips to the next significant char
		template<typename Container>
		static inline size_t skipWhitespace(Container& input, size_t i)
		{
			char c;
			for(; i < input.size(); ++i)
			{
				c = input[i];
				if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c) {
					continue;
				}
				else if (c == commentStart) {
					i = skipComment(input, i);
					continue;
				}
				return i;
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
		static inline std::string parseString(Container& input, size_t& i)
		{
			std::string string;
			++i;
			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (c == stringEnd)
				{
					++i;
					return string;
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
			++i;
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
			++i;
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
			++i;
			for (size_t j = 1; j < nullLiteral.size() && i < input.size(); ++j, ++i) {
				if (input[i] != nullLiteral[j]) {
					throw std::runtime_error("Invalid null literal");
				}
			}
			return std::make_unique<Null>();
		}

		template<typename Container>
		static std::unique_ptr<Value> parseArray(Container& input, size_t& i)
		{
			std::unique_ptr<Array> array = std::make_unique<Array>();
			for (; i < input.size();)
			{
				i = skipWhitespace(input, ++i);
				if (input[i] == endArray)
				{
					++i;
					return array;
				}
				array->m_values.emplace_back(std::move(parseValue(input, i)));
				i = skipWhitespace(input, i);
				if (i == input.size())
					throw std::runtime_error("Endless array");
				else if (input[i] != valueSeparator)
				{
					if (input[i] == endArray)
					{
						++i;
						return array;
					}
					else throw std::runtime_error("No value separator after array value");
				}
			}
			throw std::runtime_error("Endless array");
		}

		template<typename Container>
		static std::unique_ptr<Value> parseObject(Container& input, size_t& i)
		{
			std::unique_ptr<Object> object = std::make_unique<Object>();
			char c;
			for (; i < input.size();)
			{			
				i = skipWhitespace(input, ++i);
				c = input[i];
				if (c == stringStart) {
					std::string name = parseString(input, i);
					if (object->m_values.find(name) != object->m_values.end())
						throw std::runtime_error(std::string("Duplicate key: ") + name);
					i = skipWhitespace(input, i);
					if (i == input.size())
						throw std::runtime_error("Endless object");
					else if (input[i] != nameSeparator)
						throw std::runtime_error("No name separator after object key");

					i = skipWhitespace(input, ++i);
					object->m_values.emplace(
						std::move(name), std::move(parseValue(input, i)));
					i = skipWhitespace(input, i);
					if (i == input.size())
						throw std::runtime_error("Endless object");
					else if (input[i] != valueSeparator)
					{
						if (input[i] == endObject)
						{
							++i;
							return object;
						}
						else throw std::runtime_error("No value separator after object value");
					}
				}
				else if (c == endObject)
				{
					++i;
					return object;
				}
				else {
					throw std::runtime_error("Invalid object syntax: " + std::to_string(c));
				}				
			}
			throw std::runtime_error("Endless object");
		};

		template<typename Container>
		static std::unique_ptr<Value> parseValue(Container& input, size_t& i)
		{
			char c = input[i];
			if (c == beginObject) {
				return parseObject(input, i);
			}
			else if (c == beginArray) {
				return parseArray(input, i);
			}
			else if (c == stringStart) {
				return std::make_unique<String>(parseString(input, i));
			}
			else if (c == trueLiteral[0]) {
				return parseBoolTrue(input, i);
			}
			else if (c == falseLiteral[0]) {
				return parseBoolFalse(input, i);
			}
			else if (c == nullLiteral[0]) {
				return parseNull(input, i);
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

	public:
		template<typename Container>
		static Document parse(Container& input)
		{
			Document document;
			try {
				for (size_t i = 0; i < input.size();)
				{
					i = skipWhitespace(input, i);
					if (i == input.size())
						break;
					document.m_roots.emplace_back(std::move(parseValue(input, i)));
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