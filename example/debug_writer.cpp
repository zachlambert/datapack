#include "example.hpp"
#include <datapack/debug.hpp>

int main() {
    Entity value = Entity::example();
    datapack::DebugWriter(std::cout).value(value);
    return 0;
}
