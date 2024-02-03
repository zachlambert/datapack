#include <string>
#include <variant>
#include <optional>
#include <stack>
#include <sstream>
#include <charconv>
#include <iostream>
#include <memory>

#include <datapack/writer.hpp>
#include <datapack/json.hpp>
#include <datapack/yaml.hpp>

int main() {
    std::string json = R"({
    "a": {
        "1": [ 1, 2, "asdf" ],
        "2": null
    },
    "b": {
        "one": { "three": { "four": "five" }}
    }
})";

    datapack::Object object = datapack::parse(datapack::JsonParser(json));
    std::cout << object.tokens.size() << std::endl;
    datapack::YamlWriter writer;
    writer.value(object);

    std::cout << "In:\n" << json << std::endl;
    std::cout << "Out:\n" << writer.result() << std::endl;

    return 0;
}
