#pragma once
#include <stdint.h>
#include <array>
#include <stdexcept>
#include <memory>
#include <string>
#include <algorithm>

namespace Json
{
    class Parser
    {
    private:
        template<typename Container>
        static inline void handleEscapedChar(const Container& input, size_t& i, std::string& string) {
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
                i += 4;
                if(i >= input.size()) throw std::runtime_error("Invalid unicode escape");
                
                std::string hex;
                hex.append(&input[i - 3], 4);
                
                unsigned int codepoint = std::stoul(hex, nullptr, 16);
                if (codepoint <= 0x7F) {
                    string.push_back(static_cast<char>(codepoint));
                } else {
                    if (codepoint <= 0x7FF) {
                        string.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
                        string.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                    } else {
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

        template<typename Container>
        static inline void skipComment(const Container& input, size_t& i) {
            if (i >= input.size()) throw std::runtime_error("Invalid comment syntax");
            char c = input[i];
            ++i;
            if (c == lineComment) {
                for (; i < input.size() && input[i] != '\n'; ++i);
                return;
            } else if (c == blockCommentStart) {
                for (; i < input.size(); ++i) {
                    if (input[i] == blockCommentEnd && ++i < input.size() && input[i] == commentStart) {
                        return;
                    }
                }
                throw std::runtime_error("Endless block comment");
            }
            throw std::runtime_error("Invalid comment syntax");
        }

        template<typename Container>
        static inline bool skipToNextValue(const Container& input, size_t& i, uint8_t end) {
            char c;
            for (; i < input.size(); ++i) {
                c = input[i];
                if (c == commentStart)
                    skipComment(input, ++i);
                else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c) != whitespaceCharacters.end())
                    continue;
                else if (c == end)
                    return true;
                else return false;
            }
            throw std::runtime_error("Could not find next value");
        }

        template<typename Container>
        static inline bool skipToNextSeparator(const Container& input, size_t& i, uint8_t separator, uint8_t end) {
            char c;
            for (; i < input.size(); ++i) {
                c = input[i];
                if (c == commentStart)
                    skipComment(input, ++i);
                else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c) != whitespaceCharacters.end())
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
        static void skipToNextRootSeparator(const Container& input, size_t& i) {
            char c;
            for (; i < input.size(); ++i) {
                c = input[i];
                if (c == rootSeparator) {
                    return;
                } else if (c == commentStart)
                    skipComment(input, ++i);
                else if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c) == whitespaceCharacters.end())
                    throw std::runtime_error("Non whitespace character found");
            }
        }

        template<typename Container>
        static std::unique_ptr<Value> parseNumber(const Container& input, size_t& i) {
            std::string string;
            char c;
            for (; i < input.size(); ++i) {
                c = input[i];
                if (std::find(whitespaceCharacters.begin(), whitespaceCharacters.end(), c) != whitespaceCharacters.end() || 
                    c == valueSeparator || c == endObject || c == endArray || c == commentStart)
                    return std::make_unique<Number>(std::stod(string));
                string.push_back(c);
            }
            return std::make_unique<Number>(std::stod(string));
        }

        template<typename Container>
        static std::unique_ptr<Value> parseString(const Container& input, size_t& i) {
            std::string string;
            char c;
            for (; i < input.size(); ++i) {
                c = input[i];
                if (c == stringEnd) {
                    ++i;
                    return std::make_unique<String>(std::move(string));
                } else if (c == escapedCharStart)
                    handleEscapedChar(input, ++i, string);
                else string.push_back(c);
            }
            throw std::runtime_error("Invalid string syntax");
        }

        template<typename Container>
        static std::unique_ptr<Value> parseBoolTrue(const Container& input, size_t& i) {
            for (size_t j = 1; j < trueLiteral.size() && i < input.size(); ++j, ++i) {
                if (input[i] != trueLiteral[j]) {
                    throw std::runtime_error("Invalid bool true literal");
                }
            }
            return std::make_unique<Bool>(true);
        }

        template<typename Container>
        static std::unique_ptr<Value> parseBoolFalse(const Container& input, size_t& i) {
            for (size_t j = 1; j < falseLiteral.size() && i < input.size(); ++j, ++i) {
                if (input[i] != falseLiteral[j]) {
                    throw std::runtime_error("Invalid bool false literal");
                }
            }
            return std::make_unique<Bool>(false);
        }

        template<typename Container>
        static std::unique_ptr<Value> parseNull(const Container& input, size_t& i) {
            for (size_t j = 1; j < nullLiteral.size() && i < input.size(); ++j, ++i) {
                if (input[i] != nullLiteral[j]) {
                    throw std::runtime_error("Invalid null literal");
                }
            }
            return std::make_unique<Null>();
        }

        template<typename Container>
        static std::unique_ptr<Value> parseValue(const Container& input, size_t& i);

        template<typename Container>
        static std::unique_ptr<Value> parseArray(const Container& input, size_t& i);

        template<typename Container>
        static std::unique_ptr<Value> parseObject(const Container& input, size_t& i);

        template<typename Container>
        static std::unique_ptr<Value> parseValueForPair(const Container& input, size_t& i);

    public:
        template<typename Container>
        static Document parse(const Container& input);
    };
}