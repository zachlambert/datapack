#pragma once

#include "datapack/datapack.hpp"
#include <assert.h>
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

template <bool Const, typename T>
using const_value_t = std::conditional_t<Const, const T, T>;

template <bool Const, typename T>
using const_ref_t = std::conditional_t<Const, const T&, T&>;

template <bool Const, typename T>
using const_ptr_t = std::conditional_t<Const, const T*, T*>;

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
    } else {
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
      nodes[to].key = nodes_from[from].key;

      int from_next = nodes_from[from].child;
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

  int find_map_node(int root, const std::string& key) const {
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
    return -1;
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
class MapIterator_;

using ConstMapIterator = MapIterator_<true>;
using MapIterator = MapIterator_<false>;

template <bool Const>
class ListIterator_;

using ListIterator = ListIterator_<false>;
using ConstListIterator = ListIterator_<true>;

template <bool Const>
class Ref_;

using Ref = Ref_<false>;
using ConstRef = Ref_<true>;

template <bool Const>
class Ptr_;

using Ptr = Ptr_<false>;
using ConstPtr = Ptr_<true>;

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

    Ref_<Const> value();

    template <std::size_t Index>
    requires(Index <= 2)
    std::conditional_t<Index == 0, const_ref_t<Const, std::string>, Ref_<Const>> get();

  private:
    PairRef(const_ptr_t<Const, Tree> tree, int node) : tree(tree), node(node) {}
    const_ptr_t<Const, Tree> tree;
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
    Iterator_(const MapIterator_<OtherConst>& other) : parent(other.parent), pair(other.pair) {}

  private:
    Iterator_(const_ptr_t<Const, Tree> tree, int parent, int node) :
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
    return Iterator(tree, *node, (*tree)[*node].child);
  };
  Iterator end() {
    assert_map();
    return Iterator(tree, *node, -1);
  };
  ConstIterator begin() const {
    assert_map();
    return ConstIterator(tree, *node, (*tree)[*node].child);
  };
  ConstIterator end() const {
    assert_map();
    return ConstIterator(tree, *node, -1);
  };
  ConstIterator cbegin() {
    assert_map();
    return ConstIterator(tree, *node, (*tree)[*node].child);
  };
  ConstIterator cend() {
    assert_map();
    return ConstIterator(tree, *node, -1);
  };

private:
  ContainerItems(Tree* tree, const int* node) : tree(tree), node(node) {}

  void assert_map() const {
    if (!std::get_if<map_t>(&(*tree)[*node].value)) {
      throw TypeError("Cannot create an item iterator on a non-map node");
    }
  }

  Tree* tree;
  const int* node;

  friend class Object;
  template <bool Const_>
  friend class Ref_;
};

// ===================================
// ContainerValues

class ContainerValues {
public:
  template <bool Const>
  class Iterator_ {
  public:
    Ref_<Const> operator*() const;
    Ptr_<Const> operator->() const;

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
    Iterator_(const MapIterator_<OtherConst>& other) :
        tree(other.tree), parent(other.parent), node(other.node) {}

  private:
    Iterator_(const_ptr_t<Const, Tree> tree, int parent, int node) :
        tree(tree), parent(parent), node(node) {}

    const_ptr_t<Const, Tree> tree;
    int parent;
    int node;

    friend class ContainerValues;
  };
  using Iterator = Iterator_<false>;
  using ConstIterator = Iterator_<true>;

  Iterator begin() {
    assert_container();
    return Iterator(tree, *node, (*tree)[*node].child);
  };
  Iterator end() {
    assert_container();
    return Iterator(tree, *node, -1);
  };
  ConstIterator begin() const {
    assert_container();
    return ConstIterator(tree, *node, (*tree)[*node].child);
  };
  ConstIterator end() const {
    assert_container();
    return ConstIterator(tree, *node, -1);
  };
  ConstIterator cbegin() {
    assert_container();
    return ConstIterator(tree, *node, (*tree)[*node].child);
  };
  ConstIterator cend() {
    assert_container();
    return ConstIterator(tree, *node, -1);
  };

private:
  ContainerValues(Tree* tree, const int* node) : tree(tree), node(node) {}

  void assert_container() const {
    if (!std::get_if<map_t>(&(*tree)[*node].value) && !std::get_if<list_t>(&(*tree)[*node].value)) {
      throw TypeError("Cannot create an item iterator on a non-container node");
    }
  }

  Tree* tree;
  const int* node;

  friend class Object;
  template <bool Const_>
  friend class Ref_;
};

