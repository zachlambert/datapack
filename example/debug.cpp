#include "example.hpp"
#include <datapack/util/debug.hpp>

int main() {
    Entity value = Entity::example();
    datapack::DebugWriter(std::cout).value(value);
    return 0;
}
