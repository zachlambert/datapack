#include <datapack/format/binary_schema.hpp>
#include <datapack/format/binary.hpp>
#include "example.hpp"

int main() {
    Entity entity = Entity::example();
    auto data = datapack::write_binary(entity);

    auto schema = datapack::create_binary_schema<Entity>();
    auto object = datapack::load_binary(schema, data);

    std::cout << object << std::endl;

    return 0;
}
