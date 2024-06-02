#include <datapack/format/binary_schema.hpp>
#include <datapack/format/binary.hpp>
#include "example.hpp"

int main() {
    Entity entity = Entity::example();
    auto data = datapack::write_binary(entity);
    return 0;
}
