#include <string>
#include <variant>
#include <assert.h>
#include <iostream>
#include <random>
#include <optional>
#include <memory>
#include <sstream>


struct ObjectStart{};
struct ObjectEnd{};
struct ArrayStart{};
struct ArrayEnd{};
struct ObjectElement {
    std::string key;
    ObjectElement(const std::string& key): key(key) {}
};
struct ArrayElement {};
struct End{};

using Primitive = std::variant<
    int,
    long,
    float,
    double,
    bool,
    std::string
>;
using PrimitiveRef = std::variant<
    int*,
    long*,
    float*,
    double*,
    bool*,
    std::string*
>;

using Token = std::variant<
    Primitive,
    ObjectStart, ObjectElement, ObjectEnd,
    ArrayStart, ArrayElement, ArrayEnd
>;
using TokenRef = std::variant<
    PrimitiveRef,
    ObjectStart, ObjectElement, ObjectEnd,
    ArrayStart, ArrayElement, ArrayEnd
>;

class TokenInput {
public:
    virtual void read(const TokenRef& like) = 0;

    TokenInput& value(int& value) { read(PrimitiveRef(&value)); return *this; }
    TokenInput& value(long& value) { read(PrimitiveRef(&value)); return * this; }
    TokenInput& value(float& value) { read(PrimitiveRef(&value)); return *this; }
    TokenInput& value(double& value) { read(PrimitiveRef(&value)); return *this; }
    TokenInput& value(std::string& value) { read(PrimitiveRef(&value)); return *this; }
    TokenInput& value(bool& value) { read(PrimitiveRef(&value)); return *this; }

    TokenInput& key(const std::string& key) { read(ObjectElement(key)); return *this; }
    TokenInput& start_object() { read(ObjectStart()); return *this; }
    TokenInput& end_object() { read(ObjectEnd()); return *this; }

    TokenInput& next() { read(ArrayElement()); return *this; }
    TokenInput& start_array() { read(ArrayStart()); return *this; }
    TokenInput& end_array() { read(ArrayEnd()); return *this; }
};

class TokenOutput {
public:
    virtual void write(const Token& token) = 0;

    TokenOutput& value(int value) { write(Primitive(value)); return *this; }
    TokenOutput& value(long value) { write(Primitive(value)); return * this; }
    TokenOutput& value(float value) { write(Primitive(value)); return *this; }
    TokenOutput& value(double value) { write(Primitive(value)); return *this; }
    TokenOutput& value(std::string value) { write(Primitive(value)); return *this; }
    TokenOutput& value(bool value) { write(Primitive(value)); return *this; }

    TokenOutput& key(const std::string& key) { write(ObjectElement(key)); return *this; }
    TokenOutput& start_object() { write(ObjectStart()); return *this; }
    TokenOutput& end_object() { write(ObjectEnd()); return *this; }

    TokenOutput& next() { write(ArrayElement()); return *this; }
    TokenOutput& start_array() { write(ArrayStart()); return *this; }
    TokenOutput& end_array() { write(ArrayEnd()); return *this; }
};

class TokenParser {
public:
    virtual std::optional<Token> next() = 0;
};

class TokenWriter {
public:
    virtual void write(TokenOutput& output) const = 0;
};

class TokenReader {
public:
    virtual void read(TokenInput& input) = 0;
};

class ParserWriter: public TokenWriter {
public:
    template <typename Impl>
    ParserWriter(const Impl& impl):
        parser(std::make_unique<Impl>(impl))
    {}
    void write(TokenOutput& output) const override {
        while (true) {
            auto token = parser->next();
            if (!token.has_value()) break;
            output.write(token.value());
        }
    }
private:
    std::unique_ptr<TokenParser> parser;
};

