#include "example.hpp"
#include <datapack/util/random.hpp>
#include <datapack/util/debug.hpp>
#include <chrono>

int main() {
    auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    srand(timestamp % std::numeric_limits<unsigned int>::max());

    Entity value;
    datapack::RandomReader().value(value);
    datapack::DebugWriter(std::cout).value(value);
    return 0;
}
