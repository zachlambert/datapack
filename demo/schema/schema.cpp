#include <datapack/schema.hpp>
#include <datapack/format/binary.hpp>
#include <datapack/util/object.hpp>
#include <datapack/util/debug.hpp>
#include <datapack/examples/entity.hpp>

int main() {
    Entity entity = Entity::example();
    auto data = datapack::write_binary(entity);

    auto schema = datapack::create_schema<Entity>();
    // auto object_loaded = datapack::read_object(schema, data);
    std::cout << "Loaded via schema:\n------\n";
    datapack::use_schema(schema, datapack::BinaryReader(data), datapack::DebugWriter(std::cout));
    // std::cout << object_loaded << "\n------" << std::endl;

#if 0
    auto object_direct = datapack::write_object(entity);
    std::cout << "Written directly:\n------\n";
    std::cout << object_direct << "\n------" << std::endl;

    // std::cout << "Equal ? " << (compare(object_loaded, object_direct) ? "yes" : "no") << std::endl;

    auto another_schema = datapack::create_schema<Entity>();
    std::cout << "Schema equal: " << (another_schema == schema ? "yes" : "no") << std::endl;

#endif
    return 0;
}
