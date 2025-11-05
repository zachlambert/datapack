#pragma once

#include "datapack/datapack.hpp"
#include <assert.h>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <stack>
#include <stdexcept>
#include <string>
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

// ===================================
// template Const helpers

template <bool Const, typename T>
using const_value_t = std::conditional_t<Const, const T, T>;

template <bool Const, typename T>
using const_ref_t = std::conditional_t<Const, const T&, T&>;

template <bool Const, typename T>
using const_ptr_t = std::conditional_t<Const, const T*, T*>;

template <bool Const, typename T>
using shared_ptr_t = std::conditional_t<Const, std::shared_ptr<const T>, std::shared_ptr<T>>;

// ===================================
// Exceptions

class KeyError : std::runtime_error {
public:
  KeyError(const std::string& message) : std::runtime_error(message) {}
};

class OutOfRangeError : std::runtime_error {
public:
  OutOfRangeError(const std::string& message) : std::runtime_error(message) {}
};

class TypeError : std::runtime_error {
public:
  TypeError(const std::string& message) : std::runtime_error(message) {}
};

class IteratorError : std::runtime_error {
public:
  IteratorError(const std::string& message) : std::runtime_error(message) {}
};

class UsageError : std::runtime_error {
public:
  UsageError(const std::string& message) : std::runtime_error(message) {}
};

// ===================================
// Node and Tree

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

class Tree {
public:
  const Node& operator[](std::size_t i) const {
    return nodes[i];
  }
  Node& operator[](std::size_t i) {
    return nodes[i];
  }

  std::size_t size() const {
    return nodes.size() - free.size();
  }

  void set_node(int index, const value_t& value) {
    clear_node(index);
    nodes[index].value = value;
  }

  void clear_node(int index) {
    if (index == -1) {
      return;
    }
    if (nodes[index].child == -1) {
      return;
    }

    int child = nodes[index].child;
    nodes[index].child = -1;

    std::stack<int> to_remove;
    to_remove.push(child);
    while (!to_remove.empty()) {
      int node = to_remove.top();
      to_remove.pop();
      if (nodes[child].next != -1) {
        to_remove.push(nodes[child].next);
      }
      if (nodes[child].child != -1) {
        to_remove.push(nodes[child].child);
      }
      pop_node(node);
    }
  }

  void erase_node(int index) {
    if (index == -1) {
      return;
    }
    clear_node(index);

    int parent = nodes[index].parent;
    int prev = nodes[index].prev;
    int next = nodes[index].next;

    if (prev != -1) {
      nodes[prev].next = next;
    } else if (parent != -1) {
      nodes[parent].child = next;
    }
    if (next != -1) {
      nodes[next].prev = prev;
    }

    pop_node(index);
  }

  int insert_node(const value_t& value, const std::string& key, int parent, int prev) {
    int node = emplace_node(value, key, parent, prev);

    if (prev != -1) {
      nodes[node].next = nodes[prev].next;
      nodes[prev].next = node;
    } else if (parent != -1) {
      nodes[node].next = nodes[parent].child;
      nodes[parent].child = node;
    }

    int next = nodes[node].next;
    if (next != -1) {
      nodes[next].prev = node;
    }

    return node;
  }

  void copy_node(int to, const Tree& nodes_from, int from) {
    if (&nodes_from == this) {
      if (to == from) {
        throw UsageError("Cannot copy to the same node");
      }
      for (int node = nodes[to].parent; node != -1; node = nodes[node].parent) {
        if (node == from) {
          throw UsageError("Cannot copy from a node to its descendent");
        }
      }
      for (int node = nodes[from].parent; node != -1; node = nodes_from[node].parent) {
        if (node == to) {
          throw UsageError("Cannot copy from a node to its ancestor");
        }
      }
    }

    std::stack<std::pair<int, int>> stack;
    stack.emplace(to, from);
    bool at_root = true;

    while (!stack.empty()) {
      auto [to, from] = stack.top();
      stack.pop();

      nodes[to].value = nodes_from[from].value;
      if (!at_root) {
        nodes[to].key = nodes_from[from].key;
      }

      int from_next = nodes_from[from].next;
      if (!at_root && from_next != -1) {
        int to_next = insert_node(null_t(), "", nodes[to].parent, to);
        stack.emplace(to_next, from_next);
      }

      int from_child = nodes_from[from].child;
      if (from_child != -1) {
        int to_child = insert_node(null_t(), "", to, -1);
        stack.emplace(to_child, from_child);
      }

      at_root = false;
    }
  }

