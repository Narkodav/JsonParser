#pragma once
#include <stdint.h>
#include <array>
#include <stdexcept>
#include <memory>
#include <string>
#include <algorithm>
#include <fstream>

//#include "MappedFile.h"

#include "SIMDUtils.h"

namespace Json::Detail
{
	class Value;

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

#ifdef HAS_AVX2
		template<typename Container>
		static inline size_t skipCommentSIMD32(const Container& input, size_t i) {

			//static const __m256i ws_slash = _mm256_set1_epi8('/');
			static const __m256i ws_star = _mm256_set1_epi8('*');
			static const __m256i ws_lf = _mm256_set1_epi8('\n');
			__m256i chunk;
			uint32_t mask;

			i += 2;
			if (i >= input.size())
				throw std::runtime_error("Invalid comment syntax");
			else if (input[i - 1] == '/')
			{
				while (i + 32 <= input.size()) {
					chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&input[i]));
					mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, ws_lf));
					if (mask != 0)
						return i + CTZ32(mask);
					i += 32;
				}
				for (; i < input.size() && input[i] != '\n'; ++i);
				return i;
			}
			else if (input[i - 1] == '*')
			{
				while (i + 32 <= input.size()) {
					chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&input[i]));
					mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, ws_star));
					if (mask != 0) {
						size_t j = i + CTZ32(mask) + 1;
						if (j >= input.size())
							throw std::runtime_error("Invalid comment syntax");
						else if (input[j] == '/')
							return j;
						i = j + 1;
						continue;
					}
					i += 32;
				}
				for (; i < input.size(); ++i)
				{
					if (input[i] == '*' && ++i < input.size() && input[i] == '/') {
						return i;
					}
				}
				throw std::runtime_error("Endless block comment");
			}
			throw std::runtime_error(std::string("Invalid comment syntax") + input[i]);
		};
#endif

#ifdef HAS_SSE2
		template<typename Container>
		static inline size_t skipCommentSIMD16(const Container& input, size_t i) {

			//static const __m256i ws_slash = _mm256_set1_epi8('/');
			static const __m128i ws_star = _mm_set1_epi8('*');
			static const __m128i ws_lf = _mm_set1_epi8('\n');
			__m128i chunk;
			uint16_t mask;

			i += 2;
			if (i >= input.size())
				throw std::runtime_error("Invalid comment syntax");
			char c = input[i - 1];
			if (c == '/')
			{
				while (i + 16 <= input.size()) {
					chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i]));
					mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, ws_lf));
					if (mask != 0)
						return i + CTZ16(mask);
					i += 16;
				}
				for (; i < input.size() && input[i] != '\n'; ++i);
				return i;
			}
			else if (c == '*')
			{
				while (i + 16 <= input.size()) {
					chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i]));
					mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, ws_star));
					if (mask != 0) {
						size_t j = i + CTZ16(mask) + 1;
						if (j >= input.size())
							throw std::runtime_error("Invalid comment syntax");
						else if (input[j] == '/')
							return j;
						i = j + 1;
						continue;
					}
					i += 16;
				}
				for (; i < input.size(); ++i)
				{
					if (input[i] == '*' && ++i < input.size() && input[i] == '/') {
						return i;
					}
				}
				throw std::runtime_error("Endless block comment");
			}
			throw std::runtime_error(std::string("Invalid comment syntax") + input[i]);
		};
#endif

		template<typename Container>
		static inline size_t skipCommentScalar(const Container& input, size_t i) {
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


#ifdef HAS_AVX2
		template<typename Container>
		static inline size_t skipWhitespaceSIMD32(const Container& input, size_t i) {
			const size_t size = input.size();

			static const __m256i ws_space = _mm256_set1_epi8(' ');
			static const __m256i ws_tab = _mm256_set1_epi8('\t');
			static const __m256i ws_cr = _mm256_set1_epi8('\r');
			static const __m256i ws_lf = _mm256_set1_epi8('\n');

			__m256i chunk;
			uint32_t mask;

			while (i + 32 <= size) {
				chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&input[i]));

				mask = ~_mm256_movemask_epi8(_mm256_or_si256(
					_mm256_or_si256(_mm256_cmpeq_epi8(chunk, ws_space), _mm256_cmpeq_epi8(chunk, ws_tab)),
					_mm256_or_si256(_mm256_cmpeq_epi8(chunk, ws_cr), _mm256_cmpeq_epi8(chunk, ws_lf))));

				if (mask != 0) {
					size_t j = i + CTZ32(mask);
					if (input[j] == '/')
					{
						i = skipCommentSIMD32(input, j) + 1;
						continue;
					}
					return j;
				}

				i += 32;
			}

			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c) {
					continue;
				}
				else if (c == commentStart) {
					i = skipCommentScalar(input, i);
					continue;
				}
				return i;
			}

			return i;
		};
