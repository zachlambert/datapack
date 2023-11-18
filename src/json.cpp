#include "datapack/json.h"
#include <sstream>


namespace datapack {

std::ostream& operator<<(std::ostream& os, JsonConstNode root)
{
    std::stack<JsonConstNode> stack;
    auto indent = [&]() {
        for (size_t i = 0; i < stack.size(); i++) {
            os << "  ";
        }
    };

    auto node = root;

    do {
        if (auto value = node.string_if()) {
            os << '"' << *value << '"';
        }
        else if (auto value = node.number_if()) {
            os << *value;
        }
        else if (auto value = node.boolean_if()) {
            os << (*value ? "true" : "false");
        }
        else if (node.is_null()) {
            os << "null";
        }
        else if (node.is_subtract()) {
            os << "subtract";
        }
        else if (node.is_object()) {
            if (node.empty()) {
                os << "{}";
            }
            else {
                os << "{\n";
                stack.emplace(node.begin());
                indent();
                os << "\"" << node.begin().key() << "\": ";
                node = *node.begin();
                continue;
            }
        }
        else if (node.is_array()) {
            if (node.empty()) {
                os << "[]";
            }
            else {
                os << "[\n";
                stack.emplace(node.begin());
                indent();
                node = *node.begin();
                continue;
            }
        }
        else
        {
            os << "<error>";
            // assert(false);
        }

        while (!stack.empty()) {
            auto prev = stack.top();
            auto iter = ++stack.top();

            if (iter.valid()) {
                os << ",\n";
                indent();
                if (iter.has_key()) {
                    os << "\"" << iter.key() << "\": ";
                }
                node = *iter;
                break;
            }
            stack.pop();
            os << "\n";
            indent();
            if (prev.has_key()) {
                os << "}";
            } else {
                os << "]";
            }
        }
    } while(!stack.empty());

    return os;
}

std::istream& operator>>(std::istream& is, JsonNode root)
{
    char c = ' ';
    std::stringstream ss;

    size_t line_num = 0;
    size_t char_num = 0;
    auto get = [&](char& c) -> std::istream& {
        is.get(c);
        char_num++;
        if (c == '\n') {
            line_num++;
            char_num = 0;
        }
        return is;
    };
    auto error = [&](const std::string_view& message) {
        ss.str("");
        ss << "Error on line " << line_num << ", char " << char_num << "\n";
        ss << message << std::endl;
        throw std::runtime_error(ss.str());
    };

    auto consume_string = [&]() -> std::string {
        ss.str("");
        while (get(c) && c != '"') {
            ss << c;
        }
        if (is.eof()) {
            error("Unterminated string encountered");
        }
        return ss.str();
    };

    std::stack<JsonNode> stack;
    JsonNode node = root;
    while (get(c) && std::isspace(c));

    bool have_value = true;
    do {
        while (!stack.empty()) {
            have_value = false;
            while (get(c) && std::isspace(c));
            auto parent = stack.top();
            if (parent.is_object()) {
                if (c == '}') {
                    stack.pop();
                    continue;
                }
                if (c == ',') {
                    while (get(c) && std::isspace(c));
                }
                if (c != '"') {
                    error("Expected character '\"' for key start");
                }
                node = parent[consume_string()];
                while (get(c) && std::isspace(c));
                if (is.eof() || c != ':') {
                    error("Missing a character ':' following a key");
                }
                while (get(c) && std::isspace(c));
            }
            else if (parent.is_array()) {
                if ( c== ']') {
                    stack.pop();
                    continue;
                }
                if (c == ',') {
                    while (get(c) && std::isspace(c));
                }
                node = *parent.push_back();
            }
            else {
                error("Non-container node pushed to stack");
            }
            have_value = true;
            break;
        }
        if (!have_value) break;

        if (c == '{') {
            node.set_object();
            stack.push(node);
            continue;
        }
        if (c == '[') {
            node.set_array();
            stack.push(node);
            continue;
        }
        if (c == '"') {
            node = consume_string();
            continue;
        }

        ss.str("");
        ss << c;
        while (true) {
            char next = is.peek();
            if (std::isspace(next) || next == ',' || next == '}' || next == ']') break;
            get(c);
            ss << c;
        }
        while (std::isspace(c) && get(c));

        std::string value = ss.str();
        if (value == "true") {
            node = true;
            continue;
        }
        if (value == "false") {
            node = false;
            continue;
        }
        if (value == "null") {
            node.set_null();
            continue;
        }

        try {
            double number = std::stod(value);
            node = number;
        } catch (std::invalid_argument) {
            error("Invalid value encountered");
        }
    } while (!stack.empty());
    return is;
}

Json& Json::operator=(const std::string_view& string) {
    std::stringstream ss;
    ss << string;
    ss >> get();
    return *this;
}

Json& Json::operator=(const Json& other) {
    copy(other.get(), get());
    return *this;
}

Json& Json::operator=(const JsonConstNode& node) {
    copy(node, get());
    return *this;
}

void copy(JsonConstNode from, JsonNode to)
{
    std::stack<std::tuple<
        JsonConstNode,
        JsonNode
    >> stack;

    do {
        to.set_null();
        if (auto value = from.string_if()) {
            to = *value;
        }
        else if (auto value = from.number_if()) {
            to = *value;
        }
        else if (auto value = from.boolean_if()) {
            to = *value;
        }
        else if (from.is_null()) {
            to.set_null();
        }
        else if (from.is_subtract()) {
            to.set_subtract();
        }
        else if (from.is_object()) {
            to.set_object();
            stack.emplace(from.begin(), to);
        }
        else if (from.is_array()) {
            to.set_array();
            stack.emplace(from.begin(), to);
        }
        else {
            assert(false);
        }

        while (!stack.empty()) {
            auto [from_iter, to_container] = stack.top();
            stack.pop();
            if (!from_iter.valid()) {
                continue;
            }
            from = *from_iter;
            to = to_container[from_iter];
            stack.emplace(++from_iter, to_container);
            break;
        }
    } while (!stack.empty());
}

void diff(JsonConstNode from, JsonConstNode to, JsonNode diff)
{
    assert(from.is_object() && to.is_object());
    diff.set_object(); // May be empty

    std::stack<std::tuple<
        JsonConstNode,
        JsonConstNode,
        JsonNode
    >> stack;
    stack.emplace(from, to, diff);

    while (!stack.empty()) {
        auto [from, to, diff] = stack.top();
        stack.pop();
        for (auto iter = from.begin(); iter.valid(); iter++) {
            auto value = *iter;
            auto find = to.find(iter);
            if (find.valid()) {
                if (!value.compare(*find)) {
                    copy(*find, diff[iter]);
                }
                else if (value.is_container()) {
                    stack.emplace(value, *find, diff[iter]);
                    if (value.is_object()) {
                        diff[iter].set_object();
                    } else {
                        diff[iter].set_array();
                    }
                }
            }
            else {
                diff[iter].set_subtract();
            }
        }
        for (auto iter = to.begin(); iter.valid(); iter++) {
            auto find = from.find(iter);
            if (!find.valid()) {
                copy(*iter, diff[iter]);
            }
        }
    }
}

void apply(JsonConstNode diff, JsonNode to)
{
    assert(diff.is_object() && to.is_object());

    std::stack<std::tuple<
        JsonConstNode,
        JsonNode
    >> stack;
    stack.emplace(diff, to);

    while (!stack.empty()) {
        auto [diff, to] = stack.top();
        stack.pop();
        if (!diff.is_container()) {
            copy(diff, to);
            continue;
        }
        for (auto iter = diff.begin(); iter.valid(); iter++) {
            if ((*iter).is_subtract()) {
                to.erase(to.find(iter));
                continue;
            }
            auto find = to.find(iter);
            if (find.valid()) {
                stack.emplace(*iter, *find);
            }
            else {
                copy(*iter, to[iter]);
            }
        }
    }
}

} // namespace datapack
