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

namespace datapack::object {

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

  void set_node(int index, const value_t& value);
  void clear_node(int index);
  void erase_node(int index);
  int insert_node(const value_t& value, const std::string& key, int parent, int prev);
  void copy_node(int to, const Tree& nodes_from, int from);

  int find_map_node(int root, const std::string& key, bool required = true) const;
  int find_or_create_map_node(int root, const std::string& key);
  int insert_map_node(int root, const std::string& key, const value_t& value);

  int find_list_node(int root, std::size_t index) const;
  int insert_list_node(int root, const value_t& value);

  std::size_t node_child_count(int node) const;

private:
  int emplace_node(const value_t& value, const std::string& key, int parent, int prev);
  void pop_node(int index);

  std::vector<Node> nodes;
  std::vector<int> free;
};

// ===================================
// Forward declarations

template <bool Const>
class Object_;

using Object = Object_<false>;
using ConstObject = Object_<true>;

template <bool Const>
class Ptr_;

using Ptr = Ptr_<false>;
using ConstPtr = Ptr_<true>;

// ===================================
// ItemsWrapper

template <bool Const>
class Item {
public:
  Item(shared_ptr_t<Const, Tree> tree, int node) : tree(tree), node(node) {}

  const_ref_t<Const, std::string> key() const {
    return (*tree)[node].key;
  }

  Object_<Const> value() const;

  template <std::size_t Index>
  requires(Index <= 2)
  std::conditional_t<Index == 0, const_ref_t<Const, std::string>, Object_<Const>> get() const;

  bool is(const Item& other) const {
    if (tree != other.tree || (*tree)[node].parent != (*other.tree)[other.node].parent) {
      throw IteratorError("Cannot compare iterators from different containers");
    }
    return node == other.node;
  }

private:
  shared_ptr_t<Const, Tree> tree;
  int node;

  template <bool Const_>
  friend class Item;

  template <bool Const_>
  friend class ItemsIterator_;
};

template <bool Const>
class ItemsIterator_ {
public:
  const Item<Const>& operator*() const {
    return item;
  }
  const Item<Const>* operator->() const {
    return &item;
  }

  ItemsIterator_& operator++() {
    item.node = (*item.tree)[item.node].next;
    return *this;
  }
  ItemsIterator_ operator++(int) {
    ItemsIterator_ temp = (*this);
    ++(*this);
    return temp;
  }

  friend bool operator==(const ItemsIterator_& lhs, const ItemsIterator_& rhs) {
    return lhs.item.is(rhs.item);
  }
  friend bool operator!=(const ItemsIterator_& lhs, const ItemsIterator_& rhs) {
    return !lhs.item.is(rhs.item);
  }

private:
  ItemsIterator_(shared_ptr_t<Const, Tree> tree, int node) : item(tree, node) {}

  Item<Const> item;

  template <bool Const_>
  friend class ItemsWrapper;
};

template <bool Const>
class ItemsWrapper {
public:
  ItemsWrapper(shared_ptr_t<Const, Tree> tree, const int node) : tree(tree), node(node) {}

  using Iterator = ItemsIterator_<Const>;

  Iterator begin() const {
    assert_map();
    return Iterator(tree, (*tree)[node].child);
  };
  Iterator end() const {
    assert_map();
    return Iterator(tree, -1);
  };

private:
  void assert_map() const {
    if (!std::get_if<map_t>(&(*tree)[node].value)) {
      throw TypeError("Cannot create an items wrapper on a non-map node");
    }
  }

  const shared_ptr_t<Const, Tree> tree;
  const int node;

  template <bool Const_>
  friend class Object_;
};

} // namespace datapack::object

namespace std {

template <bool Const>
struct tuple_size<::datapack::object::Item<Const>> {
  static constexpr size_t value = 2;
};

template <bool Const>
struct tuple_element<0, ::datapack::object::Item<Const>> {
  using type = datapack::object::const_ref_t<Const, std::string>;
};

template <bool Const>
struct tuple_element<1, ::datapack::object::Item<Const>> {
  using type = ::datapack::object::Object_<Const>;
};

} // namespace std

namespace datapack::object {

// ===================================
// ValuesWrapper

template <bool Const>
class ValuesIterator_ {
public:
  Object_<Const> operator*() const;
  Ptr_<Const> operator->() const;

  ValuesIterator_& operator++() {
    node = (*tree)[node].next;
    return *this;
  }
  ValuesIterator_ operator++(int) {
    ValuesIterator_ temp = (*this);
    ++(*this);
    return temp;
  }

  friend bool operator==(const ValuesIterator_& lhs, const ValuesIterator_& rhs) {
    if (lhs.tree != rhs.tree || (*lhs.tree)[lhs.node].parent != (*rhs.tree)[rhs.node].parent) {
      throw IteratorError("Cannot compare iterators from different containers");
    }
    return (lhs.node == rhs.node);
  }

private:
  ValuesIterator_(shared_ptr_t<Const, Tree> tree, int node) : tree(tree), node(node) {}

