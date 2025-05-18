#include "datapack/object.hpp"
#include <assert.h>
#include <stack>

namespace datapack {

// ===========================================================================
// Object::Reference_

template <>
const Object::Reference& Object::Reference::operator=(const value_t& value) const {
  object->index_assign(index, value);
  return *this;
}

template <>
const Object::Reference& Object::Reference::operator=(const Object::Reference_<true>& value) const {
  object->index_assign(index, value.iter());
  return *this;
}

template <>
Object::Reference Object::Reference::operator[](const std::string& key) const {
  return Reference_(object, object->index_map_access_or_create(index, key));
}

template <bool IsConst>
Object::Reference_<IsConst> Object::Reference_<IsConst>::operator[](std::size_t list_index) const {
  return Reference_(object, object->index_list_access(index, list_index));
}
template Object::Reference Object::Reference::operator[](std::size_t list_index) const;
template Object::ConstReference Object::ConstReference::operator[](std::size_t list_index) const;

template <bool IsConst>
Object::Iterator_<IsConst> Object::Reference_<IsConst>::find(const std::string& key) const {
  return Iterator_<IsConst>(object, object->index_map_access(index, key));
}
template Object::Iterator Object::Reference::find(const std::string& key) const;
template Object::ConstIterator Object::ConstReference::find(const std::string& key) const;

template <bool IsConst>
Object::Reference_<IsConst> Object::Reference_<IsConst>::at(const std::string& key) const {
  int element_index = object->index_map_access(index, key);
  if (element_index == -1) {
    throw LookupException("Could not find key '" + key + "'");
  }
  return Reference_(object, element_index);
}
template Object::Reference Object::Reference::at(const std::string& key) const;
template Object::ConstReference Object::ConstReference::at(const std::string& key) const;

template <>
Object::Iterator Object::Reference::insert(const std::string& key, const value_t& value) const {
  return Iterator(object, object->index_insert(index, key, value));
}

template <>
Object::Iterator Object::Reference::push_back(const value_t& value) const {
  return Iterator(object, object->index_push_back(index, value));
}

template <>
Object::Iterator Object::Reference::erase() const {
  return Iterator(object, object->index_erase(index));
}

template <>
void Object::Reference::clear() const {
  object->index_clear(index);
}

template <bool IsConst>
std::size_t Object::Reference_<IsConst>::size() const {
  if (!is_map() && !is_list()) {
    throw ValueException("Tried to query size on a node that is not a map or list");
  }
  return object->nodes[index].child_count;
}
template std::size_t Object::Reference::size() const;
template std::size_t Object::ConstReference::size() const;

template <bool IsConst>
Object Object::Reference_<IsConst>::clone() const {
  Object result;
  result.index_assign_object(result.root_index, iter());
  return result;
}
template Object Object::Reference::clone() const;
template Object Object::ConstReference::clone() const;

template <bool IsConst>
Object::Iterator_<IsConst> Object::Reference_<IsConst>::iter() const {
  return Iterator_<IsConst>(object, index);
}
template Object::Iterator Object::Reference::iter() const;
template Object::ConstIterator Object::ConstReference::iter() const;

// ===========================================================================
// Object

Object::Object() : root_index(0) { nodes.push_back(Node(null_t(), "", -1, -1)); }

Object::Object(ConstReference& value) : root_index(0) {
  nodes.push_back(Node(null_t(), "", -1, -1));
  index_assign(root_index, value.iter());
}

Object::operator Reference() { return Reference(this, root_index); }

Object::operator ConstReference() const { return ConstReference(this, root_index); }

Object::Reference Object::operator=(const value_t& value) {
  index_assign(root_index, value);
  return Reference(this, root_index);
}

Object::Reference Object::operator=(const Object::ConstReference& value) {
  index_assign(root_index, value.iter());
  return Reference(this, root_index);
}

Object::Reference Object::operator[](const std::string& key) {
  return Reference(this, index_map_access_or_create(root_index, key));
}

Object::ConstReference Object::operator[](std::size_t list_index) const {
  return ConstReference(this, index_list_access(root_index, list_index));
}

Object::Reference Object::operator[](std::size_t list_index) {
  return Reference(this, index_list_access(root_index, list_index));
}

Object::ConstIterator Object::find(const std::string key) const {
  return ConstIterator(this, index_map_access(root_index, key));
}

Object::Iterator Object::find(const std::string key) {
  return Iterator(this, index_map_access(root_index, key));
}

Object::ConstReference Object::at(const std::string key) const {
  int element_index = index_map_access(root_index, key);
  if (element_index == -1) {
    throw LookupException("Could not find key '" + key + "'");
  }
  return ConstReference(this, element_index);
}

Object::Reference Object::at(const std::string key) {
  int element_index = index_map_access(root_index, key);
  if (element_index == -1) {
    throw LookupException("Could not find key '" + key + "'");
  }
  return Reference(this, element_index);
}

Object::Iterator Object::insert(const std::string& key, const value_t& value) {
  return Iterator(this, index_insert(root_index, key, value));
}

Object::Iterator Object::push_back(const value_t& value) {
  return Iterator(this, index_push_back(root_index, value));
}

Object::Iterator Object::erase() { return Iterator(this, index_erase(root_index)); }

void Object::clear() { index_clear(root_index); }

std::size_t Object::size() const {
  if (!is_map() && !is_list()) {
    throw ValueException("Tried to query size on a node that is not a map or list");
  }
  return nodes[root_index].child_count;
}

Object Object::clone() const {
  Object result;
  result = iter();
  return result;
}

int Object::add_node(const Node& node) {
  if (!free.empty()) {
    int result = free.top();
    free.pop();
    return result;
  }
  int result = nodes.size();
  nodes.push_back(node);
  return result;
}

int Object::add_child(int parent, const std::string& key) {
  int last_child = get_last_child(parent);
  int node = add_node(Node(null_t(), key, parent, last_child));
  if (last_child == -1) {
    nodes[parent].child = node;
  } else {
    nodes[last_child].next = node;
  }
  return node;
}

int Object::get_last_child(int node) const {
  int iter = nodes[node].child;
  int last_child = iter;
  while (iter != -1) {
    last_child = iter;
    iter = nodes[iter].next;
  }
  return last_child;
}

void Object::index_assign(int index, const value_t& value) {
  index_clear(index);
  nodes[index].value = value;
}

int Object::index_map_access(int parent, const std::string& key) const {
  auto iter = ConstIterator(this, parent);
  if (!iter->is_map()) {
    throw ValueException("Tried to access value by key on a non-map node");
  }
  iter = iter.child();

  while (iter) {
    if (iter->key() == key) {
      return iter.index();
    }
    iter = iter.next();
  }
  return -1;
}

int Object::index_map_access_or_create(int parent, const std::string& key) {
  auto iter = Iterator(this, parent);
  if (iter->is_null()) {
    *iter = map_t();
  } else if (!iter->is_map()) {
    throw ValueException("Tried to access value by key on a non-map node");
  }

  int child_index = index_map_access(parent, key);
  if (child_index == -1) {
    child_index = add_child(parent, key);
  }
  return child_index;
}

int Object::index_list_access(int parent, std::size_t index) const {
  auto iter = ConstIterator(this, parent);
  if (!iter->is_list()) {
    throw ValueException("Tried to access value by index on a non-list node");
  }
  iter = iter.child();

  std::size_t i = 0;
  while (iter && i != index) {
    iter = iter.next();
    i++;
  }
  return iter.index();
}

int Object::index_insert(int parent, const std::string& key, const value_t& value) {
  auto iter = Iterator(this, parent);
  if (iter->is_null()) {
    *iter = map_t();
  } else if (!iter->is_map()) {
    throw ValueException("Tried to call insert on a non-map node");
  }

  iter = Iterator(this, add_child(parent, key));
  *iter = value;
  return iter.index();
}

int Object::index_push_back(int parent, const value_t& value) {
  auto iter = Iterator(this, parent);
  if (iter->is_null()) {
    *iter = list_t();
  } else if (!iter->is_list()) {
    throw ValueException("Tried to call push_back on a non-list node");
  }

  iter = Iterator(this, add_child(parent));
  *iter = value;
  return iter.index();
}

int Object::index_erase(int index) {
  index_clear(index);
  if (int prev = nodes[index].prev; prev != -1) {
    nodes[prev].next = nodes[index].next;
  }
  if (int next = nodes[index].next; next != -1) {
    nodes[next].prev = nodes[index].prev;
  }
  if (int parent = nodes[index].parent; parent != -1 && nodes[parent].child == index) {
    nodes[parent].child = nodes[index].next;
  }

  int after = nodes[index].next;

  if (index == nodes.size() - 1) {
    nodes.pop_back();
  } else {
    free.push(index);
  }

  return after;
}

void Object::index_clear(int index) {
  int iter = nodes[index].child;
  while (iter != -1) {
    int prev = iter;
    iter = nodes[iter].next;
    index_erase(prev);
  }
}

void Object::index_assign_object(int index, ConstIterator from) {
  if (!from->is_map() && !from->is_list()) {
    index_assign(index, from->value());
    return;
  }
  if (!from.child()) {
    index_assign(index, null_t());
    return;
  }

  std::stack<ConstIterator> from_stack;
  from_stack.push(from);
  std::stack<Iterator> to_stack;
  to_stack.push(Iterator(this, index));

  while (!from_stack.empty()) {
    auto from = from_stack.top();
    from_stack.pop();
    if (!from) {
      to_stack.pop();
      continue;
    }
    from_stack.push(from.next());

    auto to = to_stack.top();
    Iterator new_to;
    if (to->is_map()) {
      new_to = to->insert(from->key(), from->value());
    } else if (to->is_list()) {
      new_to = to->push_back(from->value());
    } else {
      assert(false);
    }

    if (from->is_map() || from->is_list()) {
      from_stack.push(from.child());
      to_stack.push(new_to);
    }
  }
}

} // namespace datapack
