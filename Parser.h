#pragma once
#include <stdint.h>
#include <array>
#include <stdexcept>
#include <memory>
#include <string>
#include <algorithm>
#include <fstream>

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

	public:

		template<typename Stream>
		static inline void skipComment(Stream& input) {
			input.get(); // consume '/'
			char next = input.get();
			if (next == lineComment) {
				while (input.good() && input.get() != '\n'); // skip to end of line
			}
			else if (next == blockCommentStart) {
				while (input.good()) {
					if (input.get() == blockCommentEnd && input.peek() == commentStart) {
						input.get(); // consume '/'
						return;
					}
				}
				throw std::runtime_error("Endless block comment");
			}
			else {
				throw std::runtime_error("Invalid comment syntax");
			}
		}

		template<typename Stream>
		static inline bool skipToNextValue(Stream& input, uint8_t end) {
			char c;
			while (input.good()) {
				c = input.peek();
				if (c == commentStart)
					skipComment(input);
				else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
					!= whitespaceCharacters.end())
					input.get();
				else if (c == end)
				{
					input.get();
					return true;
				}
				else return false;
			}
			throw std::runtime_error("Could not find next value");
		}

		template<typename Stream>
		static inline bool skipToNextSeparator(Stream& input, uint8_t separator, uint8_t end) {
			char c;
			while (input.good()) {
				c = input.peek();
				if (c == commentStart)
					skipComment(input);
				else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
					!= whitespaceCharacters.end())
					input.get();
				else if (c == separator)
				{
					input.get();
					return false;
				}
				else if (c == end)
				{
					input.get();
					return true;
				}
				else throw std::runtime_error("Invalid JSON syntax");
			}
			throw std::runtime_error("Could not find separator or end of object");
		}

		template<typename Stream>
		static void skipToNextRootSeparator(Stream& input) {
			char c;
			while (input.good()) {
				c = input.peek();
				if(c == rootSeparator)
				{
					c = input.get();
					return;
				}
				else if (c == commentStart)
					skipComment(input);
				else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
					== whitespaceCharacters.end())
					throw std::runtime_error("Non whitespace character found");
				input.get();
			}
		}

		/*		template<typename Stream>
				static void skipToEnd(Stream& input, uint8_t end) {
					char c;
					while (input.good()) {
						c = input.peek();
						if(c == end)
						{
							c = input.get();
							return;
						}
						else if (c == commentStart)
							skipComment(input);
						else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
							== whitespaceCharacters.end())
							throw std::runtime_error("Non whitespace character found");
						input.get();
					}
					throw std::runtime_error("Endless object");
				}	*/

		template<typename Stream>
		static inline std::unique_ptr<Value> parseNumber(Stream& input) {
			std::string string;
			char c = input.peek();
			while (input.good())
			{
				if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
					!= whitespaceCharacters.end() || c == valueSeparator || c == endObject
					|| c == endArray || c == commentStart)
					return std::make_unique<Number>(std::stod(string));
				string.push_back(input.get());
				c = input.peek();
			}
			if (!input.good())
				return std::make_unique<Number>(std::stod(string));
			throw std::runtime_error("Invalid number syntax");
		}

		template<typename Stream>
		static inline std::unique_ptr<Value> parseString(Stream& input) {
			std::string string;
			input.get(); //consume stringStart
			while (input.good())
			{
				if (input.peek() == stringEnd)
				{
					input.get();
					return std::make_unique<String>(std::move(string));
				}
				string.push_back(input.get());
			}
			throw std::runtime_error("Invalid string syntax");
		}

		template<typename Stream>
		static inline std::unique_ptr<Value> parseBoolTrue(Stream& input) {
			for (size_t i = 0; i < trueLiteral.size(); ++i) {
				if (input.get() != trueLiteral[i]) {
					throw std::runtime_error("Invalid bool true literal");
				}
			}
			return std::make_unique<Bool>(true);
		}

		template<typename Stream>
		static inline std::unique_ptr<Value> parseBoolFalse(Stream& input) {
			for (size_t i = 0; i < falseLiteral.size(); ++i) {
				if (input.get() != falseLiteral[i]) {
					throw std::runtime_error("Invalid bool false literal");
				}
			}
			return std::make_unique<Bool>(false);
		}

		template<typename Stream>
		static inline std::unique_ptr<Value> parseNull(Stream& input) {
			for (size_t i = 0; i < nullLiteral.size(); ++i) {
				if (input.get() != nullLiteral[i]) {
					throw std::runtime_error("Invalid null literal");
				}
			}
			return std::make_unique<Null>();
		}

		template<typename Stream>
		static std::unique_ptr<Value> parseValue(Stream& input) {
			char c = input.peek();
			if (c == stringStart) {
				return parseString(input);
			}
			else if (c == trueLiteral[0]) {
				return parseBoolTrue(input);
			}
			else if (c == falseLiteral[0]) {
				return parseBoolFalse(input);
			}
			else if (c == nullLiteral[0]) {
				return parseNull(input);
			}
			else if (c == numberStartCharacters[11]
				|| c == numberStartCharacters[10]
				|| (c >= numberStartCharacters[0]
					&& c <= numberStartCharacters[9])) {
				return parseNumber(input);
			}
			else {
				throw std::runtime_error(std::string("Invalid value syntax: ") + c);
			}
		}

		template<typename Stream>
		static std::unique_ptr<Value> parseArray(Stream& input) {
			std::unique_ptr<Array> array = std::make_unique<Array>();
			input.get(); //consume beginArray
			if (skipToNextValue(input, endArray))
				return array;
			while (input.good()) {
				char c = input.peek();
				if (c == beginObject) {
					array->m_values.emplace_back(std::move(parseObject(input)));
					try
					{
						if (skipToNextSeparator(input, valueSeparator, endArray) ||
							skipToNextValue(input, endArray))
							return array;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
					}
				}
				else if (c == beginArray) {
					array->m_values.emplace_back(std::move(parseArray(input)));
					try
					{
						if (skipToNextSeparator(input, valueSeparator, endArray) ||
							skipToNextValue(input, endArray))
							return array;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
					}
				}
				else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
					!= whitespaceCharacters.end()) {
					input.get();
					continue;
				}
				else if (c == commentStart) {
					skipComment(input);
				}
				else {
					array->m_values.emplace_back(std::move(parseValue(input)));
					try
					{
						if (skipToNextSeparator(input, valueSeparator, endArray) ||
							skipToNextValue(input, endArray))
							return array;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
					}
				}
			}
			throw std::runtime_error("Endless array");
		};

		template<typename Stream>
		static std::unique_ptr<Value> parseValueForPair(Stream& input) {
			if (skipToNextSeparator(input, nameSeparator, endObject) ||
				skipToNextValue(input, endObject))
				throw std::runtime_error("No value for name tag");
			char c = input.peek();
			if (c == beginObject) {
				return parseObject(input);
			}
			else if (c == beginArray) {
				return parseArray(input);
			}
			else return parseValue(input);			
		}

		template<typename Stream>
		static std::unique_ptr<Value> parseObject(Stream& input) {
			std::unique_ptr<Object> object = std::make_unique<Object>();
			input.get(); //consume beginObject
			if (skipToNextValue(input, endObject))
				return object;
			while (input.good()) {
				char c = input.peek();
				if (c == stringStart) {
					std::unique_ptr<Value> name = std::move(parseString(input));
					if (object->m_values.find(name->asString().data()) != object->m_values.end())
						throw std::runtime_error(std::string("Duplicate key: ") + name->asString().data());
					auto value = parseValueForPair(input);
					object->m_values.emplace(name->asString().data(), std::move(value));
					try
					{
						if (skipToNextSeparator(input, valueSeparator, endObject) ||
							skipToNextValue(input, endObject))
							return object;
					}
					catch (const std::exception& e) {
						throw std::runtime_error(std::string("Invalid array syntax: ") + e.what());
					}
				}
				else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
					!= whitespaceCharacters.end()) {
					input.get();
					continue;
				}
				else if (c == commentStart) {
					skipComment(input);
				}
				else {
					throw std::runtime_error("Invalid object syntax: " + std::to_string(c));
				}
			}
			throw std::runtime_error("Endless object");
		};

		template<typename Stream>
		static Document parse(Stream& input) {
			Document document;
			try {
				while (input.good()) {
					char c = input.peek();
					if (c == beginObject) {
						document.m_roots.emplace_back(std::move(parseObject(input)));
						c = input.get();
						if (c != rootSeparator)
						{
							try
							{
								skipToNextRootSeparator(input);
							}
							catch (const std::exception& e) {
								throw std::runtime_error(std::string("Invalid root separation syntax: ") + e.what());
							}
						}
					}
					else if (c == beginArray) {
						document.m_roots.emplace_back(std::move(parseArray(input)));
						c = input.get();
						if (c != rootSeparator)
						{
							try
							{
								skipToNextRootSeparator(input);
							}
							catch (const std::exception& e) {
								throw std::runtime_error(std::string("Invalid root separation syntax: ") + e.what());
							}
						}
					}
					else if (c == rootSeparator) {
						input.get();
						continue;
					}
					else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c)
						!= whitespaceCharacters.end()) {
						input.get();
						continue;
					}
					else if (c == commentStart) {
						skipComment(input);
					}
					else {
						document.m_roots.emplace_back(std::move(parseValue(input)));
						c = input.get();
						if (c != rootSeparator)
						{
							try
							{
								skipToNextRootSeparator(input);
							}
							catch (const std::exception& e) {
								throw std::runtime_error(std::string("Invalid root separation syntax: ") + e.what());
							}
						}
					}
				}
			}
			catch (const std::exception& e) {
				throw std::runtime_error(std::string("JSON parsing failed: ") + e.what());
			}
			return document;
		}

		static Document parseFile(const std::string& filename) {
			std::fstream file(filename, std::ios::in);
			if (!file.is_open())
				throw std::runtime_error("File not found: " + filename);
			return parse(file);
		}
	};
}