class ParserInput: public TokenInput {
public:
    template <typename Impl>
    ParserInput(const Impl& impl):
        parser(std::make_unique<Impl>(impl))
    {}
    void read(const TokenRef& token) override {
        auto actual = parser->next().value();
        std::visit([&actual](const auto& token) {
            using T = std::decay_t<decltype(token)>;
            if constexpr(!std::is_same_v<T, PrimitiveRef>) {
                std::get_if<T>(&actual);
            }
            if constexpr(std::is_same_v<T, PrimitiveRef>) {
                auto actual_v = std::get_if<Primitive>(&actual);
                std::visit([&token](const auto& actual) {
                    using T_actual = std::decay_t<decltype(actual)>;
                    *std::get<T_actual*>(token) = actual;
                }, *actual_v);
            }
        }, token);
    }
private:
    std::unique_ptr<TokenParser> parser;
};

struct Foo: public TokenReader, TokenWriter {
    double x;
    double y;
    double a;
    double b;
    void write(TokenOutput& output) const override {
        output
            .start_object()
                .key("x").value(x)
                .key("y").value(y)
                .key("letters").start_object()
                    .key("a").value(a)
                    .key("b").value(b)
                .end_object()
            .end_object();
    }
    void read(TokenInput& input) override {
        input
            .start_object()
                .key("x").value(x)
                .key("y").value(y)
                .key("letters").start_object()
                    .key("a").value(a)
                    .key("b").value(b)
                .end_object()
            .end_object();
    }
};

class JsonParser: public TokenParser {
public:
    JsonParser(const std::string& text): text(text), pos(0), at_value(true) {}
    std::optional<Token> next() override {
        while (true) {
            if (pos == text.size()) {
                return std::nullopt;
            }
            char c = text[pos];
            pos++;
            if (std::isspace(c)) continue;

            if (c == '{')  {
                at_value = false;
                return ObjectStart();
            }
            if (c == '}') {
                match_trailing(',');
                at_value = false;
                return ObjectEnd();
            }
            if (c == '[') {
                at_value = false;
                return ArrayStart();
            }
            if (c == ']') {
                match_trailing(',');
                at_value = false;
                return ArrayEnd();
            }
            if (c == ',') {
                continue;
            }

            if (!at_value) {
                at_value = true;
                if (c == '"') {
                    std::string key = consume_string();
                    match_trailing(':');
                    return ObjectElement(key);
                }
                pos--;
                return ArrayStart();
            }

            at_value = false;
            std::string value = c + consume_value();
            match_trailing(',');

            if (!value.empty() && value.front() == '"') {
                assert(value.size() >= 2 && value.back() == '"');
                return Primitive(value.substr(1, value.size()-2));
            }
            if (value == "true") {
                return Primitive(true);
            }
            if (value == "false") {
                return Primitive(true);
            }
            try {
                double result = std::stod(value);
                return Primitive(result);
            } catch (std::invalid_argument) {
            } catch (std::out_of_range) {}

            assert(false);
        }
        // Unreachable
        return std::nullopt;
    }
private:
    void match_trailing(char match) {
        while (true) {
            assert(pos != text.size());
            char c = text[pos];
            pos++;
            if (std::isspace(c)) {
                continue;
            }
            if (c == match) {
                break;
            }
            assert(false);
        }
    }
    std::string consume_string() {
        std::size_t start = pos;
        std::size_t end = pos;
        while (true) {
            char c = text[pos];
            pos++;
            if (c != '"') {
                end = pos;
                continue;
            }
            break;
        }
        return text.substr(start, end-start);
    }
    std::string consume_value() {
        std::size_t start = pos;
        std::size_t end = pos;
        while (true) {
            assert(pos != text.size());
            char c = text[pos];
            if (!std::isspace(c) && c != ',' ) {
                end = pos;
                pos++;
                continue;
            }
            break;
        }
        return text.substr(start, end-start);
    }

    std::string text;
    std::size_t pos;
    bool at_value;
};