  int find_map_node(int root, const std::string& key, bool required = true) const {
    assert(root != -1);
    if (!std::get_if<map_t>(&nodes[root].value)) {
      throw TypeError("Expected map node");
    }

    int node = nodes[root].child;
    while (node != -1) {
      if (nodes[node].key == key) {
        return node;
      }
      node = nodes[node].next;
    }
    if (required) {
      throw KeyError("Key '" + key + "' does not exist");
    }
    return -1;
  }

  int find_or_create_map_node(int root, const std::string& key) {
    assert(root != -1);
    if (!std::get_if<map_t>(&nodes[root].value)) {
      throw TypeError("Expected map node");
    }

    int node = nodes[root].child;
    int prev = -1;
    while (node != -1) {
      if (nodes[node].key == key) {
        return node;
      }
      prev = node;
      node = nodes[node].next;
    }
    return insert_node(null_t(), key, root, prev);
  }

  int insert_map_node(int root, const std::string& key, const value_t& value) {
    assert(root != -1);
    if (!std::get_if<map_t>(&nodes[root].value)) {
      throw TypeError("Expected map node");
    }

    int node = nodes[root].child;
    int prev = -1;
    while (node != -1) {
      if (nodes[node].key == key) {
        throw TypeError("Element with key '" + key + "' already exists");
      }
      prev = node;
      node = nodes[node].next;
    }
    return insert_node(value, key, root, prev);
  }

  int find_list_node(int root, std::size_t index) const {
    assert(root != -1);
    if (!std::get_if<list_t>(&nodes[root].value)) {
      throw TypeError("Expected list node");
    }

    int node = nodes[root].child;
    std::size_t i = 0;
    while (node != -1 && i < index) {
      node = nodes[node].next;
      i++;
    }
    return node;
  }

  int insert_list_node(int root, const value_t& value) {
    assert(root != -1);
    if (!std::get_if<list_t>(&nodes[root].value)) {
      throw TypeError("Expected list node");
    }

    int node = nodes[root].child;
    int prev = -1;
    while (node != -1) {
      prev = node;
      node = nodes[node].next;
    }
    return insert_node(value, "", root, prev);
  }

  std::size_t node_child_count(int node) const {
    node = nodes[node].child;
    std::size_t count = 0;
    while (node != -1) {
      count++;
      node = nodes[node].next;
    }
    return count;
  }

private:
  template <typename... Args>
  int emplace_node(Args&&... args) {
    int index = 0;
    if (free.empty()) {
      index = nodes.size();
      nodes.emplace_back(std::forward<Args>(args)...);
    } else {
      index = free.back();
      free.pop_back();
      nodes[index] = Node(std::forward<Args>(args)...);
    }
    return index;
  }

  void pop_node(int index) {
    free.push_back(index);
  }

  std::vector<Node> nodes;
  std::vector<int> free;
};

// ===================================
// Forward declare iterators

template <bool Const>
class Object_;

using Object = Object_<false>;
using ConstObject = Object_<true>;

template <bool Const>
class NodeHandle_;

using NodeHandle = NodeHandle_<false>;
using ConstNodeHandle = NodeHandle_<true>;

// ===================================
// ContainerItems

class ContainerItems {
public:
  template <bool Const>
  class Iterator_;

