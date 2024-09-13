#include "datapack/format/json.hpp"
#include <assert.h>
#include "datapack/encode/base64.hpp"
#include "datapack/encode/float_string.hpp"


namespace datapack {

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
    Object object;
    Object::Iterator iter = object.iter();

    while (true) {
        int& state = states.top();

        if (pos == json.size()) {
            if (!(state & EXPECT_END) || (state & IS_OBJECT) || (state & IS_ARRAY)) {
                throw JsonLoadError("Not expecting the document end");
            }
            assert(!iter);
            assert(states.size() == 1);
            break;
        }

        const char c = json[pos];
        if (std::isspace(c)) {
            pos++;
            continue;
        }
        assert(iter);

        if (c == '"' && (state & IS_OBJECT) && (state & EXPECT_ELEMENT)) {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == json.size()) {
                    throw JsonLoadError("Key missing terminating '\"'");
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
                    throw JsonLoadError("Expected ':' following key");
                }
                const char c = json[pos];
                pos++;
                if (std::isspace(c)) {
                    continue;
                }
                if (c == ':') {
                    break;
                }
                throw JsonLoadError("Expected ':' following key");
            }
            state &= ~EXPECT_ELEMENT;
            state |= EXPECT_VALUE;
            std::string key = json.substr(begin, end-begin);
            iter = iter->insert(key, Object::null_t());
            continue;
        }

        if ((state & IS_ARRAY) && (state & EXPECT_ELEMENT)) {
            state &= ~EXPECT_ELEMENT;
            state |= EXPECT_VALUE;
            iter = iter->push_back(Object::null_t());
            continue;
        }

        if (c == '{') {
            if (!(state | EXPECT_VALUE)) {
                throw JsonLoadError("Unexpected character '{'");
            }
            pos++;
            states.push(IS_OBJECT | EXPECT_ELEMENT | EXPECT_END);
            *iter = Object::map_t();
            continue;
        }
        if (c == '[') {
            if (!(state & EXPECT_VALUE)) {
                throw JsonLoadError("Unexpected character '['");
            }
            pos++;
            states.push(IS_ARRAY | EXPECT_ELEMENT | EXPECT_END);
            *iter = Object::list_t();
            continue;
        }
        if (c == '}') {
            if (!(state & IS_OBJECT) || !(state & EXPECT_END)) {
                throw JsonLoadError("Unexpected character '}'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_NEXT | EXPECT_END;
            iter = iter.parent();
            continue;
        }
        if (c == ']') {
            if (!(state & IS_ARRAY) | !(state & EXPECT_END)) {
                throw JsonLoadError("Unexpected character ']'");
            }
            pos++;
            states.pop();

            int& new_state = states.top();
            new_state &= ~EXPECT_VALUE;
            new_state |= EXPECT_NEXT | EXPECT_END;
            iter = iter.parent();
            continue;
        }
        if (c == ',') {
            pos++;

            if (!(state & EXPECT_NEXT)) {
                throw JsonLoadError("Unexpected character ','");
            }
            state &= ~EXPECT_NEXT;
            state |= EXPECT_ELEMENT;
            continue;
        }

        if (!(state & EXPECT_VALUE)) {
            throw JsonLoadError("Not expecting a value");
        }

        state &= ~EXPECT_VALUE;
        state |= EXPECT_END | EXPECT_NEXT;

        if (c == '"') {
            pos++;
            std::size_t begin = pos;
            while (true) {
                if (pos == json.size()) {
                    throw JsonLoadError("String missing terminating '\"'");
                }
                if (json[pos] == '"') {
                    break;
                }
                pos++;
            }
            std::size_t end = pos;
            pos++;
            *iter = json.substr(begin, end-begin);
            iter = iter.parent();
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
            *iter = true;
            iter = iter.parent();
            continue;
        }
        if (value == "false") {
            *iter = false;
            iter = iter.parent();
            continue;
        }
        if (value == "null") {
            *iter = Object::null_t();
            iter = iter.parent();
            continue;
        }
        try {
            Object::integer_t result_int = std::strtoll(value.c_str(), nullptr, 10);
            Object::floating_t result_float = std::strtod(value.c_str(), nullptr);
            // Note: If a double/float has an integer value, comparisons are
            // still valid since it is guaranteed to represent the integer
            // exactly
            if (result_int == result_float) {
                *iter = result_int;
            } else {
                *iter = result_float;
            }
            iter = iter.parent();
            continue;
        } catch (std::invalid_argument) {
        } catch (std::out_of_range) {}

        throw JsonLoadError("Invalid value '" + std::string(value) + "'");
    }

    return object;
}

std::string dump_json(const Object::ConstReference& object) {
    std::string json = "";

    std::stack<Object::ConstIterator> stack;
    stack.push(object.iter());
    bool start = true;
    bool end = false;

    while (!stack.empty()) {
        auto iter = stack.top();
        if (!iter) {
            stack.pop();
            end = true;
            continue;
        }

        if (end) {
            end = false;
            if (!start) {
                json += "\n";
                for (int i = 0; i < (stack.size()-1); i++) {
                    json += "    ";
                }
            }
            start = false;
            if (iter->is_map()) {
                json += "}";
                stack.pop();
                stack.push(iter.next());
                continue;
            }
            if (iter->is_list()) {
                json += "]";
                stack.pop();
                stack.push(iter.next());
                continue;
            }
            stack.pop();
            assert(stack.empty());
            continue;
        }

        if (!start) {
            json += ",\n";
        }
        start = false;

        for (int i = 0; i < (stack.size()-1); i++) {
            json += "    ";
        }
        if (!iter->key().empty()) {
            json += "\"" + iter->key() + "\": ";
        }

        if (iter->is_map()) {
            json += "{\n";
            stack.push(iter.child());
            start = true;
            continue;
        }
        if (iter->is_list()) {
            json += "[\n";
            stack.push(iter.child());
            start = true;
            continue;
        }

        if (auto value = iter->integer_if()) {
            json += std::to_string(*value);
        }
        else if (auto value = iter->floating_if()) {
            json += float_to_string(*value);
        }
        else if (auto value = iter->string_if()) {
            json += "\"" + *value + "\"";
        }
        else if (iter->is_null()) {
            json += "null";
        }
        else if (auto value = iter->boolean_if()) {
            json += (*value ? "true" : "false");
        }
        else if (auto value = iter->binary_if()) {
            json += "\"" + base64_encode(*value) + "\"";
        }

        stack.pop();
        stack.push(iter.next());
    }

    return json;
}

} // namespace datapack
