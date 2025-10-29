#pragma once

#include "datapack/datapack.hpp"
#include <assert.h>
#include <concepts>
#include <memory>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace datapack {

using number_t = double;
using binary_t = std::vector<std::uint8_t>;
struct null_t {};
struct map_t {};
struct list_t {};

using primitive_t = std::variant<int, double, bool, std::string, binary_t>;
using value_t = std::variant<number_t, bool, std::string, binary_t, null_t, map_t, list_t>;

inline value_t primitive_to_value(const primitive_t& value) {
  return std::visit(
      [](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
          return value_t(double(value));
        }
        if constexpr (!std::is_same_v<T, int>) {
          return value_t(value);
        }
      },
      value);
}

struct Node {
  value_t value;
  std::string key;
  int parent;
  int child;
  int prev;
  int next;
  Node(const value_t& value, const std::string& key, int parent, int prev) :
      value(value), key(key), parent(parent), child(-1), prev(prev), next(-1) {}
};

class NodeList {
public:
  const Node& operator[](std::size_t i) const { return data[i]; }
  Node& operator[](std::size_t i) { return data[i]; }

  template <typename... Args>
  int emplace(Args&&... args) {
    int index = 0;
    if (free.empty()) {
      index = data.size();
      data.emplace_back(std::forward<Args>(args)...);
    } else {
      index = free.back();
      free.pop_back();
      data[index] = Node(std::forward<Args>(args)...);
    }
    return index;
  }

  void pop(int index) { free.push_back(index); }

  std::size_t size() const { return data.size() - free.size(); }

private:
  std::vector<Node> data;
  std::vector<int> free;
};

template <bool IsConst, typename T>
using const_value_t = std::conditional_t<IsConst, const T, T>;

template <bool IsConst, typename T>
using const_ref_t = std::conditional_t<IsConst, const T&, T&>;

template <bool IsConst, typename T>
using const_ptr_t = std::conditional_t<IsConst, const T*, T*>;

class KeyError : std::runtime_error {
public:
  KeyError(const std::string& message) : std::runtime_error(message) {}
};

// ===================================
// Forward declarations

template <bool IsConst>
class Iterator_;

using ConstIterator = Iterator_<true>;
using Iterator = Iterator_<false>;

// ===================================
// Object_

class Object {
public:
  Object() : node_list(std::make_shared<NodeList>()), index(0) {
    node_list->emplace(null_t(), "", -1, -1);
  }

  Object(const Object& other) : node_list(std::make_shared<NodeList>()), index(0) {
    node_list->emplace(null_t(), "", -1, -1);
    node_copy(0, other.node_list, other.index);
  }

  Object(const primitive_t& value) {
    node_list = std::make_shared<NodeList>();
    index = 0;
    node_list->emplace(primitive_to_value(value), "", -1, -1);
  }

  Object& operator=(const Object& other) {
    node_copy(index, other.node_list, other.index);
    return *this;
  }

  Object& operator=(const primitive_t& value) {
    node_clear(index);
    (*node_list)[index].value = primitive_to_value(value);
    return *this;
  }

  bool valid() const { return node_list && index >= 0; }

  void node_clear(int index) {
    if (index == -1) {
      return;
    }
    if ((*node_list)[index].child == -1) {
      return;
    }

    int child = (*node_list)[index].child;
    (*node_list)[index].child = -1;

    std::stack<int> to_remove;
    to_remove.push(child);
    while (!to_remove.empty()) {
      int node = to_remove.top();
      to_remove.pop();
      if ((*node_list)[child].next != -1) {
        to_remove.push((*node_list)[child].next);
      }
      if ((*node_list)[child].child != -1) {
        to_remove.push((*node_list)[child].child);
      }
      node_list->pop(node);
    }
  }

  void node_erase(int index) {
    if (index == -1) {
      return;
    }
    node_clear(index);

    int parent = (*node_list)[index].parent;
    if (parent == -1) {
      assert(index == 0);
      node_list.reset();
      index = -1;
      return;
    }

    int prev = (*node_list)[index].prev;
    int next = (*node_list)[index].next;

    if (prev != -1) {
      (*node_list)[prev].next = next;
    } else {
      (*node_list)[parent].child = next;
    }
    if (next != -1) {
      (*node_list)[next].prev = prev;
    }
  }

  int node_insert(const value_t& value, const std::string& key, int parent, int prev) {
    int node = node_list->emplace(value, key, parent, prev);

    if (prev != -1) {
      (*node_list)[node].next = (*node_list)[prev].next;
      (*node_list)[prev].next = node;
    } else {
      (*node_list)[node].next = (*node_list)[parent].child;
      (*node_list)[parent].child = node;
    }

    int next = (*node_list)[node].next;
    if (next != -1) {
      (*node_list)[next].prev = node;
    }

    return node;
  }