  template <bool Const>
  class PairRef {
  public:
    const_ref_t<Const, std::string> key() {
      return (*tree)[node].key;
    }

    Object_<Const> value();

    template <std::size_t Index>
    requires(Index <= 2)
    std::conditional_t<Index == 0, const_ref_t<Const, std::string>, Object_<Const>> get();

  private:
    PairRef(shared_ptr_t<Const, Tree> tree, int node) : tree(tree), node(node) {}
    shared_ptr_t<Const, Tree> tree;
    int node;

    template <bool Const_>
    friend class ::datapack::ContainerItems::Iterator_;
  };

  template <bool Const>
  class Iterator_ {
  public:
    PairRef<Const>& operator*() const {
      return pair;
    }
    PairRef<Const>* operator->() const {
      return &pair;
    }

    Iterator_& operator++() {
      pair.node = (*pair.tree)[pair.node].next;
      return *this;
    }
    Iterator_ operator++(int) {
      Iterator_ temp = (*this);
      ++(*this);
      return temp;
    }

    friend bool operator==(const Iterator_& lhs, const Iterator_& rhs) {
      return lhs.equal_to(rhs);
    }
    friend bool operator!=(const Iterator_& lhs, const Iterator_& rhs) {
      return !lhs.equal_to(rhs);
    }

    template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
    Iterator_(const Iterator_<OtherConst>& other) : parent(other.parent), pair(other.pair) {}

  private:
    Iterator_(shared_ptr_t<Const, Tree> tree, int parent, int node) :
        parent(parent), pair(tree, node) {}

    bool equal_to(const Iterator_& other) const {
      if (pair.tree != other.pair.tree || parent != other.parent) {
        throw IteratorError("Cannot compare iterators from different containers");
      }
      return (pair.node == other.pair.node);
    }

    int parent;
    mutable PairRef<Const> pair;

    friend class ContainerItems;
  };
  using Iterator = Iterator_<false>;
  using ConstIterator = Iterator_<true>;

  Iterator begin() {
    assert_map();
    return Iterator(tree, node, (*tree)[node].child);
  };
  Iterator end() {
    assert_map();
    return Iterator(tree, node, -1);
  };
  ConstIterator begin() const {
    assert_map();
    return ConstIterator(tree, node, (*tree)[node].child);
  };
  ConstIterator end() const {
    assert_map();
    return ConstIterator(tree, node, -1);
  };
  ConstIterator cbegin() {
    assert_map();
    return ConstIterator(tree, node, (*tree)[node].child);
  };
  ConstIterator cend() {
    assert_map();
    return ConstIterator(tree, node, -1);
  };

private:
  ContainerItems(std::shared_ptr<Tree> tree, const int node) : tree(tree), node(node) {}

  void assert_map() const {
    if (!std::get_if<map_t>(&(*tree)[node].value)) {
      throw TypeError("Cannot create an item iterator on a non-map node");
    }
  }

  std::shared_ptr<Tree> tree;
  const int node;

  template <bool Const_>
  friend class Object_;
};

// ===================================
// ContainerValues

class ContainerValues {
public:
  template <bool Const>
  class Iterator_ {
  public:
    Object_<Const> operator*() const;
    NodeHandle_<Const> operator->() const;

    Iterator_& operator++() {
      node = (*tree)[node].next;
      return *this;
    }
    Iterator_ operator++(int) {
      Iterator_ temp = (*this);
      ++(*this);
      return temp;
    }

    friend bool operator==(const Iterator_& lhs, const Iterator_& rhs) {
      if (lhs.tree != rhs.tree || lhs.parent != rhs.parent) {
        throw IteratorError("Cannot compare iterators from different containers");
      }
      return (lhs.node == rhs.node);
    }

    template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
    Iterator_(const Iterator_<OtherConst>& other) :
        tree(other.tree), parent(other.parent), node(other.node) {}

  private:
    Iterator_(shared_ptr_t<Const, Tree> tree, int parent, int node) :
        tree(tree), parent(parent), node(node) {}

