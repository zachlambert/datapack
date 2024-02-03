#include "datapack/json.hpp"

namespace datapack {

JsonParser::JsonParser(const std::string& source):
    source(source),
    pos(0)
{
    states.push(EXPECT_VALUE);
}

std::optional<Token> JsonParser::next() {
    while (true) {
        int& state = states.top();

        if (pos == source.size()) {
            if (!(state & EXPECT_END) || (state & (IS_OBJECT | IS_ARRAY))) {
                throw ParseException("Not expecting the document end");
            }
            return std::nullopt;
        }

        const char c = source[pos];
        if (std::isspace(c)) {
            pos++;
            continue;
        }

        if (c == '{') {
            if (!(state | EXPECT_VALUE)) {
                throw ParseException("Unexpected character '{'");
            }
            pos++;
            states.push(IS_OBJECT | EXPECT_KEY | EXPECT_END);
            return ObjectBegin();
        }
        if (c == '[') {
            if (!(state & EXPECT_VALUE)) {
                throw ParseException("Unexpected character '['");
            }
            pos++;
            states.push(IS_ARRAY | EXPECT_VALUE | EXPECT_END);
            return ArrayBegin();
        }
        if (c == '}') {
            if (!(state & IS_OBJECT & EXPECT_END)) {
                throw ParseException("Unexpected character '}'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_END;
            if (new_state | IS_OBJECT) {
                new_state |= EXPECT_KEY;
            } else if (new_state | IS_ARRAY) {
                new_state |= EXPECT_VALUE;
            }

            return ObjectEnd();
        }
        if (c == ']') {
            if (!(state & IS_ARRAY & EXPECT_END)) {
                throw ParseException("Unexpected character ']'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_END;
            if (new_state | IS_OBJECT) {
                new_state |= EXPECT_KEY;
            } else if (new_state | IS_ARRAY) {
                new_state |= EXPECT_VALUE;
            }

            return ObjectEnd();
        }
        if (c == ',') {
            if (!(state & EXPECT_NEXT)) {
                throw ParseException("Unexpected character ','");
            }
            state &= ~EXPECT_NEXT;
            if (state & IS_OBJECT) {
                state |= EXPECT_KEY;
            } else if (state & IS_ARRAY) {
                state |= EXPECT_VALUE;
            } else {
                throw ParseException("Unexpected character ','");
            }
            pos++;
            continue;
        }

        if (c == '"' && (state & EXPECT_KEY)) {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == source.size()) {
                    throw ParseException("Key missing terminating '\"'");
                }
                const char c = source[pos];
                if (c == '"') {
                    break;
                }
                pos++;
            }
            std::size_t end = pos;
            pos++;

            while (true) {
                if (pos == source.size()) {
                    throw ParseException("Expected ':' following key");
                }
                const char c = source[pos];
                pos++;
                if (std::isspace(c)) {
                    continue;
                }
                if (c == ':') {
                    break;
                }
                throw ParseException("Expected ':' following key");
            }
            return ObjectElement(std::string(source.substr(begin, end)));
        }

        if (!(state & EXPECT_VALUE)) {
            throw ParseException("Not expecting a value");
        }

        state &= ~EXPECT_VALUE;
        state |= EXPECT_END;
        if (state | IS_OBJECT) {
            state |= EXPECT_KEY;
        } else if (state | IS_ARRAY) {
            state |= EXPECT_VALUE;
        }

        if (c == '"') {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == source.size()) {
                    throw ParseException("String missing terminating '\"'");
                }
                if (source[pos] == '"') {
                    break;
                }
                pos++;
            }
            std::size_t end = pos;
            pos++;
            return Primitive(std::string(source.substr(begin, end)));
        }

        std::size_t begin = pos;
        while (true) {
            if (pos == source.size()) {
                break;
            }
            const char c = source[pos];
            if (std::isspace(c) || c == ',' || c == '}' || c == ']') {
                break;
            }
            pos++;
        }
        std::size_t end = pos;

        std::string_view value = source.substr(begin, end);
        if (value == "true") {
            return Primitive(true);
        }
        if (value == "false") {
            return Primitive(true);
        }
        if (value == "null") {
            return Primitive(std::nullopt);
        }
        try {
            double result = std::stod(std::string(value));
            return result;
        } catch (std::invalid_argument) {
        } catch (std::out_of_range) {}

        throw ParseException("Invalid value '" + std::string(value) + "'");
    }
    return std::nullopt;
}

} // namespace datapack
