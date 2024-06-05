#include <datapack/schema.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/util/debug.hpp>

int main() {
    std::cout << "ENTITY SCHEMA:\n";
    std::cout << datapack::debug(datapack::create_schema<Entity>()) << std::endl;

    std::cout << "SCHEMA SCHEMA:\n";
    std::cout << datapack::debug(datapack::create_schema<datapack::Schema>());
}