    shared_ptr_t<Const, Tree> tree;
    int parent;
    int node;

    friend class ContainerValues;
  };
  using Iterator = Iterator_<false>;
  using ConstIterator = Iterator_<true>;

  Iterator begin() {
    assert_container();
    return Iterator(tree, node, (*tree)[node].child);
  };
  Iterator end() {
    assert_container();
    return Iterator(tree, node, -1);
  };
  ConstIterator begin() const {
    assert_container();
    return ConstIterator(tree, node, (*tree)[node].child);
  };
  ConstIterator end() const {
    assert_container();
    return ConstIterator(tree, node, -1);
  };
  ConstIterator cbegin() {
    assert_container();
    return ConstIterator(tree, node, (*tree)[node].child);
  };
  ConstIterator cend() {
    assert_container();
    return ConstIterator(tree, node, -1);
  };

private:
  ContainerValues(std::shared_ptr<Tree> tree, int node) : tree(tree), node(node) {}

  void assert_container() const {
    if (!std::get_if<map_t>(&(*tree)[node].value) && !std::get_if<list_t>(&(*tree)[node].value)) {
      throw TypeError("Cannot create an item iterator on a non-container node");
    }
  }

  std::shared_ptr<Tree> tree;
  const int node;

  template <bool Const_>
  friend class Object_;
};

// ===================================
// Object_

#define OBJECT_PRIMITIVE_METHODS(Type, name)                                                       \
  const_ref_t<Const, Type> name() {                                                                \
    return std::get<Type>((*tree)[node].value);                                                    \
  }                                                                                                \
  const_ptr_t<Const, Type> name##_if() {                                                           \
    return std::get_if<Type>(&(*tree)[node].value);                                                \
  }

template <bool Const>
class Object_ {
  Object_(shared_ptr_t<Const, Tree> tree, int node) :
      tree(tree),
      node(node),
      items_(std::const_pointer_cast<Tree>(tree), node),
      values_(std::const_pointer_cast<Tree>(tree), node) {}

public:
  Object_() :
      tree(std::make_shared<Tree>()),
      node(0),
      items_(std::const_pointer_cast<Tree>(tree), node),
      values_(std::const_pointer_cast<Tree>(tree), node) {
    static_assert(!Const);
    if constexpr (!Const) {
      tree->insert_node(null_t(), "", -1, -1);
    }
  }

  template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
  Object_(const Object_<OtherConst>& other) :
      tree(other.tree),
      node(other.node),
      items_(std::const_pointer_cast<Tree>(tree), node),
      values_(std::const_pointer_cast<Tree>(tree), node) {}

  Object_(const primitive_t& value) :
      tree(std::make_shared<Tree>()),
      node(0),
      items_(std::const_pointer_cast<Tree>(tree), node),
      values_(std::const_pointer_cast<Tree>(tree), node) {
    auto mutable_tree = std::const_pointer_cast<Tree>(tree);
    mutable_tree->insert_node(primitive_to_value(value), "", -1, -1);
  }

  Object_(std::initializer_list<primitive_t> list) :
      tree(std::make_shared<Tree>()),
      node(0),
      items_(std::const_pointer_cast<Tree>(tree), node),
      values_(std::const_pointer_cast<Tree>(tree), node) {
    auto mutable_tree = std::const_pointer_cast<Tree>(tree);
    int list_node = mutable_tree->insert_node(list_t(), "", -1, -1);
    for (const auto& value : list) {
      mutable_tree->insert_list_node(list_node, primitive_to_value(value));
    }
  }

  static Object make_list(std::initializer_list<Object> list) {
    Object result;
    for (const auto& value : list) {
      result.push_back(value);
    }
    return result;
  }

