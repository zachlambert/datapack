#include "datapack/format/json.hpp"

namespace datapack {

JsonWriter::JsonWriter(std::string& result):
    result(result),
    first_element(false),
    depth(0)
{}

void JsonWriter::value_i32(std::int32_t value) {
    result += std::to_string(value);
}

void JsonWriter::value_i64(std::int64_t value) {
    result += std::to_string(value);
}

void JsonWriter::value_u32(std::uint32_t value) {
    result += std::to_string(value);
}

void JsonWriter::value_u64(std::uint64_t value) {
    result += std::to_string(value);
}


void JsonWriter::value_f32(float value) {
    result += std::to_string(value);
}

void JsonWriter::value_f64(double value) {
    result += std::to_string(value);
}


void JsonWriter::value_string(const std::string& value) {
    result += "\"" + value + "\"";
}

void JsonWriter::value_bool(bool value) {
    result += (value ? "true" : "false");
}


void JsonWriter::enumerate(int value, const std::vector<const char*>& labels) {
    value_string(std::string(labels[value]));
}

void JsonWriter::optional(bool has_value) {
    if (!has_value) {
        result += "null";
    }
}

void JsonWriter::variant_begin(const char* label, const std::vector<const char*>& labels) {
    object_begin();
    object_next("label");
    value_string(label);
    object_next("value");
}

void JsonWriter::variant_end() {
    object_end();
}


void JsonWriter::binary(std::size_t size, const std::uint8_t* data) {
    result += "\"binary - not implemented\"";
}


void JsonWriter::map_begin() {
    result += "{\n";
    first_element = true;
    depth++;
}

void JsonWriter::map_end() {
    depth--;
    if (!first_element) {
        result += "\n";
        indent();
    }
    result += "}";
    first_element = false;
}

void JsonWriter::map_next(const std::string& key) {
    if (!first_element) {
        result += ",\n";
    }
    first_element = false;
    indent();
    result += "\"" + key + "\": ";
}


void JsonWriter::list_begin() {
    result += "{\n";
    first_element = true;
    depth++;
}

void JsonWriter::list_end() {
    depth--;
    if (!first_element) {
        result += "\n";
        indent();
    }
    result += "]";
    first_element = false;
}

void JsonWriter::list_next() {
    if (!first_element) {
        result += ",\n";
    }
    first_element = false;
    indent();
}


void JsonWriter::indent() {
    for (int i = 0; i < depth; i++) {
        result += "    ";
    }
}


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

        throw ParseException("Invalid value '" + std::string(value) + "'");
    }
    return std::nullopt;
}

#endif

} // namespace datapack
