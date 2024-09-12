#include "datapack/util/debug.hpp"

namespace datapack {

DebugWriter::DebugWriter(std::ostream& os):
    os(os),
    depth(0)
{}

void DebugWriter::integer(IntType type, const void* value) {
    switch (type) {
        case IntType::I32:
            os << *(const std::int32_t*)value;
            break;
        case IntType::I64:
            os << *(const std::int64_t*)value;
            break;
        case IntType::U32:
            os << *(const std::uint32_t*)value;
            break;
        case IntType::U64:
            os << *(const std::uint64_t*)value;
            break;
        case IntType::U8:
            os << *(const std::uint8_t*)value;
            break;
    };
    os << ",\n";
}

void DebugWriter::floating(FloatType type, const void* value) {
    switch (type) {
        case FloatType::F32:
            os << *(const float*)value;
            break;
        case FloatType::F64:
            os << *(const double*)value;
            break;
    };
    os << ",\n";
}

void DebugWriter::boolean(bool value) {
    os << (value ? "true" : "false") << ",\n";;
}

void DebugWriter::string(const char* value) {
    os << value << ",\n";
}

void DebugWriter::enumerate(int value, const char* label) {
    os << "(enum, " << value << " = " << label << "),\n";
}

void DebugWriter::binary(const std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) {
    if (fixed_length) {
        os << "(binary, fixed length = " << length << ", stride = " << stride << "),\n";
    } else {
        os << "(binary, variable length = " << length << ", stride = " << stride << "),\n";
    }
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

void DebugWriter::variant_begin(int value, const char* label) {
    os << "(variant, " << value << " = " << label << ") {\n";
    depth++;
    indent();
}

void DebugWriter::variant_end() {
    depth--;
    indent();
    os << "},\n";
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
