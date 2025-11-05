#include "datapack/object.hpp"

namespace datapack {
void Tree::set_node(int index, const value_t& value) {
  clear_node(index);
  nodes[index].value = value;
}

void Tree::clear_node(int index) {
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

void Tree::erase_node(int index) {
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

int Tree::insert_node(const value_t& value, const std::string& key, int parent, int prev) {
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

void Tree::copy_node(int to, const Tree& nodes_from, int from) {
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

int Tree::find_map_node(int root, const std::string& key, bool required) const {
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

int Tree::find_or_create_map_node(int root, const std::string& key) {
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

int Tree::insert_map_node(int root, const std::string& key, const value_t& value) {
  assert(root != -1);
  if (!std::get_if<map_t>(&nodes[root].value)) {
    throw TypeError("Expected map node");
  }

  int node = nodes[root].child;
  int prev = -1;
  while (node != -1) {
    if (nodes[node].key == key) {
      throw KeyError("Element with key '" + key + "' already exists");
    }
    prev = node;
    node = nodes[node].next;
  }
  return insert_node(value, key, root, prev);
}

int Tree::find_list_node(int root, std::size_t index) const {
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

int Tree::insert_list_node(int root, const value_t& value) {
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

std::size_t Tree::node_child_count(int node) const {
  node = nodes[node].child;
  std::size_t count = 0;
  while (node != -1) {
    count++;
    node = nodes[node].next;
  }
  return count;
}

int Tree::emplace_node(const value_t& value, const std::string& key, int parent, int prev) {
  int index = 0;
  if (free.empty()) {
    index = nodes.size();
    nodes.emplace_back(value, key, parent, prev);
  } else {
    index = free.back();
    free.pop_back();
    nodes[index] = Node(value, key, parent, prev);
  }
  return index;
}

void Tree::pop_node(int index) {
  free.push_back(index);
}

} // namespace datapack
