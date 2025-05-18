#include "datapack/debug.hpp"

#ifdef PRINT_BINARY
#include "datapack/encode/base64.hpp"
#endif

namespace datapack {

DebugWriter::DebugWriter(std::ostream& os) : os(os), depth(0) {}

void DebugWriter::number(NumberType type, const void* value) {
  switch (type) {
  case NumberType::I32:
    os << *(const std::int32_t*)value;
    break;
  case NumberType::I64:
    os << *(const std::int64_t*)value;
    break;
  case NumberType::U32:
    os << *(const std::uint32_t*)value;
    break;
  case NumberType::U64:
    os << *(const std::uint64_t*)value;
    break;
  case NumberType::U8:
    os << *(const std::uint8_t*)value;
    break;
  case NumberType::F32:
    os << *(const float*)value;
    break;
  case NumberType::F64:
    os << *(const double*)value;
    break;
  };
  os << ",\n";
}

void DebugWriter::boolean(bool value) {
  os << (value ? "true" : "false") << ",\n";
  ;
}

void DebugWriter::string(const char* value) { os << value << ",\n"; }

void DebugWriter::enumerate(int value, const char* label) {
  os << "(enum, " << value << " = " << label << "),\n";
}

void DebugWriter::binary(const std::span<const std::uint8_t>& data) {
#ifdef PRINT_BINARY
  os << "(binary, length = " << data.size() << ", data = '" << base64_encode(data) << "'),\n";
#else
  os << "(binary, length = " << data.size() << "),\n";
#endif
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

void DebugWriter::object_begin() {
  os << "(object) {\n";
  depth++;
}

void DebugWriter::object_end() {
  depth--;
  indent();
  os << "},\n";
}

void DebugWriter::object_next(const char* key) {
  indent();
  os << key << ": ";
}

void DebugWriter::tuple_begin() {
  os << "(tuple) {\n";
  depth++;
}

void DebugWriter::tuple_end() {
  depth--;
  indent();
  os << "},\n";
}

void DebugWriter::tuple_next() { indent(); }

void DebugWriter::list_begin() {
  os << "(list) {\n";
  depth++;
}

void DebugWriter::list_end() {
  depth--;
  indent();
  os << "},\n";
}

void DebugWriter::list_next() { indent(); }

void DebugWriter::indent() {
  for (int i = 0; i < depth; i++) {
    os << "    ";
  }
}

} // namespace datapack
