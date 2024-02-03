#include "datapack/writer.hpp"
namespace datapack {

void write(Writer& writer, const int& value) {
    writer.i32(value);
}

void write(Writer& writer, const long& value) {
    writer.i64(value);
}

void write(Writer& writer, const float& value) {
    writer.f32(value);
}

void write(Writer& writer, const double& value) {
    writer.f64(value);
}

void write(Writer& writer, const std::string& value) {
    writer.string(value);
}

void write(Writer& writer, const bool& value) {
    writer.boolean(value);
}

void write(Writer& writer, const std::nullopt_t& value) {
    writer.null();
}

void write(Writer& writer, const binary_t& value) {
    writer.binary(value);
}

void write(Writer& writer, const Writeable& value) {
    value.write(writer);
}

} // namespace datapack