  void node_copy(int to, std::shared_ptr<const NodeList> node_list_from, int from) {
    std::stack<std::pair<int, int>> stack;
    stack.emplace(to, from);
    bool at_root = true;

    while (!stack.empty()) {
      auto [to, from] = stack.top();
      stack.pop();

      (*node_list)[to].value = (*node_list_from)[from].value;
      (*node_list)[to].key = (*node_list_from)[from].key;

      int from_next = (*node_list_from)[from].child;
      if (!at_root && from_next != -1) {
        int to_next = node_insert(null_t(), "", (*node_list)[to].parent, to);
        stack.emplace(to_next, from_next);
      }

      int from_child = (*node_list_from)[from].child;
      if (from_child != -1) {
        int to_child = node_insert(null_t(), "", to, -1);
        stack.emplace(to_child, from_child);
      }

      at_root = false;
    }
  }

  Object operator[](const std::string& key) {
    if (!valid()) {
      throw KeyError("Empty object");
    }
    if (is_null()) {
      (*node_list)[index].value = map_t();
    } else if (!is_map()) {
      throw KeyError("Not a map or null");
    }

    int prev = -1;
    int node = (*node_list)[index].child;
    while (node != -1) {
      if ((*node_list)[node].key == key) {
        return Object(node_list, node);
      }
      prev = node;
      node = (*node_list)[node].next;
    }
    int new_node = node_insert(null_t(), key, index, prev);
    return Object(node_list, new_node);
  }

  Object operator[](std::size_t list_index) const {
    if (!valid()) {
      throw KeyError("Empty object");
    }
    if (!is_list()) {
      throw KeyError("Not a list");
    }
    int node = (*node_list)[index].child;
    std::size_t index = 0;
    while (node != -1) {
      if (index == list_index) {
        return Object(node_list, node);
      }
      node = (*node_list)[node].next;
      index++;
    }
    throw KeyError(
        "Requested index " + std::to_string(list_index) + " from list of length " +
        std::to_string(index) + "");
  }

  Object at(const std::string& key) const {
    if (!valid()) {
      throw KeyError("Empty object");
    }
    if (!is_map()) {
      throw KeyError("Not a map");
    }
    int node = (*node_list)[index].child;
    while (node != -1) {
      if ((*node_list)[node].key == key) {
        return Object(node_list, node);
      }
      node = (*node_list)[node].next;
    }
    throw KeyError("Could not find key '" + key + "'");
  }

  Iterator find(const std::string& key);
  ConstIterator find(const std::string& key) const;

  Iterator insert(const std::string& key, const primitive_t& value);
  Iterator push_back(const primitive_t& value);

  Iterator insert(const std::string& key, const Object& value);
  Iterator push_back(const Object& value);

  void erase(const ConstIterator& iterator);

  Iterator begin();
  Iterator end();
  ConstIterator begin() const;
  ConstIterator end() const;
  ConstIterator cbegin();
  ConstIterator cend();

  bool is_map() const { return std::get_if<map_t>(&value()); }
  bool is_list() const { return std::get_if<list_t>(&value()); }
  bool is_null() const { return std::get_if<null_t>(&value()); }
  bool is_primitive() const { return !is_map() && !is_list() && !is_null(); }

  number_t& number() { return std::get<number_t>(value()); }
  const number_t& number() const { return std::get<number_t>(value()); }
  number_t* number_if() { return std::get_if<number_t>(&value()); }
  const number_t* number_if() const { return std::get_if<number_t>(&value()); }

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

private:
  Object(const std::shared_ptr<NodeList>& node_list, int index) :
      node_list(node_list), index(index) {}

  value_t& value() { return (*node_list)[index].value; }
  const value_t& value() const { return (*node_list)[index].value; }

  bool iter_equal(const Object& other) const {
    if (index == -1 && other.index == -1) {
      return true;
    }
    return (index == other.index) && (node_list.get() == other.node_list.get());
  }

  std::shared_ptr<NodeList> node_list;
  int index;

  friend class Pair;
  template <bool IsConst>
  friend class Iterator_;

  template <bool IsConst_>
  friend bool operator==(const Iterator_<IsConst_>& lhs, const Iterator_<IsConst_>& rhs);
};

// ===================================
// Pair

class Pair {
public:
  const std::string& key() const { return (*object.node_list)[object.index].key; }
  std::string& key() { return (*object.node_list)[object.index].key; }

  Object& value() { return object; }
  const Object& value() const { return object; }

  template <std::size_t Index>
  auto get() {
    static_assert(Index < 2);
    if constexpr (Index == 0)
      return key();
    if constexpr (Index == 1)
      return value();
  }

  template <std::size_t Index>
  auto get() const {
    static_assert(Index < 2);
    if constexpr (Index == 0)
      return key();
    if constexpr (Index == 1)
      return value();
  }

  Pair(const Pair& other) : object(other.object.node_list, other.object.index) {}

private:
  Pair(std::shared_ptr<NodeList> node_list, int index) : object(node_list, index) {}
  Object object;

  template <bool IsConst>
  friend class Iterator_;

