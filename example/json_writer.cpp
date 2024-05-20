#include "example.hpp"
#include <datapack/format/json.hpp>
#include <datapack/util/debug.hpp>
#include <iostream>

int main() {
    Entity entity = Entity::example();
    std::string json;
    datapack::JsonWriter(json).value(entity);

    std::cout << "Value:\n";
    datapack::DebugWriter(std::cout).value(entity);

    std::cout << "Json:\n" << json << std::endl;

    return 0;
}
