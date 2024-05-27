#include "example.hpp"
#include <datapack/format/binary.hpp>
#include <datapack/util/debug.hpp>

int main() {
    Entity in = Entity::example();

    std::vector<std::uint8_t> data = datapack::write_binary(in);
    Entity out = datapack::read_binary<Entity>(data);

    std::cout << datapack::debug(in) << std::endl;
    std::cout << datapack::debug(out) << std::endl;
    return 0;
}