  friend class Object;

  template <bool IsConst_>
  friend bool operator==(const Iterator_<IsConst_>& lhs, const Iterator_<IsConst_>& rhs);
};

// ===================================
// Iterator_

template <bool IsConst>
class Iterator_ {
public:
  std::conditional_t<IsConst, const Pair&, Pair&> operator*() const { return pair; }
  std::conditional_t<IsConst, const Pair*, Pair*> operator->() const { return &pair; }

  Iterator_& operator++() {
    pair.object.index = (*pair.object.node_list)[pair.object.index].next;
    return *this;
  }
  Iterator_ operator++(int) {
    Iterator_ temp = *this;
    ++(*this);
    return temp;
  }

  template <bool OtherConst, typename = std::enable_if_t<!(!IsConst && OtherConst)>>
  Iterator_(const Iterator_<OtherConst>& other) : pair(other.pair) {}

  Iterator_() : pair(nullptr, -1) {}

private:
  Iterator_(const std::shared_ptr<NodeList>& node_list, int index) : pair(node_list, index) {}
  mutable Pair pair;

  template <bool OtherConst>
  friend class Iterator_;

  friend class Object;

  template <bool IsConst_>
  friend bool operator==(const Iterator_<IsConst_>& lhs, const Iterator_<IsConst_>& rhs);
};

template <bool IsConst>
inline bool operator==(const Iterator_<IsConst>& lhs, const Iterator_<IsConst>& rhs) {
  return lhs.pair.object.iter_equal(rhs.pair.object);
}
template <bool IsConst>
inline bool operator!=(const Iterator_<IsConst>& lhs, const Iterator_<IsConst>& rhs) {
  return !(lhs == rhs);
}

// ===================================
// Object methods that use Iterator

inline Iterator Object::find(const std::string& key) {
  if (!valid()) {
    throw KeyError("Empty object");
  }
  if (!is_map()) {
    throw KeyError("Not a map");
  }
  int prev = -1;
  int node = (*node_list)[index].child;
  while (node != -1) {
    if ((*node_list)[node].key == key) {
      return Iterator(node_list, node);
    }
    prev = node;
    node = (*node_list)[node].next;
  }
  int new_node = node_insert(null_t(), key, index, prev);
  return Iterator(node_list, new_node);
}

inline ConstIterator Object::find(const std::string& key) const {
  return ConstIterator(const_cast<Object&>(*this).find(key));
}

inline Iterator Object::insert(const std::string& key, const primitive_t& value) {
  if (is_null()) {
    (*node_list)[index].value = map_t();
  } else if (!is_map()) {
    throw KeyError("Not a map or null");
  }

  int node = (*node_list)[index].child;
  int prev = -1;
  while (node != -1) {
    if ((*node_list)[node].key == key) {
      throw KeyError("Key '" + key + "' already exists");
    }
    prev = node;
    node = (*node_list)[node].next;
  }
  int new_node = node_insert(primitive_to_value(value), key, index, prev);
  return Iterator(node_list, new_node);
}

inline Iterator Object::push_back(const primitive_t& value) {
  if (is_null()) {
    (*node_list)[index].value = list_t();
  } else if (!is_list()) {
    throw KeyError("Not a list or null");
  }

  int node = (*node_list)[index].child;
  int prev = -1;
  while (node != -1) {
    prev = node;
    node = (*node_list)[node].next;
  }
  int new_node = node_insert(primitive_to_value(value), "", index, prev);
  return Iterator(node_list, new_node);
}

inline Iterator Object::insert(const std::string& key, const Object& value) {
  auto iter = insert(key, 0.0);
  iter->value() = value;
  return iter;
}

inline Iterator Object::push_back(const Object& value) {
  auto iter = push_back(0.0);
  iter->value() = value;
  return iter;
}

inline void Object::erase(const ConstIterator& iterator) {
  if (iterator.pair.object.node_list.get() != node_list.get()) {
    throw KeyError("Tried to erase with an iterator for a different object");
  }
  node_erase(iterator.pair.object.index);
}

inline Iterator Object::begin() { return Iterator(node_list, (*node_list)[index].child); }

inline Iterator Object::end() { return Iterator(); }

inline ConstIterator Object::begin() const {
  return ConstIterator(node_list, (*node_list)[index].child);
}

inline ConstIterator Object::end() const { return ConstIterator(); };

inline ConstIterator Object::cbegin() {
  return ConstIterator(node_list, (*node_list)[index].child);
}
inline ConstIterator Object::cend() { return ConstIterator(); }

std::ostream& operator<<(std::ostream& os, const Object& object);

} // namespace datapack

namespace std {

template <>
struct tuple_size<::datapack::Pair> {
  static constexpr size_t value = 2;
};

template <>
struct tuple_element<0, ::datapack::Pair> {
  using type = std::string;
};

template <>
struct tuple_element<1, ::datapack::Pair> {
  using type = ::datapack::Object;
};

} // namespace std
