#pragma once

#include <assert.h>
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace datapack {

class Object {
public:
  using integer_t = std::int64_t;
  using floating_t = double;
  using binary_t = std::vector<std::uint8_t>;
  struct null_t {};
  struct map_t {};
  struct list_t {};

  using value_t =
      std::variant<integer_t, floating_t, bool, std::string, binary_t, null_t, map_t, list_t>;

  class LookupException : public std::runtime_error {
  public:
    LookupException(const std::string& message) : std::runtime_error(message) {}
  };

  class ValueException : public std::runtime_error {
  public:
    ValueException(const std::string& message) : std::runtime_error(message) {}
  };

private:
  struct Node {
    value_t value;
    std::string key;
    int parent;
    int child;
    int prev;
    int next;
    int child_count;
    Node(const value_t& value, const std::string& key, int parent, int prev) :
        value(value), key(key), parent(parent), child(-1), prev(prev), next(-1), child_count(0) {}
  };

  template <bool IsConst>
  class Iterator_;

  template <bool IsConst>
  class Reference_ {
    using object_t = std::conditional_t<IsConst, const Object*, Object*>;

  public:
    template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
    Reference_(const Reference_<OtherConst>& other) : object(other.object), index(other.index) {}

    const Reference_& operator=(const value_t& value) const;
    const Reference_& operator=(const Reference_<true>& value) const;

    Reference_ operator[](const std::string& key) const;
    Reference_ operator[](std::size_t list_index) const;
    Reference_<IsConst> at(const std::string& key) const;
    Iterator_<IsConst> find(const std::string& key) const;

    Iterator_<IsConst> insert(const std::string& key, const value_t& value) const;
    Iterator_<IsConst> push_back(const value_t& value) const;
    Iterator_<IsConst> erase() const;
    void clear() const;
    std::size_t size() const;

    Object clone() const;

    bool is_map() const { return std::get_if<map_t>(&value()); }
    bool is_list() const { return std::get_if<list_t>(&value()); }
    bool is_null() const { return std::get_if<null_t>(&value()); }
    bool is_primitive() const { return !is_map() && !is_list(); }

    std::conditional_t<IsConst, const integer_t&, integer_t&> integer() const {
      return std::get<integer_t>(value());
    }
    std::conditional_t<IsConst, const integer_t*, integer_t*> integer_if() const {
      return std::get_if<integer_t>(&value());
    }

    std::conditional_t<IsConst, const floating_t&, floating_t&> floating() const {
      return std::get<floating_t>(value());
    }
    std::conditional_t<IsConst, const floating_t*, floating_t*> floating_if() const {
      return std::get_if<floating_t>(&value());
    }

    std::conditional_t<IsConst, const bool&, bool&> boolean() const {
      return std::get<bool>(value());
    }
    std::conditional_t<IsConst, const bool*, bool*> boolean_if() const {
      return std::get_if<bool>(&value());
    }

    std::conditional_t<IsConst, const std::string&, std::string&> string() const {
      return std::get<std::string>(value());
    }
    std::conditional_t<IsConst, const std::string*, std::string*> string_if() const {
      return std::get_if<std::string>(&value());
    }

    std::conditional_t<IsConst, const binary_t&, binary_t&> binary() const {
      return std::get<binary_t>(value());
    }
    std::conditional_t<IsConst, const binary_t*, binary_t*> binary_if() const {
      return std::get_if<binary_t>(&value());
    }

    const std::string& key() const { return object->nodes[index].key; }
    std::conditional_t<IsConst, const value_t&, value_t&> value() const {
      return object->nodes[index].value;
    }
    Iterator_<IsConst> iter() const;

  private:
    Reference_(object_t object, int index) : object(object), index(index) {}
    object_t object;
    int index;

    template <bool OtherConst>
    friend class Iterator_;
    friend class Object;
  };

  template <bool IsConst>
  class Iterator_ {
    using object_t = std::conditional_t<IsConst, const Object*, Object*>;

  public:
    const Reference_<IsConst>& operator*() const { return ref; }
    const Reference_<IsConst>* operator->() const { return &ref; }
    operator bool() const { return valid(); }

    Iterator_ parent() const {
      if (!valid())
        return Iterator_();
      return Iterator_(ref.object, node().parent);
    }
    Iterator_ child() const {
      if (!valid())
        return Iterator_();
      return Iterator_(ref.object, node().child);
    }
    Iterator_ prev() const {
      if (!valid())
        return Iterator_();
      return Iterator_(ref.object, node().prev);
    }
    Iterator_ next() const {
      if (!valid())
        return Iterator_();
      return Iterator_(ref.object, node().next);
    }

    template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
    Iterator_(const Iterator_<OtherConst>& other) : ref(other.ref) {}

    Iterator_() : ref(nullptr, 0) {}

    // The operator= is overloaded for reference to assign the value
    // Therefore, cannot use the default operator= for iterator, since it will
    // use the default operator= for ref.
    // Within Iterator, copy the Reference raw values instead
    // NOTE: It won't compile otherwise, since it will be looking for the operator=
    // implemented for Reference_<true> which isn't implemented.

    Iterator_& operator=(const Iterator_& other) {
      ref.object = other.ref.object;
      ref.index = other.ref.index;
      return *this;
    }
    template <bool OtherConst>
    Iterator_& operator=(Iterator_<OtherConst>&& other) {
      ref.object = other.ref.object;
      ref.index = other.ref.index;
      return *this;
    }
    template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
    Iterator_(Iterator_<OtherConst>&& other) : ref(other.ref.object, other.ref.index) {}

  private:
    Iterator_(object_t object, int index) : ref(object, index) {}

    bool valid() const { return ref.object != nullptr && ref.index != -1; }
    const Node& node() const { return ref.object->nodes[ref.index]; }
    int index() const { return ref.index; }

    Reference_<IsConst> ref;
    friend class Object;
  };

public:
  using Reference = Reference_<false>;
  using ConstReference = Reference_<true>;
  using Iterator = Iterator_<false>;
  using ConstIterator = Iterator_<true>;

  Object();
  Object(ConstReference& value);

  operator Reference();
  operator ConstReference() const;

  Reference operator=(const value_t& value);
  Reference operator=(const ConstReference& value);

  Reference operator[](const std::string& key);
  ConstReference operator[](std::size_t list_index) const;
  Reference operator[](std::size_t list_index);

  ConstIterator find(const std::string key) const;
  Iterator find(const std::string key);
  ConstReference at(const std::string key) const;
  Reference at(const std::string key);

  Iterator insert(const std::string& key, const value_t& value);
  Iterator push_back(const value_t& value);
  Iterator erase();
  void clear();
  std::size_t size() const;

  Object clone() const;

  bool is_map() const { return std::get_if<map_t>(&value()); }
  bool is_list() const { return std::get_if<list_t>(&value()); }
  bool is_null() const { return std::get_if<null_t>(&value()); }
  bool is_primitive() const { return !is_map() && !is_list(); }

  integer_t& integer() { return std::get<integer_t>(value()); }
  const integer_t& integer() const { return std::get<integer_t>(value()); }
  integer_t* integer_if() { return std::get_if<integer_t>(&value()); }
  const integer_t* integer_if() const { return std::get_if<integer_t>(&value()); }

  floating_t& floating() { return std::get<floating_t>(value()); }
  const floating_t& floating() const { return std::get<floating_t>(value()); }
  floating_t* floating_if() { return std::get_if<floating_t>(&value()); }
  const floating_t* floating_if() const { return std::get_if<floating_t>(&value()); }

  bool& boolean() { return std::get<bool>(value()); }
  const bool& boolean() const { return std::get<bool>(value()); }
  bool* boolean_if() { return std::get_if<bool>(&value()); }
  const bool* boolean_if() const { return std::get_if<bool>(&value()); }

  std::string& string() { return std::get<std::string>(value()); }
  const std::string& string() const { return std::get<std::string>(value()); }
  std::string* string_if() { return std::get_if<std::string>(&value()); }
  const std::string* string_if() const { return std::get_if<std::string>(&value()); }

  binary_t& binary() { return std::get<binary_t>(value()); }
  const binary_t& binary() const { return std::get<binary_t>(value()); }
  binary_t* binary_if() { return std::get_if<binary_t>(&value()); }
  const binary_t* binary_if() const { return std::get_if<binary_t>(&value()); }

  const value_t& value() const { return nodes[root_index].value; }
  value_t& value() { return nodes[root_index].value; }

  Iterator iter() { return Iterator(this, root_index); }
  ConstIterator iter() const { return ConstIterator(this, root_index); }

private:
  int add_node(const Node& node);
  int add_child(int parent, const std::string& key = "");
  int get_last_child(int node) const;

  void index_assign(int index, const value_t& value);

  int index_map_access(int parent, const std::string& key) const;
  int index_map_access_or_create(int parent, const std::string& key);
  int index_list_access(int parent, std::size_t index) const;

  int index_insert(int parent, const std::string& key, const value_t& value);
  int index_push_back(int parent, const value_t& value);
  int index_erase(int index);
  void index_clear(int index);

  void index_assign_object(int index, ConstIterator from);

  std::vector<Node> nodes;
  std::stack<int> free;
  int root_index;
};

// How merge and diff work:
// - merge(base, diff) applies "diff" on top of "base"
// - diff(base, modified) returns the difference between the two, such that
//   merge(base, diff(base, modified)) == modified
//   AND
//   diff(base, merge(base, diff)) == diff
//   (assuming diff doesn't contain redundant changes)
// - A diff object has the following properties:
//   - Value at the same address as the base overwrites this value
//   - If a key is present in the diff but not base, it is appended,
//     including parent maps.
//   - For the base/modified, a null value is treated equivalent to if the key
//     is not present. Similarly, if a map only contains null values, it is
//     treated the same as if it wasn't present.
//   - When a null value is in the diff, this means the value is overwritten to
//     null, but instead of setting to null, erases the key/value, since this is
//     treated as equivalent.
//   - Null values in lists will be retained if they aren't at the end of the list
//   - For lists, the diff will contain a map with the following properties:
//     - The keys are the indices in the base object
//     - If an index doesn't have a matching key, then it isn't modified
//     - Indices past the end of the original list will append. If there are gaps,
//       these will be set to null.
//     - If the modified list has fewer elements, then corresponding keys
//       for the erased indices will have null values.
//     - If an element is erased not at the end of the list, this is equivalent
//       to modifying all the subsequent elements to shift them down, and erasing
//       the last element. There is no wasy for diff(base, modified) to distinguish
//       between the two.

Object merge(Object::ConstReference base, Object::ConstReference diff);
Object diff(Object::ConstReference base, Object::ConstReference modified);

bool operator==(const Object::ConstReference& lhs, const Object::ConstReference& rhs);
inline bool operator==(const Object::Reference& lhs, const Object::Reference& rhs) {
  return Object::ConstReference(lhs) == Object::ConstReference(rhs);
}
inline bool operator==(const Object& lhs, const Object& rhs) {
  return Object::ConstReference(lhs) == Object::ConstReference(rhs);
}
std::ostream& operator<<(std::ostream& os, Object::ConstReference object);

} // namespace datapack