  Object_(std::initializer_list<std::pair<std::string, primitive_t>> list) :
      tree(std::make_shared<Tree>()),
      node(0),
      items_(std::const_pointer_cast<Tree>(tree), node),
      values_(std::const_pointer_cast<Tree>(tree), node) {
    auto mutable_tree = std::const_pointer_cast<Tree>(tree);
    int map_node = mutable_tree->insert_node(map_t(), "", -1, -1);
    for (const auto& [key, value] : list) {
      mutable_tree->insert_map_node(map_node, key, primitive_to_value(value));
    }
  }

  Object_(std::initializer_list<std::pair<std::string, Object>> list) :
      tree(std::make_shared<Tree>()),
      node(0),
      items_(std::const_pointer_cast<Tree>(tree), node),
      values_(std::const_pointer_cast<Tree>(tree), node) {
    auto mutable_tree = std::const_pointer_cast<Tree>(tree);
    int map_node = mutable_tree->insert_node(map_t(), "", -1, -1);
    for (const auto& [key, value] : list) {
      int child_node = mutable_tree->insert_map_node(map_node, key, null_t());
      mutable_tree->copy_node(child_node, *value.tree, value.node);
    }
  }

  Object_ clone() const {
    Object_ result;
    result = (*this);
    return result;
  }

  Object_& to_map() {
    static_assert(!Const);
    tree->set_node(node, map_t());
    return *this;
  }

  Object_& to_list() {
    static_assert(!Const);
    tree->set_node(node, list_t());
    return *this;
  }

  Object_& operator=(const Object_& other) {
    static_assert(!Const);
    tree->copy_node(node, *other.tree, other.node);
    return *this;
  }

  Object_& operator=(primitive_t value) {
    static_assert(!Const);
    tree->set_node(node, primitive_to_value(value));
    return *this;
  }

#if 0
  Object_& operator=(std::initializer_list<std::pair<std::string, Object>> list) {
    for (const auto& [key, value] : list) {
      (*this)[key] = value;
    }
    return *this;
  }
#endif

  Object_ operator[](const std::string& key) {
    if constexpr (!Const) {
      if (is_null()) {
        tree->set_node(node, map_t());
      }
      return Object_(tree, tree->find_or_create_map_node(node, key));
    }
    if constexpr (Const) {
      return Object_(tree, tree->find_map_node(node, key));
    }
  }

  Object_ operator[](const std::size_t index) {
    return Object_(tree, tree->find_list_node(node, index));
  }

  Object_ at(const std::string& key) {
    return Object_(tree, tree->find_map_node(node, key, true));
  }

  NodeHandle_<Const> find(const std::string& key);

  bool contains(const std::string& key) {
    if (!is_map()) {
      throw TypeError("Can only call contains() on a map node");
    }
    return tree->find_map_node(node, key, false) != -1;
  }

  void insert(const std::string& key, const primitive_t& value) {
    if (is_null()) {
      tree->set_node(node, map_t());
    }
    tree->insert_map_node(node, key, primitive_to_value(value));
  }
  void push_back(const primitive_t& value) {
    if (is_null()) {
      tree->set_node(node, list_t());
    }
    tree->insert_list_node(node, primitive_to_value(value));
  }

  void insert(const std::string& key, const ConstObject& value) {
    if (is_null()) {
      tree->set_node(node, map_t());
    }
    int new_node = tree->insert_map_node(node, key, null_t());
    tree->copy_node(new_node, *value.tree, value.node);
  }
  void push_back(const ConstObject& value) {
    if (is_null()) {
      tree->set_node(node, list_t());
    }
    int new_node = tree->insert_list_node(node, null_t());
    tree->copy_node(new_node, *value.tree, value.node);
  }

  void erase() {
    tree->erase_node(node);
  }

  std::size_t size() {
    return tree->node_child_count(node);
  }

  bool is_map() const {
    return std::get_if<map_t>(&(*tree)[node].value);
  }
  bool is_list() const {
    return std::get_if<list_t>(&(*tree)[node].value);
  }
  bool is_null() const {
    return std::get_if<null_t>(&(*tree)[node].value);
  }
  bool is_primitive() const {
    return !is_map() && !is_list() && !is_null();
  }

