#include "datapack/reader.hpp"

namespace datapack {

void read(Reader& reader, int& value) {
    reader.i32(value);
}

void read(Reader& reader, long& value) {
    reader.i64(value);
}

void read(Reader& reader, float& value) {
    reader.f32(value);
}

void read(Reader& reader, double& value) {
    reader.f64(value);
}

void read(Reader& reader, std::string& value) {
    reader.string(value);
}

void read(Reader& reader, bool& value) {
    reader.boolean(value);
}

void read(Reader& reader, binary_t& value) {
    reader.binary(value);
}

void read(Reader& reader, Readable& value) {
    value.read(reader);
}

} // namespace datapack