  shared_ptr_t<Const, Tree> tree;
  int node;

  template <bool Const_>
  friend class ValuesWrapper;
};

template <bool Const>
class ValuesWrapper {
public:
  ValuesWrapper(shared_ptr_t<Const, Tree> tree, int node) : tree(tree), node(node) {}

  using Iterator = ValuesIterator_<Const>;

  Iterator begin() const {
    assert_container();
    return Iterator(tree, (*tree)[node].child);
  };
  Iterator end() const {
    assert_container();
    return Iterator(tree, -1);
  };

private:
  void assert_container() const {
    if (!std::get_if<map_t>(&(*tree)[node].value) && !std::get_if<list_t>(&(*tree)[node].value)) {
      throw TypeError("Cannot create a values wrapper on a non-container node");
    }
  }

  const shared_ptr_t<Const, Tree> tree;
  const int node;
};

// ===================================
// Object_

#define OBJECT_PRIMITIVE_METHODS(Type, name)                                                       \
  const_ref_t<Const, Type> name() const {                                                          \
    return std::get<Type>((*tree)[node].value);                                                    \
  }                                                                                                \
  const_ptr_t<Const, Type> name##_if() const {                                                     \
    return std::get_if<Type>(&(*tree)[node].value);                                                \
  }

template <bool Const>
class Object_ {
  Object_(shared_ptr_t<Const, Tree> tree, int node) : tree(tree), node(node) {}

public:
  Object_() : tree(std::make_shared<Tree>()), node(0) {
    static_assert(!Const);
    if constexpr (!Const) {
      tree->insert_node(null_t(), "", -1, -1);
    }
  }

  template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
  Object_(const Object_<OtherConst>& other) : tree(other.tree), node(other.node) {}

  Object_(const primitive_t& value) : tree(std::make_shared<Tree>()), node(0) {
    auto mutable_tree = std::const_pointer_cast<Tree>(tree);
    mutable_tree->insert_node(primitive_to_value(value), "", -1, -1);
  }

