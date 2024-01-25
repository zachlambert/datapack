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

class TokenReader {
public:
    virtual std::optional<Token> read() = 0;
};
class TokenWriter {
public:
    virtual void write(const Token& token) = 0;
};

class Input {
public:
    virtual Input& value(int& value) = 0;
    virtual Input& value(long& value) = 0;
    virtual Input& value(float& value) = 0;
    virtual Input& value(double& value) = 0;
    virtual Input& value(std::string& value) = 0;
    virtual Input& value(bool& value) = 0;

    virtual Input& key(const std::string& key) = 0;
    virtual Input& start_object() = 0;
    virtual Input& end_object() = 0;

    virtual Input& next() = 0;
    virtual Input& start_array() = 0;
    virtual Input& end_array() = 0;
};

class TokenInput: public Input {
public:
    TokenInput(TokenReader& reader):
        reader(reader)
    {}

    TokenInput& value(int& value) {
        value = std::get<int>(std::get<Primitive>(reader.read().value()));
        return *this;
    }
    TokenInput& value(long& value) {
        value = std::get<long>(std::get<Primitive>(reader.read().value()));
        return *this;
    }
    TokenInput& value(float& value) {
        value = std::get<float>(std::get<Primitive>(reader.read().value()));
        return *this;
    }
    TokenInput& value(double& value) {
        value = std::get<double>(std::get<Primitive>(reader.read().value()));
        return *this;
    }
    TokenInput& value(std::string& value) {
        value = std::get<std::string>(std::get<Primitive>(reader.read().value()));
        return *this;
    }
    TokenInput& value(bool& value) {
        value = std::get<bool>(std::get<Primitive>(reader.read().value()));
        return *this;
    }

    TokenInput& key(const std::string& key) {
        assert(std::get<ObjectElement>(reader.read().value()).key == key);
        return *this;
    }
    TokenInput& start_object() {
        std::get<ObjectStart>(reader.read().value());
        return *this;
    }
    TokenInput& end_object() {
        std::get<ObjectEnd>(reader.read().value());
        return *this;
    }

    TokenInput& next() {
        std::get<ArrayElement>(reader.read().value());
        return *this;
    }
    TokenInput& start_array() {
        std::get<ArrayStart>(reader.read().value());
        return *this;
    }
    TokenInput& end_array() {
        std::get<ArrayEnd>(reader.read().value());
        return *this;
    }

private:
    TokenReader& reader;
};

class Output {
public:
    virtual Output& value(int value) = 0;
    virtual Output& value(long value) = 0;
    virtual Output& value(float value) = 0;
    virtual Output& value(double value) = 0;
    virtual Output& value(std::string value) = 0;
    virtual Output& value(bool value) = 0;

    virtual Output& key(const std::string& key) = 0;
    virtual Output& start_object() = 0;
    virtual Output& end_object() = 0;

    virtual Output& next() = 0;
    virtual Output& start_array() = 0;
    virtual Output& end_array() = 0;
};

class TokenOutput: public Output {
public:
    TokenOutput(TokenWriter& writer):
        writer(writer)
    {}

    TokenOutput& value(int value) { writer.write(Primitive(value)); return *this; }
    TokenOutput& value(long value) { writer.write(Primitive(value)); return * this; }
    TokenOutput& value(float value) { writer.write(Primitive(value)); return *this; }
    TokenOutput& value(double value) { writer.write(Primitive(value)); return *this; }
    TokenOutput& value(std::string value) { writer.write(Primitive(value)); return *this; }
    TokenOutput& value(bool value) { writer.write(Primitive(value)); return *this; }

    TokenOutput& key(const std::string& key) { writer.write(ObjectElement(key)); return *this; }
    TokenOutput& start_object() { writer.write(ObjectStart()); return *this; }
    TokenOutput& end_object() { writer.write(ObjectEnd()); return *this; }

    TokenOutput& next() { writer.write(ArrayElement()); return *this; }
    TokenOutput& start_array() { writer.write(ArrayStart()); return *this; }
    TokenOutput& end_array() { writer.write(ArrayEnd()); return *this; }

private:
    TokenWriter& writer;
};

class Writer {
public:
    virtual void write(Output& output) const = 0;
};

class Reader {
public:
    virtual void read(Input& input) = 0;
};

struct Foo: public Reader, Writer {
    double x;
    double y;
    double a;
    double b;
    void write(Output& output) const override {
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
    void read(Input& input) override {
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

class JsonReader: public TokenReader {
public:
    JsonReader(const std::string& text): text(text), pos(0), at_value(true) {}
    std::optional<Token> read() override {
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

class JsonWriter: public TokenWriter {
public:
    JsonWriter(): level(0) {}
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

class RandomInput: public Input {
public:
    RandomInput& value(int& value) {
        value = rand() % 10;
        return *this;
    }
    RandomInput& value(long& value) {
        value = rand() % 100;
        return *this;
    }
    RandomInput& value(float& value) {
        value = (float)rand() / (float)RAND_MAX;
        return *this;
    }
    RandomInput& value(double& value) {
        value = (double)rand() / (double)RAND_MAX;
        return *this;
    }
    RandomInput& value(std::string& value) {
        value.resize(16);
        for (std::size_t i = 0; i < value.size(); i++) {
            value[i] = 'a' + rand() % 26;
        }
        return *this;
    }
    RandomInput& value(bool& value) {
        value = rand() > RAND_MAX/2;
        return *this;
    }

    RandomInput& key(const std::string& key) {
        return *this;
    }
    RandomInput& start_object() {
        return *this;
    }
    RandomInput& end_object() {
        return *this;
    }

    RandomInput& next() {
        return *this;
    }
    RandomInput& start_array() {
        return *this;
    }
    RandomInput& end_array() {
        return *this;
    }
};

class PrintOutput: public TokenWriter {
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
        JsonWriter writer;
        TokenOutput output(writer);
        foo.read(input);
        foo.write(output);
        json = writer.result();
    }
    std::cout << json;
    {
        JsonReader reader(json);
        TokenInput input(reader);
        PrintOutput print_output(std::cout);
        TokenOutput output(print_output);
        foo.read(input);
        foo.write(output);
    }

    return 0;
}
