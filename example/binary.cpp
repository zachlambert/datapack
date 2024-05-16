#include "example.hpp"
#include <datapack/binary.hpp>
#include <datapack/util/debug.hpp>

int main() {
    Entity in = Entity::example();

    std::vector<std::uint8_t> data;
    datapack::BinaryWriter(data).value(in);

    Entity out;
    datapack::BinaryReader(data).value(out);

    datapack::DebugWriter(std::cout).value(in);
    datapack::DebugWriter(std::cout).value(out);
    return 0;
}
