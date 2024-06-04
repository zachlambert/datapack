#include <datapack/schema.hpp>
#include <datapack/format/binary.hpp>
#include <datapack/util/object.hpp>
#include <datapack/examples/entity.hpp>

int main() {
    Entity entity = Entity::example();
    auto data = datapack::write_binary(entity);

    auto schema = datapack::create_schema<Entity>();
    auto object_loaded = datapack::read_object(schema, data);
    std::cout << "Loaded via schema:\n------\n";
    std::cout << object_loaded << "\n------" << std::endl;

    auto object_direct = datapack::write_object(entity);
    std::cout << "Written directly:\n------\n";
    std::cout << object_direct << "\n------" << std::endl;

    std::cout << "Equal ? " << (compare(object_loaded, object_direct) ? "yes" : "no") << std::endl;

    return 0;
}
