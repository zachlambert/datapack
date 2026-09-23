#pragma once

#include "datapack/datapack.hpp"
#include <fmt/format.h>
#include <iostream>

namespace dpack {

// Format specifier applied to every floating point value written by a
// DebugWriter. A separate formatter is needed per value type, so the specifier
// is parsed once for float and once for double.
class FloatFormat {
public:
  constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx) {
    const auto begin = ctx.begin();
    const auto end = f64.parse(ctx);
    fmt::format_parse_context f32_ctx(fmt::string_view(begin, std::size_t(end - begin)));
    f32.parse(f32_ctx);
    return end;
  }

  fmt::format_context::iterator format(float value, fmt::format_context& ctx) const {
    return f32.format(value, ctx);
  }
  fmt::format_context::iterator format(double value, fmt::format_context& ctx) const {
    return f64.format(value, ctx);
  }

private:
  fmt::formatter<float> f32;
  fmt::formatter<double> f64;
};

class DebugWriter : public Writer {
public:
  DebugWriter(fmt::format_context& ctx, const FloatFormat& float_format = FloatFormat());

  // Output iterator past the last character written, to be returned from
  // fmt::formatter::format
  fmt::format_context::iterator iter() const {
    return out;
  }

  void number(NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char* value) override;
  void enumerate(int value, const std::span<const std::string_view>& labels) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override;

  void variant_begin(int value, const std::span<const char*>& labels) override;
  void variant_end() override;

  void object_begin() override;
  void object_next(const char* key) override;
  void object_end() override;

  void tuple_begin() override;
  void tuple_next() override;
  void tuple_end() override;

  void list_begin(size_t) override;
  void list_next() override;
  void list_end() override;

private:
  void indent();

  fmt::format_context& ctx;
  fmt::format_context::iterator out;
  FloatFormat float_format;
  int depth;
};

template <writeable T>
class debug {
public:
  debug(const T& value) : value_(value) {}
  const T& value() const {
    return value_;
  }

private:
  const T& value_;
};

} // namespace dpack

template <dpack::writeable T>
struct fmt::formatter<dpack::debug<T>> {
  dpack::FloatFormat float_fmt; // parsed once, applied to every float/double leaf

  constexpr auto parse(fmt::format_parse_context& ctx) {
    return float_fmt.parse(ctx);
  }

  fmt::format_context::iterator format(const dpack::debug<T>& value, fmt::format_context& ctx)
      const {
    dpack::DebugWriter writer(ctx, float_fmt);
    writer.value(value.value());
    return writer.iter();
  }
};

namespace dpack {

template <writeable T>
std::ostream& operator<<(std::ostream& os, const debug<T>& manipulator) {
  return os << fmt::format("{}", manipulator);
}

} // namespace dpack
