#include <datapack/format/json.hpp>
#include <iostream>

int main() {
    std::cout << "JSON1\n";
    {
        const std::string json =
    R"({
        "a": 1.1,
        "b": 2,
        "c": {
            "foo": "bar"
        }
    })";
        auto object = datapack::load_json(json);
        std::cout << object << std::endl;
    }

    std::cout << "JSON2\n";
    {
        const std::string json = R"("hello")";
        auto object = datapack::load_json(json);
        std::cout << object << std::endl;
    }

    return 0;
}
