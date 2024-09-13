#include "datapack/schema/binary.hpp"

#include "datapack/util/object_reader.hpp"
#include "datapack/util/object_writer.hpp"
#include "datapack/format/binary_reader.hpp"
#include "datapack/format/binary_writer.hpp"


namespace datapack {

Object binary_to_object(const Schema& schema, const std::vector<std::uint8_t>& bytes) {
    BinaryReader reader(bytes);
    Object object;
    ObjectWriter writer(object);
    use_schema(schema, reader, writer);
    return object;
}

std::vector<std::uint8_t> object_to_binary(const Schema& schema, const Object::ConstReference& object) {
    ObjectReader reader(object);
    std::vector<std::uint8_t> bytes;
    BinaryWriter writer(bytes);
    use_schema(schema, reader, writer);
    return bytes;
}

} // namespace datapack
