#include "datapack/object.hpp"
#include <iomanip>
#include <stack>

namespace datapack {

std::ostream& operator<<(std::ostream& os, const Object& object) {
  struct State {
    int node;
    std::size_t depth;
    State(int node, std::size_t depth) : node(node), depth(depth) {}
  };
  std::stack<State> stack;
  stack.emplace(object.index, 0);

  auto apply_indent = [&os](std::size_t depth) {
    for (std::size_t i = 0; i < depth; i++) {
      os << "  ";
    }
  };

  while (!stack.empty()) {
    auto [node, depth] = stack.top();
    stack.pop();
    const Node& node_d = (*object.node_list)[node];

    apply_indent(depth);
    if (depth != 0) {
      os << "\n";
      if (node_d.key.empty()) {
        os << "- ";
      } else {
        os << node_d.key << ": ";
      }
      if (node_d.next != -1) {
        stack.emplace(node_d.next, depth);
      }
    }

    if (auto value = std::get_if<double>(&node_d.value)) {
      os << *value;
    } else if (auto value = std::get_if<bool>(&node_d.value)) {
      os << (*value ? "true" : "false");
    } else if (auto value = std::get_if<std::string>(&node_d.value)) {
      os << *value;
    } else if (auto value = std::get_if<binary_t>(&node_d.value)) {
      os << "[\n";
      apply_indent(depth + 1);
      os << std::hex;
      for (std::size_t i = 0; i < value->size(); i++) {
        os << std::setfill('0') << std::setw(2) << (*value)[i] << " ";
        if ((i + 1) % 8 == 0) {
          os << "\n";
          apply_indent(depth + 1);
        }
      }
      if (value->size() % 8 != 0) {
        os << "\n";
      }
      apply_indent(depth);
      os << "]";
    } else if (std::get_if<map_t>(&node_d.value)) {
      os << "[map]";
    } else if (std::get_if<list_t>(&node_d.value)) {
      os << "[list]";
    }

    if (node_d.child != -1) {
      stack.emplace(node_d.child, depth + 1);
    }
  }

  return os;
}

} // namespace datapack
