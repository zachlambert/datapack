#include <datapack/format/json.hpp>
#include <datapack/util/debug.hpp>
#include <datapack/examples/entity.hpp>
#include <iostream>

int main() {
    Entity in = Entity::example();
    const std::string json = datapack::write_json(in);
    std::cout << json << std::endl;
    Entity out = datapack::read_json<Entity>(json);
    std::cout << "EQUAL ? " << (compare(in, out) ? "yes" : "no") << std::endl;
    return 0;
}
