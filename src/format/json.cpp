#include "datapack/format/json.hpp"
#include "datapack/exception.hpp"
#include <assert.h>
#include <iostream>

namespace datapack {

#if 0

JsonParser::JsonParser(const std::string& source):
    source(source),
    pos(0)
{
    states.push(EXPECT_VALUE);
}

Token JsonParser::next() {
    while (true) {
        int& state = states.top();

        if (pos == source.size()) {
            if (!(state & EXPECT_END) || (state & IS_OBJECT) || (state & IS_ARRAY)) {
                error("Not expecting the document end");
            }
            return token::DocumentEnd();
        }

        const char c = source[pos];
        if (std::isspace(c)) {
            pos++;
            continue;
        }

        if (c == '{') {
            if (!(state | EXPECT_VALUE)) {
                error("Unexpected character '{'");
            }
            pos++;
            states.push(IS_OBJECT | EXPECT_ELEMENT | EXPECT_END);
            return token::ObjectBegin();
        }
        if (c == '[') {
            if (!(state & EXPECT_VALUE)) {
                error("Unexpected character '['");
            }
            pos++;
            states.push(IS_ARRAY | EXPECT_ELEMENT | EXPECT_END);
            return token::ArrayBegin();
        }
        if (c == '}') {
            if (!(state & IS_OBJECT) || !(state & EXPECT_END)) {
                error("Unexpected character '}'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_NEXT | EXPECT_END;
            return token::ObjectEnd();
        }
        if (c == ']') {
            if (!(state & IS_ARRAY) | !(state & EXPECT_END)) {
                error("Unexpected character ']'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_NEXT | EXPECT_END;
            return token::ArrayEnd();
        }
        if (c == ',') {
            pos++;

            if (!(state & EXPECT_NEXT)) {
                error("Unexpected character ','");
            }
            state &= ~EXPECT_NEXT;
            state |= EXPECT_ELEMENT;
            continue;
        }

        if (c == '"' && (state & IS_OBJECT) && (state & EXPECT_ELEMENT)) {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == source.size()) {
                    error("Key missing terminating '\"'");
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
                    error("Expected ':' following key");
                }
                const char c = source[pos];
                pos++;
                if (std::isspace(c)) {
                    continue;
                }
                if (c == ':') {
                    break;
                }
                error("Expected ':' following key");
            }
            state &= ~EXPECT_ELEMENT;
            state |= EXPECT_VALUE;
            return token::ObjectElement(std::string(source.substr(begin, end-begin)));
        }

        if ((state & IS_ARRAY) && (state & EXPECT_ELEMENT)) {
            state &= ~EXPECT_ELEMENT;
            state |= EXPECT_VALUE;
            return token::ArrayElement();
        }

        if (!(state & EXPECT_VALUE)) {
            error("Not expecting a value");
        }

        state &= ~EXPECT_VALUE;
        state |= EXPECT_END | EXPECT_NEXT;

        if (c == '"') {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == source.size()) {
                    error("String missing terminating '\"'");
                }
                if (source[pos] == '"') {
                    break;
                }
                pos++;
            }
            std::size_t end = pos;
            pos++;
            return token::Primitive(std::string(source.substr(begin, end-begin)));
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

        std::string_view value = source.substr(begin, end-begin);
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

        throw LoadException("Invalid value '" + std::string(value) + "'");
    }
    return std::nullopt;
}

#endif

Object load_json(const std::string& json) {
    static constexpr int EXPECT_ELEMENT = 1 << 0;
    static constexpr int EXPECT_VALUE = 1 << 1;
    static constexpr int EXPECT_END = 1 << 2;
    static constexpr int EXPECT_NEXT = 1 << 3;
    static constexpr int IS_OBJECT = 1 << 4;
    static constexpr int IS_ARRAY = 1 << 5;

    std::size_t pos = 0;
    std::stack<int> states;
    states.push(EXPECT_VALUE);
    Object object(Object::null);
    Object node = object;

    while (true) {
        int& state = states.top();

        if (pos == json.size()) {
            if (!(state & EXPECT_END) || (state & IS_OBJECT) || (state & IS_ARRAY)) {
                throw LoadException("Not expecting the document end");
            }
            assert(!node);
            assert(states.size() == 1);
            break;
        }

        const char c = json[pos];
        if (std::isspace(c)) {
            pos++;
            continue;
        }

        if (c == '{') {
            if (!(state | EXPECT_VALUE)) {
                throw LoadException("Unexpected character '{'");
            }
            pos++;
            states.push(IS_OBJECT | EXPECT_ELEMENT | EXPECT_END);
            node.set(Object::map_t());
            continue;
        }
        if (c == '[') {
            if (!(state & EXPECT_VALUE)) {
                throw LoadException("Unexpected character '['");
            }
            pos++;
            states.push(IS_ARRAY | EXPECT_ELEMENT | EXPECT_END);
            node.set(Object::list_t());
            continue;
        }
        if (c == '}') {
            if (!(state & IS_OBJECT) || !(state & EXPECT_END)) {
                throw LoadException("Unexpected character '}'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_NEXT | EXPECT_END;
            node = node.parent();
            continue;
        }
        if (c == ']') {
            if (!(state & IS_ARRAY) | !(state & EXPECT_END)) {
                throw LoadException("Unexpected character ']'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_NEXT | EXPECT_END;
            node = node.parent();
            continue;
        }
        if (c == ',') {
            pos++;

            if (!(state & EXPECT_NEXT)) {
                throw LoadException("Unexpected character ','");
            }
            state &= ~EXPECT_NEXT;
            state |= EXPECT_ELEMENT;
            continue;
        }

        if (c == '"' && (state & IS_OBJECT) && (state & EXPECT_ELEMENT)) {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == json.size()) {
                    throw LoadException("Key missing terminating '\"'");
                }
                const char c = json[pos];
                if (c == '"') {
                    break;
                }
                pos++;
            }
            std::size_t end = pos;
            pos++;

            while (true) {
                if (pos == json.size()) {
                    throw LoadException("Expected ':' following key");
                }
                const char c = json[pos];
                pos++;
                if (std::isspace(c)) {
                    continue;
                }
                if (c == ':') {
                    break;
                }
                throw LoadException("Expected ':' following key");
            }
            state &= ~EXPECT_ELEMENT;
            state |= EXPECT_VALUE;
            std::string key = json.substr(begin, end-begin);
            node = node.insert(key, Object::null);
            continue;
        }

        if ((state & IS_ARRAY) && (state & EXPECT_ELEMENT)) {
            state &= ~EXPECT_ELEMENT;
            state |= EXPECT_VALUE;
            node = node.append(Object::null);
        }

        if (!(state & EXPECT_VALUE)) {
            throw LoadException("Not expecting a value");
        }

        state &= ~EXPECT_VALUE;
        state |= EXPECT_END | EXPECT_NEXT;

        if (c == '"') {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == json.size()) {
                    throw LoadException("String missing terminating '\"'");
                }
                if (json[pos] == '"') {
                    break;
                }
                pos++;
            }
            std::size_t end = pos;
            pos++;
            node.set(json.substr(begin, end-begin));
            node = node.parent();
            continue;
        }

        std::size_t begin = pos;
        while (true) {
            if (pos == json.size()) {
                break;
            }
            const char c = json[pos];
            if (std::isspace(c) || c == ',' || c == '}' || c == ']') {
                break;
            }
            pos++;
        }
        std::size_t end = pos;

        std::string value = json.substr(begin, end-begin);
        if (value == "true") {
            node.set(true);
            node = node.parent();
            continue;
        }
        if (value == "false") {
            node.set(false);
            node = node.parent();
            continue;
        }
        if (value == "null") {
            node.set(Object::null);
            node = node.parent();
            continue;
        }
        try {
            std::int64_t result_int = std::strtoll(value.c_str(), nullptr, 10);
            double result_float = std::strtod(value.c_str(), nullptr);
            // Note: If a double/float has an integer value, comparisons are
            // still valid since it is guaranteed to represent the integer
            // exactly
            if (result_int == result_float) {
                node.set(result_int);
            } else {
                node.set(result_float);
            }
            node = node.parent();
            continue;
        } catch (std::invalid_argument) {
        } catch (std::out_of_range) {}

        throw LoadException("Invalid value '" + std::string(value) + "'");
    }

    return object;
}

std::string dump_json(ConstObject object) {
    std::string json = "";
    // TODO
    return json;
}

} // namespace datapack