#endif

#ifdef HAS_SSE2
		template<typename Container>
		static inline size_t skipWhitespaceSIMD16(const Container& input, size_t i) {
			const size_t size = input.size();

			static const __m128i ws_space = _mm_set1_epi8(' ');
			static const __m128i ws_tab = _mm_set1_epi8('\t');
			static const __m128i ws_cr = _mm_set1_epi8('\r');
			static const __m128i ws_lf = _mm_set1_epi8('\n');

			__m128i chunk;
			uint16_t mask;

			while (i + 16 <= size) {
				chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&input[i]));

				mask = ~_mm_movemask_epi8(_mm_or_si128(
					_mm_or_si128(_mm_cmpeq_epi8(chunk, ws_space), _mm_cmpeq_epi8(chunk, ws_tab)),
					_mm_or_si128(_mm_cmpeq_epi8(chunk, ws_cr), _mm_cmpeq_epi8(chunk, ws_lf))));

				if (mask != 0) {
					size_t j = i + CTZ16(mask);
					if (input[j] == '/')
					{
						i = skipCommentSIMD16(input, j) + 1;
						continue;
					}
					return j;
				}

				i += 16;
			}

			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c) {
					continue;
				}
				else if (c == commentStart) {
					i = skipCommentScalar(input, i);
					continue;
				}
				return i;
			}
			return i;
		};
