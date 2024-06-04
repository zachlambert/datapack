#include <datapack/object.hpp>
#include <datapack/util/object.hpp>
#include <datapack/util/debug.hpp>
#include <datapack/examples/entity.hpp>

int main() {
    Entity in = Entity::example();

    datapack::Object object = datapack::write_object(in);
    Entity out = datapack::read_object<Entity>(object);

    std::cout << "IN:\n" << datapack::debug(in) << std::endl;
    std::cout << "OUT:\n" << datapack::debug(out) << std::endl;
    std::cout << "EQUAL ? " << (compare(in, out) ? "yes" : "no") << std::endl;

    return 0;
}
