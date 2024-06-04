#include <datapack/object.hpp>
#include <datapack/util/object.hpp>
#include <datapack/util/debug.hpp>
#include <datapack/examples/entity.hpp>

int main() {
    Entity entity = Entity::example();

    datapack::Object object = datapack::write_object(entity);

    std::cout << "TYPE:\n" << datapack::debug(entity) << std::endl;
    std::cout << "OBJECT\n" << object << std::endl;

    return 0;
}