#endif

		template<typename Container>
		static inline size_t skipWhitespaceScalar(const Container& input, size_t i) {
			char c;
			for (; i < input.size(); ++i)
			{
				c = input[i];
				if (whitespaceCharacters[0] == c
					|| whitespaceCharacters[1] == c
					|| whitespaceCharacters[2] == c
					|| whitespaceCharacters[3] == c) {
					continue;
				}
				else if (c == commentStart) {
					i = skipCommentScalar(input, i);
					continue;
				}
				return i;
			}
			return i;
		}


		template<typename Container>
		static inline size_t skipWhitespace(const Container& input, size_t i) {
#ifdef HAS_AVX2
			return skipWhitespaceSIMD16(input, i);
#elif defined(HAS_SSE2)
			return skipWhitespaceSIMD16(input, i);
#else 
			return skipWhitespaceScalar(input, i);
#endif
		}

		template<typename Container>
		static inline Value parseNumber(Container& input, size_t& i)
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
					return Value(std::stod(string));
				string.push_back(c);
			}
			return Value(std::stod(string));
		}

		template<typename Container>
		static inline std::string parseString(Container& input, size_t& i)
		{
			++i;
			size_t size = 0;

			size_t end = i;
			for (; end < input.size(); ++end, ++size) {
				if (input[end] == stringEnd) break;
				if (input[end] == escapedCharStart) ++end;
			}
			if (end >= input.size()) throw std::runtime_error("Invalid string syntax");
			
			std::string string;
			string.reserve(size);
			
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
		static inline Value parseBoolTrue(Container& input, size_t& i)
		{
			++i;
			for (size_t j = 1; j < trueLiteral.size() && i < input.size(); ++j, ++i) {
				if (input[i] != trueLiteral[j]) {
					throw std::runtime_error("Invalid bool true literal");
				}
			}
			return Value(true);
		}

		template<typename Container>
		static inline Value parseBoolFalse(Container& input, size_t& i)
		{
			++i;
			for (size_t j = 1; j < falseLiteral.size() && i < input.size(); ++j, ++i) {
				if (input[i] != falseLiteral[j]) {
					throw std::runtime_error("Invalid bool false literal");
				}
			}
			return Value(false);
		}

		template<typename Container>
		static inline Value parseNull(Container& input, size_t& i)
		{
			++i;
			for (size_t j = 1; j < nullLiteral.size() && i < input.size(); ++j, ++i) {
				if (input[i] != nullLiteral[j]) {
					throw std::runtime_error("Invalid null literal");
				}
			}
			return Value();
		}

		template<typename Container>
		static Value parseArray(Container& input, size_t& i)
		{
			Value value = Value::array();
			auto& array = value.asArray();
			for (; i < input.size();)
			{
				i = skipWhitespace(input, ++i);
				if (input[i] == endArray)
				{
					++i;
					return value;
				}
				array.emplace_back(std::move(parseValue(input, i)));
				i = skipWhitespace(input, i);
				if (i == input.size())
					throw std::runtime_error("Endless array");
				else if (input[i] != valueSeparator)
				{
					if (input[i] == endArray)
					{
						++i;
						return value;
					}
					else throw std::runtime_error("No value separator after array value");
				}
			}
			throw std::runtime_error("Endless array");
		}

		template<typename Container>
		static Value parseObject(Container& input, size_t& i)
		{
			Value value = Value::object();
			auto& object = value.asObject();
			char c;
			for (; i < input.size();)
			{			
				i = skipWhitespace(input, ++i);
				c = input[i];
				if (c == stringStart) {
					std::string name = parseString(input, i);
					if (object.find(name) != object.end())
						throw std::runtime_error(std::string("Duplicate key: ") + name);
					i = skipWhitespace(input, i);
					if (i == input.size())
						throw std::runtime_error("Endless object");
					else if (input[i] != nameSeparator)
						throw std::runtime_error("No name separator after object key");

					i = skipWhitespace(input, ++i);
					object.emplace(
						std::move(name), std::move(parseValue(input, i)));
					i = skipWhitespace(input, i);
					if (i == input.size())
						throw std::runtime_error("Endless object");
					else if (input[i] != valueSeparator)
					{
						if (input[i] == endObject)
						{
							++i;
							return value;
						}
						else throw std::runtime_error("No value separator after object value");
					}
				}
				else if (c == endObject)
				{
					++i;
					return value;
				}
				else {
					throw std::runtime_error("Invalid object syntax: " + std::to_string(c));
				}				
			}
			throw std::runtime_error("Endless object");
		};

		template<typename Container>
		static Value parseValue(Container& input, size_t& i)
		{
			char c = input[i];
			switch (c) {
			case beginObject:
				return parseObject(input, i);
			case beginArray:
				return parseArray(input, i);
			case stringStart:
				return Value(parseString(input, i));
			case trueLiteral[0]:
				return parseBoolTrue(input, i);
			case falseLiteral[0]:
				return parseBoolFalse(input, i);
			case nullLiteral[0]:
				return parseNull(input, i);
			default:
				if (c == numberStartCharacters[11]
					|| c == numberStartCharacters[10]
					|| (c >= numberStartCharacters[0]
						&& c <= numberStartCharacters[9]))
					return parseNumber(input, i);
				else {
					throw std::runtime_error(std::string("Invalid value syntax: ") + c);
				}
				break;
			}
			return Value();
		}

	public:
		template<typename Container>
		static Value parse(Container& input)
		{
			Value document = Value::array();
			try {
				for (size_t i = 0; i < input.size();)
				{
					i = skipWhitespace(input, i);
					if (i >= input.size())
						break;
					document.pushBack(std::move(parseValue(input, i)));
				}
			}
			catch (const std::exception& e) {
				throw std::runtime_error(std::string("JSON parsing failed: ") + e.what());
			}
			return document;
		}

		//static Value parseFile(const std::string& filename) {
		//	std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
		//	if (!file)
		//		throw std::runtime_error("File not found: " + filename);
		//	return parse(file);
		//}
	
		//static Value parseFile(const std::string& filename) {
		//	MappedFile file(filename.c_str());
		//	if (!file.isMapped())
		//		throw std::runtime_error("File not found: " + filename);
		//	return parse(file);
		//}
	};
}