#define REF_PRIMITIVE_METHODS(Type, name)                                                          \
  const_ref_t<Const, Type> name() {                                                                \
    return std::get<Type>((*tree)[node].value);                                                    \
  }                                                                                                \
  const_ptr_t<Const, Type> name##_if() {                                                           \
    return std::get_if<Type>(&(*tree)[node].value);                                                \
  }

template <bool Const>
class Ref_ {
public:
  template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
  Ref_(const Ref_<OtherConst>& other) :
      tree(other.tree),
      node(other.node),
      values_(const_cast<Tree*>(tree), &node),
      items_(const_cast<Tree*>(tree), &node) {}

  Ref_& operator=(const Ref_& other) {
    static_assert(!Const);
    tree->copy_node(node, other.tree, other.node);
    return *this;
  }

  Ref_& operator=(const primitive_t& value) {
    static_assert(!Const);
    tree->clear_node(node);
    (*tree)[node].value = primitive_to_value(value);
    return *this;
  }

  Ref_ operator[](const std::string& key) {
    if constexpr (!Const) {
      if (is_null()) {
        tree->set_node(node, map_t());
      }
    }
    return Ref_(tree, tree->find_map_node(node, key));
  }

  Ref_ operator[](std::size_t index) const {
    return Ref_(tree, tree->find_list_node(node, index));
  }

  Ref_ at(const std::string& key) const {
    return Ref_(tree, tree->find_map_node(node, key));
  }

  Ptr_<Const> find(const std::string& key);

  void insert(const std::string& key, const primitive_t& value) {
    tree->insert_map_node(node, key, value);
  }
  void push_back(const primitive_t& value) {
    if (is_null()) {
      tree->set_node(node, list_t());
    }
    tree->insert_list_node(node, primitive_to_value(value));
  }

  void insert(const std::string& key, const ConstRef& value) {
    int new_node = tree->insert_map_node(node, key, null_t());
    tree->copy_node(new_node, *value.tree, value.node);
  }
  void push_back(const ConstRef& value) {
    if (is_null()) {
      tree->set_node(node, list_t());
    }
    int new_node = tree->insert_list_node(node, null_t());
    tree->copy_node(new_node, *value.tree, value.node);
  }

  void erase() {
    tree->erase_node(node);
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

  REF_PRIMITIVE_METHODS(number_t, number)
  REF_PRIMITIVE_METHODS(bool, boolean)
  REF_PRIMITIVE_METHODS(std::string, string)
  REF_PRIMITIVE_METHODS(binary_t, binary)

  const_ref_t<Const, ContainerItems> items() const {
    return items_;
  }
  const_ref_t<Const, ContainerValues> values() const {
    return values_;
  }

  Ptr_<Const> ptr() const;

private:
  Ref_(const_ptr_t<Const, Tree> tree, int node) :
      tree(tree),
      node(node),
      items_(const_cast<Tree*>(tree), &node),
      values_(const_cast<Tree*>(tree), &node) {}

  const_ptr_t<Const, Tree> tree;
  int node;
  ContainerItems items_;
  ContainerValues values_;

  template <bool Const_>
  friend class PairRef;
  template <bool Const_>
  friend class Ptr_;

  friend class Object;

  template <bool Const_>
  friend class Ref_;

  template <bool Const_>
  friend class ContainerItems::PairRef;
};

template <bool Const>
Ref_<Const> ContainerItems::PairRef<Const>::value() {
  return Ref_<Const>(tree, node);
}

template <bool Const>
template <std::size_t Index>
requires(Index <= 2)
std::conditional_t<Index == 0, const_ref_t<Const, std::string>, Ref_<Const>> ContainerItems::
    PairRef<Const>::get() {
  static_assert(Index <= 2);
  if constexpr (Index == 0) {
    return (*tree)[node].key;
  }
  if constexpr (Index == 1) {
    return Ref_<Const>(tree, node);
  }
}

// ===================================
// Ptr_

template <bool Const>
class Ptr_ {
public:
  Ref_<Const>& operator*() const {
    return ref;
  }
  Ref_<Const>* operator->() const {
    return &ref;
  }

  operator bool() const {
    return ref.node != -1;
  }

  Ptr_ child() const {
    return Ptr_(ref.tree, (*ref.tree)[ref.node].child);
  }
  Ptr_ next() const {
    return Ptr_(ref.tree, (*ref.tree)[ref.node].next);
  }
  const_ref_t<Const, std::string> key() const {
    return (*ref.tree)[ref.node].key;
  }

  Ptr_() : ref(nullptr, -1) {}

