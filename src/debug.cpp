#include "datapack/debug.hpp"

namespace datapack {

void DebugWriter::value_f64(const double& value) {
    os << value << "\n";
}

void DebugWriter::value_i32(const int& value) {
    os << value << "\n";
}

void DebugWriter::optional_begin(bool has_value) {
    if (has_value) {
        os << "optional:\n";
        depth++;
    } else {
        os << "optional: null\n";
    }
}

void DebugWriter::optional_end() {
    depth--;
}

void DebugWriter::variant_begin() {

}

void DebugWriter::variant_match(const char* label) {
    os << "variant[" << label << "]: ";
}

void DebugWriter::variant_end() {

}

void DebugWriter::object_begin() {
    if (depth != 0) {
        os << "\n";
    }
    depth++;
}

void DebugWriter::object_end() {
    depth--;
}

void DebugWriter::object_next(const char* key) {
    indent();
    os << key << ": ";
}

void DebugWriter::indent() {
    for (int i = 0; i < depth; i++) {
        os << "    ";
    }
}

} // namespace datapack
