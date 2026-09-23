#include "datapack/debug.hpp"

#ifdef PRINT_BINARY
#include "datapack/encode/base64.hpp"
#endif

namespace dpack {

DebugWriter::DebugWriter(fmt::format_context& ctx, const FloatFormat& float_format) :
    ctx(ctx), out(ctx.out()), float_format(float_format), depth(0) {}

void DebugWriter::number(NumberType type, const void* value) {
  switch (type) {
  case NumberType::I32:
    out = fmt::format_to(out, "{}", *(const std::int32_t*)value);
    break;
  case NumberType::I64:
    out = fmt::format_to(out, "{}", *(const std::int64_t*)value);
    break;
  case NumberType::U32:
    out = fmt::format_to(out, "{}", *(const std::uint32_t*)value);
    break;
  case NumberType::U64:
    out = fmt::format_to(out, "{}", *(const std::uint64_t*)value);
    break;
  case NumberType::U8:
    out = fmt::format_to(out, "{}", *(const std::uint8_t*)value);
    break;
  case NumberType::F32:
    ctx.advance_to(out);
    out = float_format.format(*(const float*)value, ctx);
    break;
  case NumberType::F64:
    ctx.advance_to(out);
    out = float_format.format(*(const double*)value, ctx);
    break;
  };
  out = fmt::format_to(out, ",\n");
}

void DebugWriter::boolean(bool value) {
  out = fmt::format_to(out, "{},\n", value ? "true" : "false");
}

void DebugWriter::string(const char* value) {
  out = fmt::format_to(out, "{},\n", value);
}

void DebugWriter::enumerate(int value, const std::span<const std::string_view>& labels) {
  out = fmt::format_to(out, "(enum, {} = {}),\n", value, labels[value]);
}

void DebugWriter::binary(const std::span<const std::uint8_t>& data) {
#ifdef PRINT_BINARY
  out = fmt::format_to(
      out,
      "(binary, length = {}, data = '{}'),\n",
      data.size(),
      base64_encode(data));
#else
  out = fmt::format_to(out, "(binary, length = {}),\n", data.size());
#endif
}

void DebugWriter::optional_begin(bool has_value) {
  if (!has_value) {
    out = fmt::format_to(out, "(optional, empty),\n");
  } else {
    out = fmt::format_to(out, "(optional, has_value) {{\n");
    depth++;
    indent();
  }
}

void DebugWriter::optional_end() {
  depth--;
  indent();
  out = fmt::format_to(out, "}},\n");
}

void DebugWriter::variant_begin(int value, const std::span<const char*>& labels) {
  out = fmt::format_to(out, "(variant, {} = {}) {{\n", value, labels[value]);
  depth++;
  indent();
}

void DebugWriter::variant_end() {
  depth--;
  indent();
  out = fmt::format_to(out, "}},\n");
}

void DebugWriter::object_begin() {
  out = fmt::format_to(out, "(object) {{\n");
  depth++;
}

void DebugWriter::object_end() {
  depth--;
  indent();
  out = fmt::format_to(out, "}},\n");
}

void DebugWriter::object_next(const char* key) {
  indent();
  out = fmt::format_to(out, "{}: ", key);
}

void DebugWriter::tuple_begin() {
  out = fmt::format_to(out, "(tuple) {{\n");
  depth++;
}

void DebugWriter::tuple_end() {
  depth--;
  indent();
  out = fmt::format_to(out, "}},\n");
}

void DebugWriter::tuple_next() {
  indent();
}

void DebugWriter::list_begin(size_t) {
  out = fmt::format_to(out, "(list) {{\n");
  depth++;
}

void DebugWriter::list_end() {
  depth--;
  indent();
  out = fmt::format_to(out, "}},\n");
}

void DebugWriter::list_next() {
  indent();
}

void DebugWriter::indent() {
  for (int i = 0; i < depth; i++) {
    out = fmt::format_to(out, "    ");
  }
}

} // namespace dpack