  OBJECT_PRIMITIVE_METHODS(number_t, number)
  OBJECT_PRIMITIVE_METHODS(bool, boolean)
  OBJECT_PRIMITIVE_METHODS(std::string, string)
  OBJECT_PRIMITIVE_METHODS(binary_t, binary)

  const_ref_t<Const, ContainerItems> items() {
    return items_;
  }
  const_ref_t<Const, ContainerValues> values() {
    return values_;
  }

  NodeHandle_<Const> handle() const;

private:
  shared_ptr_t<Const, Tree> tree;
  const int node;
  ContainerItems items_;
  ContainerValues values_;

  template <bool Const_>
  friend class PairRef;
  template <bool Const_>
  friend class NodeHandle_;

  template <bool Const_>
  friend class Object_;

  template <bool Const_>
  friend class ContainerItems::PairRef;

  template <bool Const_>
  friend class ContainerValues::Iterator_;
};

template <bool Const>
Object_<Const> ContainerItems::PairRef<Const>::value() {
  return Object_<Const>(tree, node);
}

template <bool Const>
template <std::size_t Index>
requires(Index <= 2)
std::conditional_t<Index == 0, const_ref_t<Const, std::string>, Object_<Const>> ContainerItems::
    PairRef<Const>::get() {
  static_assert(Index <= 2);
  if constexpr (Index == 0) {
    return (*tree)[node].key;
  }
  if constexpr (Index == 1) {
    return Object_<Const>(tree, node);
  }
}

// ===================================
// NodeHandle_

template <bool Const>
class NodeHandle_ {
public:
  Object_<Const>& operator*() const {
    return ref;
  }
  Object_<Const>* operator->() const {
    return &ref;
  }

  operator bool() const {
    return ref.node != -1;
  }

  NodeHandle_ child() const {
    return NodeHandle_(ref.tree, (*ref.tree)[ref.node].child);
  }
  NodeHandle_ next() const {
    return NodeHandle_(ref.tree, (*ref.tree)[ref.node].next);
  }
  const_ref_t<Const, std::string> key() const {
    return (*ref.tree)[ref.node].key;
  }

  NodeHandle_() : ref(nullptr, -1) {}

  template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
  NodeHandle_(const NodeHandle_<OtherConst>& other) : ref(other.ref) {}

private:
  NodeHandle_(shared_ptr_t<Const, Tree> tree, int node) : ref(tree, node) {}

  mutable Object_<Const> ref;

  template <bool Const_>
  friend class Object_;
};

template <bool Const>
NodeHandle_<Const> Object_<Const>::find(const std::string& key) {
  if (!is_map()) {
    throw TypeError("Can only call find() on a map node");
  }
  return NodeHandle_<Const>(tree, tree->find_map_node(node, key, false));
}

template <bool Const>
NodeHandle_<Const> Object_<Const>::handle() const {
  return NodeHandle_<Const>(tree, node);
}

template <bool Const>
Object_<Const> ContainerValues::Iterator_<Const>::operator*() const {
  return Object_<Const>(tree, node);
}

template <bool Const>
NodeHandle_<Const> ContainerValues::Iterator_<Const>::operator->() const {
  return NodeHandle_<Const>(tree, node);
}

std::ostream& operator<<(std::ostream& os, ConstObject ref);

} // namespace datapack

namespace std {

template <bool Const>
struct tuple_size<::datapack::ContainerItems::PairRef<Const>> {
  static constexpr size_t value = 2;
};

template <bool Const>
struct tuple_element<0, ::datapack::ContainerItems::PairRef<Const>> {
  using type = datapack::const_ref_t<Const, std::string>;
};

template <bool Const>
struct tuple_element<1, ::datapack::ContainerItems::PairRef<Const>> {
  using type = ::datapack::Object_<Const>;
};

} // namespace std
