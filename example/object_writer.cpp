#include <datapack/object.hpp>
#include <datapack/util/object.hpp>
#include <datapack/util/debug.hpp>
#include "example.hpp"

int main() {
    Entity entity = Entity::example();

    datapack::Object object;
    datapack::ObjectWriter(object).value(entity);

    std::cout << "TYPE:\n";
    datapack::DebugWriter(std::cout).value(entity);

    std::cout << "OBJECT\n" << object << std::endl;

    return 0;
}
