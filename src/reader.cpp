#include "datapack/reader.hpp"

namespace datapack {

void read(Reader& reader, int& value) {
    value = reader.i32();
}

void read(Reader& reader, long& value) {
    value = reader.i64();
}

void read(Reader& reader, float& value) {
    value = reader.f32();
}

void read(Reader& reader, double& value) {
    value = reader.f64();
}

void read(Reader& reader, std::string& value) {
    value = reader.string();
}

void read(Reader& reader, bool& value) {
    value = reader.boolean();
}

void read(Reader& reader, binary_t& value) {
    value = reader.binary();
}

void read(Reader& reader, Readable& value) {
    value.read(reader);
}

} // namespace datapack
