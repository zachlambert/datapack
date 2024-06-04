#include <datapack/examples/entity.hpp>
#include <datapack/util/debug.hpp>

int main() {
    Entity value = Entity::example();
    std::cout << datapack::debug(value) << std::endl;
    return 0;
}
