#include "datapack/util/debug.hpp"

namespace datapack {

DebugWriter::DebugWriter(std::ostream& os):
    os(os),
    depth(0)
{}

void DebugWriter::value_i32(std::int32_t value) {
    os << value << ",\n";
}

void DebugWriter::value_i64(std::int64_t value) {
    os << value << ",\n";
}

void DebugWriter::value_u32(std::uint32_t value) {
    os << value << ",\n";
}

void DebugWriter::value_u64(std::uint64_t value) {
    os << value << ",\n";
}


void DebugWriter::value_f32(float value) {
    os << value << ",\n";
}

void DebugWriter::value_f64(double value) {
    os << value << ",\n";
}


void DebugWriter::value_string(const char* value) {
    os << value << ",\n";
}

void DebugWriter::value_bool(bool value) {
    os << (value ? "true" : "false") << ",\n";
}


void DebugWriter::enumerate(int value, const std::span<const char*>& labels) {
    os << "(enum, " << labels[value] << "),\n";
}

void DebugWriter::optional_begin(bool has_value) {
    if (!has_value) {
        os << "(optional, empty),\n";
    } else {
        os << "(optional, has_value) {\n";
        depth++;
        indent();
    }
}

void DebugWriter::optional_end() {
    depth--;
    indent();
    os << "},\n";
}

void DebugWriter::variant_begin(const char* label, const std::span<const char*>& labels) {
    os << "(variant, " << label << ") {\n";
    depth++;
    indent();
}

void DebugWriter::variant_end() {
    depth--;
    indent();
    os << "},\n";
}

void DebugWriter::binary_data(const std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) {
    if (fixed_length) {
        os << "(binary, fixed length = " << length << ", stride = " << stride << "),\n";
    } else {
        os << "(binary, variable length = " << length << ", stride = " << stride << "),\n";
    }
}


void DebugWriter::object_begin(std::size_t size) {
    if (size == 0) {
        os << "(object) {\n";
    } else {
        os << "(object, trivial size = " << size << ") {\n";
    }
    depth++;
}

void DebugWriter::object_end(std::size_t size) {
    depth--;
    indent();
    os << "},\n";
}

void DebugWriter::object_next(const char* key) {
    indent();
    os << key << ": ";
}


void DebugWriter::tuple_begin(std::size_t size) {
    if (size == 0) {
        os << "(tuple) {\n";
    } else {
        os << "(tuple, trivial size = " << size << ") {\n";
    }
    depth++;
}

void DebugWriter::tuple_end(std::size_t size) {
    depth--;
    indent();
    os << "},\n";
}

void DebugWriter::tuple_next() {
    indent();
}


void DebugWriter::list_begin(bool is_trivial) {
    if (is_trivial) {
        os << "(list, trivial) {\n";
    } else {
        os << "(list) {\n";
    }
    depth++;
}

void DebugWriter::list_end() {
    depth--;
    indent();
    os << "},\n";
}

void DebugWriter::list_next() {
    indent();
}


void DebugWriter::indent() {
    for (int i = 0; i < depth; i++) {
        os << "    ";
    }
}

} // namespace datapack
