#include <array>
#include <string_view>
#include <type_traits>

namespace dpack {

// Portable macro to return the pretty function for all supported compilers
#if defined(__GNUC__) || defined(__clang__)
#define DPACK_PRETTY_FUNC __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define DPACK_PRETTY_FUNC __FUNCSIG__
#else
static_assert(false, "Compiler doesn't support type names");
#endif

namespace detail {

/* ====================================================================
 * Extracting type names
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
inline constexpr std::string_view probe_type_name = "dpack::detail::ProbeType";

inline constexpr std::string_view probe_type_name_sig = type_name_sig<ProbeType>();
// Returns std::string_view type_name_raw() [with T = dpack::detail::Probe]"
// type_name_previx ->                                ^
// length - type_name_suffix ->                                           ^

inline constexpr size_t type_name_prefix = probe_type_name_sig.find(probe_type_name);
static_assert(type_name_prefix != std::string_view::npos);

inline constexpr size_t type_name_suffix =
    probe_type_name_sig.size() - type_name_prefix - probe_type_name.size();

template <typename T>
constexpr std::string_view type_name() {
  constexpr std::string_view sig = type_name_sig<T>();
  return sig.substr(type_name_prefix, sig.size() - type_name_prefix - type_name_suffix);
}

/* ===================================================================
 * Extracting enum value names
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
inline constexpr std::string_view probe_value_name = "dpack::detail::ProbeValue::Value";

inline constexpr std::string_view probe_value_name_sig = value_name_sig<ProbeValue::Value>();

inline constexpr size_t value_name_prefix = probe_value_name_sig.find(probe_value_name);
static_assert(value_name_prefix != std::string_view::npos);

inline constexpr size_t value_name_suffix =
    probe_value_name_sig.size() - value_name_prefix - probe_value_name.size();

// NOTE: The above could be used for any arbitrary compile-time value, but
// currently only used to extract enum names
template <auto V>
requires std::is_enum_v<decltype(V)>
constexpr std::string_view enum_value_name() {
  constexpr std::string_view sig = value_name_sig<V>();
  return sig.substr(value_name_prefix, sig.size() - value_name_prefix - value_name_suffix);
}

/* ===================================================================
 * Constexpr string conversions
 *
 * For certain formatting choices (see below), would like constexpr string
 * conversions, such as converting PascalCase to snake_case.
 *
 * Again, surprisingly a lot of this can be done fine with constexpr functions.
 */

constexpr bool is_upper(char c) {
  return c >= 'A' && c <= 'Z';
}
constexpr char to_lower(char c) {
  return is_upper(c) ? char(c - 'A' + 'a') : c;
}

// Returns true if name[i] starts a new word
constexpr bool word_boundary(std::string_view name, size_t i) {
  return i != 0 && is_upper(name[i]);
}

// Returns the expected snake_case(name) size, by counting the words
// and adding this to the original name length
constexpr size_t snake_case_size(std::string_view name) {
  size_t size = name.size();
  for (size_t i = 0; i < name.size(); i++) {
    if (word_boundary(name, i)) {
      size++;
    }
  }
  return size;
}

// Must provide N = snake_case_size(name)
template <size_t N>
constexpr std::array<char, N> snake_case(std::string_view name) {
  std::array<char, N> result;
  size_t pos = 0;
  for (size_t i = 0; i < name.size(); i++) {
    if (word_boundary(name, i)) {
      result[pos++] = '_';
    }
    result[pos++] = to_lower(name[i]);
  }
  return result;
}

// Removes the template arguments and namespaces/scopes (including enum scope)
// eg: std::vector<int>  ->  vector
//     MyEnum::Foo       ->  Foo
constexpr std::string_view unqualified_head(std::string_view name) {
  const size_t template_begin = name.find('<');
  const std::string_view head =
      name.substr(0, template_begin == std::string_view::npos ? name.size() : template_begin);
  const size_t scope = head.rfind("::");
  return scope == std::string_view::npos ? head : head.substr(scope + 2);
}

// The following two structs store the pair {name, label},
// where name  =  unqualified_head(type_name<T> / enum_value_name<V>)
//       label =  snake_case(name)

template <typename T>
struct TypeNameDetails {
  static constexpr std::string_view name = unqualified_head(type_name<T>());
  static constexpr auto label_value = snake_case<snake_case_size(name)>(name);
  static constexpr std::string_view label{label_value.data(), label_value.size()};
};

template <auto V>
struct EnumValueNameDetails {
  static constexpr std::string_view name = unqualified_head(enum_value_name<V>());
  static constexpr auto label_value = snake_case<snake_case_size(name)>(name);
  static constexpr std::string_view label{label_value.data(), label_value.size()};
};

} // namespace detail

template <typename T>
constexpr std::string_view default_type_name() {
  return detail::TypeNameDetails<T>::name;
}

template <typename T>
constexpr std::string_view default_type_label() {
  return detail::TypeNameDetails<T>::label;
}

template <auto V>
requires std::is_enum_v<decltype(V)>
constexpr std::string_view default_enum_value_name() {
  return detail::EnumValueNameDetails<V>::name;
}

template <auto V>
requires std::is_enum_v<decltype(V)>
constexpr std::string_view default_enum_value_label() {
  return detail::EnumValueNameDetails<V>::label;
}

} // namespace dpack
