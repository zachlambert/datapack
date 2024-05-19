#include <datapack/object.hpp>
#include <datapack/util/object.hpp>
#include <datapack/util/debug.hpp>
#include "example.hpp"

int main() {
    Entity in = Entity::example();

    datapack::Object object;
    datapack::ObjectWriter(object).value(in);

    Entity out;
    datapack::ObjectReader(object).value(out);

    std::cout << "IN:\n";
    datapack::DebugWriter(std::cout).value(in);
    std::cout << "OUT:\n";
    datapack::DebugWriter(std::cout).value(in);

    return 0;
}
