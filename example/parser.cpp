#include <string>
#include <variant>
#include <optional>
#include <stack>
#include <sstream>
#include <charconv>
#include <iostream>
#include <memory>

#include "datapack/writer.h"
#include "datapack/yaml.h"

namespace datapack {

class ParseException: public std::exception {
public:
    ParseException(std::size_t line, std::size_t col, const std::string& message) {
        std::stringstream ss;
        ss << "Line " << line << ", column " << col << ": " << message;
        this->message = ss.str();
    }
private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};


struct ObjectStart{};
struct ObjectEnd{};
struct ArrayStart{};
struct ArrayEnd{};
struct ObjectKey {
    std::string key;
    ObjectKey(const std::string& key): key(key) {}
};
struct ArrayNext {};
struct End{};

using Token = std::variant<
    double,
    bool,
    std::string,
    std::nullopt_t,
    ObjectStart, ObjectEnd,
    ArrayStart, ArrayEnd,
    ObjectKey,
    ArrayNext,
    End
>;

class Parser {
public:
    virtual Token next() = 0;
};

class JsonParser: public Parser {
    enum class Container {
        Object,
        Array,
    };
    enum class Expected {
        Element,
        Value,
        Key,
        NextOrEnd,
        ElementOrEnd,
    };
public:
    JsonParser(const std::string& text):
        text(text),
        pos(0),
        expected(Expected::Value)
    {}

    Token next() override {
        while (true) {
            char c = advance();
            if (c == '\0') {
                return End();
            }

            if (c == '}' || c == ']') {
                if (expected != Expected::NextOrEnd
                    && expected != Expected::ElementOrEnd)
                {
                    throw ParseException(line, pos, "Unexpected '}'");
                }
                container.pop();
                expected = Expected::NextOrEnd;
                if (c == '}') {
                    return ObjectEnd();
                } else {
                    return ArrayEnd();
                }
            }
            if (expected == Expected::ElementOrEnd) {
                expected = Expected::Element;
            }
            if (c == ',') {
                if (expected != Expected::NextOrEnd) {
                    throw ParseException(line, pos, "Unexpected ','");
                }
                expected = Expected::Element;
                continue;
            }
            if (expected == Expected::NextOrEnd) {
                if (container.top() == Container::Object) {
                    throw ParseException(line, pos, "Expected ',' or '}'");
                } else {
                    throw ParseException(line, pos, "Expected ',' or ']'");
                }
            }

            if (expected == Expected::Element) {
                if (container.empty() || container.top() == Container::Object) {
                    expected = Expected::Key;
                } else {
                    expected = Expected::Value;
                    if (c != ',') {
                        pos--;
                    }
                    return ArrayNext();
                }
            }

            if (c == '"' && expected == Expected::Key) {
                std::string key = consume_string();
                if (advance() != ':') {
                    throw ParseException(line, pos, "Expected ':' following  key");
                }
                expected = Expected::Value;
                return ObjectKey(key);
            }

            if (c == '{') {
                container.push(Container::Object);
                expected = Expected::ElementOrEnd;
                return ObjectStart();
            }
            if (c == '[') {
                container.push(Container::Array);
                expected = Expected::ElementOrEnd;
                return ArrayStart();
            }

            // If here, must be matching a value (that isn't a container)
            expected = Expected::NextOrEnd;

            if (c == '"') {
                return consume_string();
            }

            std::string value = consume_value();
            if (value == "true") {
                return true;
            }
            if (value == "false") {
                return false;
            }
            if (value == "null") {
                return std::nullopt;
            }

            try {
                double result = std::stod(value);
                return result;
            } catch (std::invalid_argument) {
            } catch (std::out_of_range) {}

            throw ParseException(line, col, "Invalid value '" + value + "'");
        }
    }
private:
    // Consume whitespace and return first non-whitespace character
    char advance() {
        while (true) {
            if (pos == text.size()) {
                return '\0';
            }
            char c = text[pos];
            pos++;
            if (c == '\n') {
                line++;
                col = 0;
                continue;
            }
            col++;

            if (std::isspace(c)) {
                continue;
            }

            return c;
        }
    }

    std::string consume_string() {
        std::size_t start = pos;
        std::size_t end = pos;
        while (true) {
            if (pos == text.size()) {
                throw ParseException(line, pos, "Unterminated string");
            }
            char c = text[pos];
            pos++;
            if (c != '"') {
                end = pos;
                continue;
            }
            return text.substr(start, end - start);
        }
    }

    std::string consume_value() {
        if (pos == 0) {
            throw ParseException(line, pos, "Unexpected error");
        }
        std::size_t start = pos-1;
        while (pos != text.size()) {
            char c = text[pos];
            if (std::isspace(c)) break;
            if (c == ',' || c == '}' || c == ']') break;
            pos++;
        }
        return text.substr(start, pos-start);
    }

    std::string text;
    std::size_t pos;
    std::size_t line;
    std::size_t col;

    std::stack<Container> container;
    Expected expected;
};

class ParserWriteable: public Writeable {
public:
    template <typename Impl>
    ParserWriteable(const Impl& impl):
        parser(std::make_unique<Impl>(impl))
    {}
    void write(Writer& writer) const override {
        while (true) {
            auto token = parser->next();
            if (std::get_if<End>(&token)) {
                return;
            }
            else if (std::get_if<ObjectStart>(&token)) {
                writer.start_object();
            }
            else if (std::get_if<ObjectEnd>(&token)) {
                writer.end_object();
            }
            else if (std::get_if<ArrayStart>(&token)) {
                writer.start_array();
            }
            else if (std::get_if<ArrayEnd>(&token)) {
                writer.end_array();
            }
            else if (auto key = std::get_if<ObjectKey>(&token)) {
                writer.key(key->key);
            }
            else if (std::get_if<ArrayNext>(&token)) {
                writer.next();
            }
            else if (auto value = std::get_if<std::string>(&token)) {
                writer.string(*value);
            }
            else if (auto value = std::get_if<double>(&token)) {
                writer.f64(*value);
            }
            else if (std::get_if<std::nullopt_t>(&token)) {
                writer.null();
            }
            else if (auto value = std::get_if<bool>(&token)) {
                writer.boolean(*value);
            }
            else {
                throw std::logic_error("Should not reach here");
            }
        }
    }
private:
    std::unique_ptr<Parser> parser;
};

} // namespace datapack

int main() {
    using namespace datapack;

    std::string json = R"(
{
    "a": {
        "1": [ 1, 2, "asdf" ],
        "2": null
    },
    "b": {
        "one": { "three": { "four": "five" }}
    }
}
)";

    std::cout
        << YamlWriter()
            .value(ParserWriteable(JsonParser(json)))
            .result()
        << std::endl;
    return 0;
}
