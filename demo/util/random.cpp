#include <datapack/examples/entity.hpp>
#include <datapack/util/random.hpp>
#include <datapack/util/debug.hpp>
#include <chrono>

int main() {
    auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    srand(timestamp % std::numeric_limits<unsigned int>::max());

    Entity value = datapack::random<Entity>();
    if (value.hitbox.has_value())  {
        std::cout << "HITBOX INDEX: " << value.hitbox.value().index() << std::endl;
    }
    std::cout << "RANDOM:\n" << datapack::debug(value) << std::endl;
    return 0;
}