  template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
  Ptr_(const Ptr_<OtherConst>& other) : ref(other.ref) {}

private:
  Ptr_(const_ptr_t<Const, Tree> tree, int node) : ref(tree, node) {}

  mutable Ref_<Const> ref;

  friend class Object;
  template <bool Const_>
  friend class Ref_;
};

template <bool Const>
Ptr_<Const> Ref_<Const>::ptr() const {
  return Ptr_<Const>(tree, node);
}

// ===================================
// Object

#define OBJECT_PRIMITIVE_METHODS(Type, name)                                                       \
  Type& name() {                                                                                   \
    return std::get<Type>(tree[root].value);                                                       \
  }                                                                                                \
  const Type& name() const {                                                                       \
    return std::get<Type>(tree[root].value);                                                       \
  }                                                                                                \
  Type* name##_if() {                                                                              \
    return std::get_if<Type>(&tree[root].value);                                                   \
  }                                                                                                \
  const Type* name##_if() const {                                                                  \
    return std::get_if<Type>(&tree[root].value);                                                   \
  }

class Object {

public:
  Object() : items_(&tree, &root), values_(&tree, &root) {
    tree.insert_node(null_t(), "", -1, -1);
  }

  Object& operator=(const Object& other) {
    tree.copy_node(root, other.tree, other.root);
    return *this;
  }

  Object& operator=(const primitive_t& value) {
    tree.clear_node(root);
    tree[root].value = primitive_to_value(value);
    return *this;
  }

  Ref operator[](const std::string& key) {
    if (is_null()) {
      tree.set_node(root, map_t());
    }
    return Ref(&tree, tree.find_map_node(root, key));
  }

  ConstRef operator[](std::size_t index) const {
    return ConstRef(&tree, tree.find_list_node(root, index));
  }

  ConstRef at(const std::string& key) const {
    return ConstRef(&tree, tree.find_map_node(root, key));
  }

  Ptr find(const std::string& key) {
    return Ptr(&tree, tree.find_map_node(root, key));
  }
  ConstPtr find(const std::string& key) const {
    return ConstPtr(&tree, tree.find_map_node(root, key));
  }

  void insert(const std::string& key, const primitive_t& value) {
    tree.insert_map_node(root, key, primitive_to_value(value));
  }
  void push_back(const primitive_t& value) {
    if (is_null()) {
      tree.set_node(root, list_t());
    }
    tree.insert_list_node(root, primitive_to_value(value));
  }

  void insert(const std::string& key, const ConstRef& value) {
    int new_node = tree.insert_map_node(root, key, null_t());
    tree.copy_node(new_node, *value.tree, value.node);
  }
  void push_back(const ConstRef& value) {
    if (is_null()) {
      tree.set_node(root, list_t());
    }
    int new_node = tree.insert_list_node(root, null_t());
    tree.copy_node(new_node, *value.tree, value.node);
  }

  bool is_map() const {
    return std::get_if<map_t>(&tree[root].value);
  }
  bool is_list() const {
    return std::get_if<list_t>(&tree[root].value);
  }
  bool is_null() const {
    return std::get_if<null_t>(&tree[root].value);
  }
  bool is_primitive() const {
    return !is_map() && !is_list() && !is_null();
  }

  OBJECT_PRIMITIVE_METHODS(number_t, number)
  OBJECT_PRIMITIVE_METHODS(bool, boolean)
  OBJECT_PRIMITIVE_METHODS(std::string, string)
  OBJECT_PRIMITIVE_METHODS(binary_t, binary)

  ContainerItems& items() {
    return items_;
  }
  const ContainerItems& items() const {
    return items_;
  }
  ContainerValues& values() {
    return values_;
  }
  const ContainerValues& values() const {
    return values_;
  }

  Ptr ptr() {
    return Ptr(&tree, root);
  }
  ConstPtr ptr() const {
    return ConstPtr(&tree, root);
  }

private:
  Tree tree;
  const int root = 0;
  ContainerItems items_;
  ContainerValues values_;
};

std::ostream& operator<<(std::ostream& os, const Object& object);
std::ostream& operator<<(std::ostream& os, const ConstRef& ref);

} // namespace datapack

namespace std {

template <bool Const>
struct tuple_size<::datapack::ContainerItems::PairRef<Const>> {
  static constexpr size_t value = 2;
};

template <bool Const>
struct tuple_element<0, ::datapack::ContainerItems::PairRef<Const>> {
  using type = std::string;
};

template <bool Const>
struct tuple_element<1, ::datapack::ContainerItems::PairRef<Const>> {
  using type = ::datapack::Ref_<Const>;
};

} // namespace std
