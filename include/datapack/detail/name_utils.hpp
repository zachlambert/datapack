#pragma once

#include <array>
#include <string_view>

// Portable macro to return the pretty function for all supported compilers
#if defined(__GNUC__) || defined(__clang__)
#define DPACK_PRETTY_FUNC __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define DPACK_PRETTY_FUNC __FUNCSIG__
#else
static_assert(false, "Compiler doesn't support type names");
#endif

namespace dpack::name_utils {

/* ====================================================================
 * Extracting type names
 *   std::string_view type_name<T>()
 *
 * The DPACK_PRETTY_FUNC macro gives the following results:
 *     std::string func(int num)  =>  "std::string func(int)"
 *     int func(int num)          =>  "int func(int)"
 *     void func(T)  =>  "void func(T) [with T = <the type>]"
 *     T func()      =>  "T func() [with T = <the type>]"
 *
 * For concrete functions, the type name is at the start.
 * For templated functions, which is needed to make this generic, the type is
 * given in the [with T = <the type>] block.
 *
 * Can use a probe and costexpr functions to get compile-time offsets for where
 * this is located Note: Surprisingly, you can do contexpr
 * string_view.find(string_view) !
 *
 * Also since, we need to get a string_view out of the function itself, the
 * signature has to have std::string_view as the return type anyway, so we
 * couldn't have read the type from the return type, it's more convenient for it
 * to be a templated function.
 */

// Returns std::string_view type_name_sig() [with T = <type-name>]"
template <typename T>
constexpr std::string_view type_name_sig() {
  return DPACK_PRETTY_FUNC;
}

struct ProbeType {}; // Arbitrary "probe" type name
inline constexpr std::string_view probe_type_name = "dpack::name_utils::ProbeType";

inline constexpr std::string_view probe_type_name_sig = type_name_sig<ProbeType>();
// Returns std::string_view type_name_raw() [with T = dpack::name_utils::Probe]"
// type_name_previx ->                                ^
// length - type_name_suffix ->                                           ^

inline constexpr size_t type_name_prefix = probe_type_name_sig.find(probe_type_name);
static_assert(type_name_prefix != std::string_view::npos);

inline constexpr size_t type_name_suffix =
    probe_type_name_sig.size() - type_name_prefix - probe_type_name.size();

template <typename T>
constexpr std::string_view type_name_full() {
  constexpr std::string_view sig = type_name_sig<T>();
  return sig.substr(type_name_prefix, sig.size() - type_name_prefix - type_name_suffix);
}

/* ===================================================================
 * Extracting value names
 *   std::string_view value_name<V>()
 *
 * Templates can also be defined with compile-time constants, not just types.
 * eg:
 * template <int N>    void func();        func<2>();
 * template <MyEnum V> void func();        func<MyEnum::Foo>();
 *
 * Since C++17, you can also use deduced types, which allows supporting
 * arbitrary compile-time constants in the template parameter list. template
 * <auto V>   void func() { using T = std::decltype(V); }
 *
 * The signature above converts to:
 * "void func() [with auto V = <value-name>]"
 * eg:
 * "void func() [with auto V = MyEnum::Foo]"
 *
 * Therefore, the exact same logic can be used as before, but with <auto V>
 * instead.
 */

template <auto V>
constexpr std::string_view value_name_sig() {
  return DPACK_PRETTY_FUNC;
}

enum class ProbeValue { Value };
inline constexpr std::string_view probe_value_name = "dpack::name_utils::ProbeValue::Value";

inline constexpr std::string_view probe_value_name_sig = value_name_sig<ProbeValue::Value>();

inline constexpr size_t value_name_prefix = probe_value_name_sig.find(probe_value_name);
static_assert(value_name_prefix != std::string_view::npos);

inline constexpr size_t value_name_suffix =
    probe_value_name_sig.size() - value_name_prefix - probe_value_name.size();

template <auto V>
constexpr std::string_view value_name_full() {
  constexpr std::string_view sig = value_name_sig<V>();
  return sig.substr(value_name_prefix, sig.size() - value_name_prefix - value_name_suffix);
}

/* ===================================================================
 * Consteval string conversions
 */

// Removes "<prefix>::<type>"
consteval std::string_view remove_scope(std::string_view name) {
  const size_t last_scope = name.rfind("::");
  return last_scope == std::string_view::npos ? name : name.substr(last_scope + 2);
}

consteval bool is_upper(char c) {
  return c >= 'A' && c <= 'Z';
}

consteval char to_lower(char c) {
  return is_upper(c) ? char(c - 'A' + 'a') : c;
}

consteval bool is_alnum(char c) {
  return is_upper(c) || c >= 'a' && c <= 'z' || c >= '0' && c <= '9';
}

consteval bool is_simple_name(std::string_view name) {
  for (size_t i = 0; i < name.size(); i++) {
    if (!is_alnum(name[i])) {
      return false;
    }
  }
  return true;
}

consteval size_t label_size(std::string_view name) {
  size_t size = name.size();
  for (size_t i = 0; i < name.size(); i++) {
    if (i != 0 && is_upper(name[i])) {
      size++;
    }
  }
  return size;
}

// Must provide N = label_size(name)
template <size_t N>
consteval std::array<char, N> name_to_label(std::string_view name) {
  if (!is_simple_name(name)) {
    throw "name must be PascalCase, camelCase or lowercase to support automatic names";
  }
  std::array<char, N> result;
  size_t pos = 0;
  for (size_t i = 0; i < name.size(); i++) {
    if (i != 0 && is_upper(name[i])) {
      result[pos++] = '_';
    }
    result[pos++] = to_lower(name[i]);
  }
  return result;
}

} // namespace dpack::name_utils