  Object_(std::initializer_list<primitive_t> list) : tree(std::make_shared<Tree>()), node(0) {
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
      tree(std::make_shared<Tree>()), node(0) {
    auto mutable_tree = std::const_pointer_cast<Tree>(tree);
    int map_node = mutable_tree->insert_node(map_t(), "", -1, -1);
    for (const auto& [key, value] : list) {
      mutable_tree->insert_map_node(map_node, key, primitive_to_value(value));
    }
  }

  Object_(std::initializer_list<std::pair<std::string, Object>> list) :
      tree(std::make_shared<Tree>()), node(0) {
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

  const Object_& to_map() const {
    static_assert(!Const);
    tree->set_node(node, map_t());
    return *this;
  }

  const Object_& to_list() const {
    static_assert(!Const);
    tree->set_node(node, list_t());
    return *this;
  }

  const Object_& operator=(const Object_& other) const {
    static_assert(!Const);
    tree->copy_node(node, *other.tree, other.node);
    return *this;
  }

  template <bool OtherConst, typename = std::enable_if_t<!Const && OtherConst>>
  const Object_& operator=(const Object_<OtherConst>& other) const {
    static_assert(!Const);
    tree->copy_node(node, *other.tree, other.node);
    return *this;
  }

  const Object_& operator=(primitive_t value) const {
    static_assert(!Const);
    tree->set_node(node, primitive_to_value(value));
    return *this;
  }

  Object_ operator[](const std::string& key) const {
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

  Object_ operator[](const std::size_t index) const {
    return Object_(tree, tree->find_list_node(node, index));
  }

  Object_ at(const std::string& key) const {
    return Object_(tree, tree->find_map_node(node, key, true));
  }

  Ptr_<Const> find(const std::string& key) const;

  bool contains(const std::string& key) const {
    if (!is_map()) {
      throw TypeError("Can only call contains() on a map node");
    }
    return tree->find_map_node(node, key, false) != -1;
  }

  void insert(const std::string& key, const primitive_t& value) const {
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

  void insert(const std::string& key, const ConstObject& value) const {
    if (is_null()) {
      tree->set_node(node, map_t());
    }
    int new_node = tree->insert_map_node(node, key, null_t());
    tree->copy_node(new_node, *value.tree, value.node);
  }
  void push_back(const ConstObject& value) const {
    if (is_null()) {
      tree->set_node(node, list_t());
    }
    int new_node = tree->insert_list_node(node, null_t());
    tree->copy_node(new_node, *value.tree, value.node);
  }

  void erase() const {
    tree->erase_node(node);
  }

  std::size_t size() const {
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

  ItemsWrapper<Const> items() const {
    return ItemsWrapper<Const>(tree, node);
  }
  ValuesWrapper<Const> values() const {
    return ValuesWrapper<Const>(tree, node);
  }

  ItemsWrapper<true> const_items() const {
    return ItemsWrapper<true>(tree, node);
  }
  ValuesWrapper<true> const_values() const {
    return ValuesWrapper<true>(tree, node);
  }

  Ptr_<Const> ptr() const;

  // Make these classes "public" via Object::

  using Ptr = Ptr_<Const>;
  using ValuesIterator = ValuesIterator_<Const>;
  using ItemsIterator = ValuesIterator_<Const>;

  using TypeError = ::datapack::object::TypeError;
  using KeyError = ::datapack::object::KeyError;
  using UsageError = ::datapack::object::UsageError;
  using IteratorError = ::datapack::object::IteratorError;

private:
  shared_ptr_t<Const, Tree> tree;
  int node;

  template <bool Const_>
  friend class Item;
  template <bool Const_>
  friend class Ptr_;

  template <bool Const_>
  friend class Object_;

  template <bool Const_>
  friend class ValuesIterator_;
};

template <bool Const>
Object_<Const> Item<Const>::value() const {
  return Object_<Const>(tree, node);
}

template <bool Const>
template <std::size_t Index>
requires(Index <= 2)
std::conditional_t<Index == 0, const_ref_t<Const, std::string>, Object_<Const>> Item<Const>::get()
    const {
  static_assert(Index <= 2);
  if constexpr (Index == 0) {
    return (*tree)[node].key;
  }
  if constexpr (Index == 1) {
    return Object_<Const>(tree, node);
  }
}

// ===================================
// Ptr_

template <bool Const>
class Ptr_ {
public:
  Object_<Const> operator*() const {
    return object;
  }
  const Object_<Const>* operator->() const {
    return &object;
  }

  operator bool() const {
    return object.node != -1;
  }

  Ptr_ parent() const {
    return Ptr_(object.tree, (*object.tree)[object.node].parent);
  }
  Ptr_ child() const {
    return Ptr_(object.tree, (*object.tree)[object.node].child);
  }
  Ptr_ prev() const {
    return Ptr_(object.tree, (*object.tree)[object.node].prev);
  }
  Ptr_ next() const {
    return Ptr_(object.tree, (*object.tree)[object.node].next);
  }
  const_ref_t<Const, std::string> key() const {
    return (*object.tree)[object.node].key;
  }

  Ptr_() : object(nullptr, -1) {}

  Ptr_(const Ptr_& other) : object(other.object.tree, other.object.node) {}
  Ptr_(Ptr_&& other) : object(std::move(other.object.tree), other.object.node) {}

  Ptr_& operator=(const Ptr_& other) {
    object.tree = other.object.tree;
    object.node = other.object.node;
    return *this;
  }

  Ptr_& operator=(Ptr_&& other) {
    object.tree = std::move(other.object.tree);
    object.node = other.object.node;
    return *this;
  }

  template <bool OtherConst, typename = std::enable_if_t<!(!Const && OtherConst)>>
  Ptr_(const Ptr_<OtherConst>& other) : object(other.object.tree, other.object.node) {}

private:
  Ptr_(shared_ptr_t<Const, Tree> tree, int node) : object(tree, node) {}

  Object_<Const> object;

  template <bool Const_>
  friend class Object_;

  template <bool Const_>
  friend class ValuesIterator_;

  template <bool Const_>
  friend class Ptr_;
};

template <bool Const>
Ptr_<Const> Object_<Const>::find(const std::string& key) const {
  if (!is_map()) {
    throw TypeError("Can only call find() on a map node");
  }
  return Ptr_<Const>(tree, tree->find_map_node(node, key, false));
}

template <bool Const>
Ptr_<Const> Object_<Const>::ptr() const {
  return Ptr_<Const>(tree, node);
}

template <bool Const>
Object_<Const> ValuesIterator_<Const>::operator*() const {
  return Object_<Const>(tree, node);
}

template <bool Const>
Ptr_<Const> ValuesIterator_<Const>::operator->() const {
  return Ptr_<Const>(tree, node);
}

std::ostream& operator<<(std::ostream& os, ConstObject ref);
bool operator==(ConstObject lhs, ConstObject rhs);
bool operator==(ConstObject lhs, const primitive_t& rhs);

inline bool operator==(const primitive_t& lhs, ConstObject rhs) {
  return (rhs == lhs);
}
inline bool operator!=(ConstObject lhs, ConstObject rhs) {
  return !(lhs == rhs);
}
inline bool operator!=(ConstObject lhs, const primitive_t& rhs) {
  return !(lhs == rhs);
}
inline bool operator!=(const primitive_t& lhs, ConstObject rhs) {
  return !(rhs == lhs);
}

} // namespace datapack::object

namespace datapack {

// Only Object is "public" within the datapack:: namespace

template <bool Const>
using Object_ = object::Object_<Const>;
using Object = Object_<false>;
using ConstObject = Object_<true>;

void prune(Object object);
Object merge(ConstObject base, ConstObject diff);
Object diff(ConstObject base, ConstObject modified);

} // namespace datapack