class JsonOutput: public TokenOutput {
public:
    JsonOutput(): level(0) {}
    void write(const Token& token) override {
        if (auto primitive = std::get_if<Primitive>(&token)) {
            std::visit([this](const auto& value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr(std::is_same_v<T, bool>) {
                    ss << (value ? "true" : "false");
                    return;
                }
                ss << value;
            }, *primitive);
            ss << ",\n";
        }
        else if (std::get_if<ObjectStart>(&token)) {
            level++;
            ss << "{\n";
        }
        else if (std::get_if<ArrayStart>(&token)) {
            level++;
            ss << "[\n";
        }
        else if (std::get_if<ObjectEnd>(&token)) {
            level--;
            indent();
            ss << "},\n";
        }
        else if (std::get_if<ArrayEnd>(&token)) {
            level--;
            indent();
            ss << "],\n";
        }
        else if (auto element = std::get_if<ObjectElement>(&token)) {
            indent();
            ss << "\"" << element->key << "\": ";
        }
        else if (std::get_if<ArrayElement>(&token)) {
            indent();
        }
    }
    std::string result() const {
        return ss.str();
    }
private:
    void indent() {
        for (std::size_t i = 0; i < level; i++) {
            ss << "    ";
        }
    }

    std::stringstream ss;
    int level;
};

class RandomInput: public TokenInput {
public:
    void read(const TokenRef& token) override {
        auto primitive = std::get_if<PrimitiveRef>(&token);
        if (!primitive) return;

        if (auto value = std::get_if<int*>(primitive)) {
            **value = rand() % 10;
        }
        else if (auto value = std::get_if<long*>(primitive)) {
            **value = rand() % 100;
        }
        else if (auto value = std::get_if<float*>(primitive)) {
            **value = (float)rand() / (float)RAND_MAX;
        }
        else if (auto value = std::get_if<double*>(primitive)) {
            **value = (double)rand() / (double)RAND_MAX;
        }
        else if (auto value = std::get_if<std::string*>(primitive)) {
            std::string& result = **value;
            result.clear();
            for (std::size_t i = 0; i < 16; i++) {
                result.push_back('a' + rand() % 26);
            }
        }
        else if (auto value = std::get_if<bool*>(primitive)) {
            **value = rand() > RAND_MAX/2 ? true : false;
        }
    }
};

class PrintOutput: public TokenOutput {
public:
    PrintOutput(std::ostream& os):
        os(os),
        level(-1)
    {}
    void write(const Token& token) override {
        if (auto primitive = std::get_if<Primitive>(&token)) {
            std::visit([this](const auto& value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr(std::is_same_v<T, bool>) {
                    os << (value ? "true" : "false") << "\n";
                    return;
                }
                os << value << "\n";
            }, *primitive);
        }
        else if (std::get_if<ObjectStart>(&token) || std::get_if<ArrayStart>(&token)) {
            if (level >= 0) {
                os << '\n';
            }
            level++;
        }
        else if (std::get_if<ObjectEnd>(&token) || std::get_if<ArrayEnd>(&token)) {
            level--;
        }
        else if (auto element = std::get_if<ObjectElement>(&token)) {
            indent();
            os << element->key << ": ";
        }
        else if (std::get_if<ArrayElement>(&token)) {
            indent();
            os << "- ";
        }
    }
private:
    void indent() {
        for (std::size_t i = 0; (int)i < level; i++) {
            os << "  ";
        }
    }
    std::ostream& os;
    int level;
};

int main() {
    Foo foo;

    // Note: Currently requiring trailing commas in Json to make
    // parsing and printing easier

    std::string json;
    {
        RandomInput input;
        JsonOutput output;
        foo.read(input);
        foo.write(output);
        json = output.result();
    }
    std::cout << json;
    {
        JsonParser parser(json);
        ParserInput input(parser);
        PrintOutput output(std::cout);
        foo.read(input);
        foo.write(output);
    }

    return 0;
}
