#include "datapack/object.hpp"
#include <iomanip>
#include <stack>

namespace datapack {

static void indent(std::ostream& os, std::size_t depth) {
  for (std::size_t i = 0; i < depth; i++) {
    os << "  ";
  }
}

static void print_value(std::ostream& os, ConstObject value, std::size_t depth) {
  if (auto x = value.number_if()) {
    os << *x;
  } else if (auto x = value.boolean_if()) {
    os << (*x ? "true" : "false");
  } else if (auto x = value.string_if()) {
    os << *x;
  } else if (auto x = value.binary_if()) {
    os << "[binary]\n";
    indent(os, depth + 1);
    const auto flags = os.flags();
    os << std::hex << std::uppercase << std::setfill('0') << std::setw(2);
    for (std::size_t i = 0; i < x->size(); i++) {
      os << int((*x)[i]);
      if (i != x->size() - 1) {
        if ((i + 1) % 8 == 0) {
          os << "\n";
          indent(os, depth + 1);
        } else {
          os << " ";
        }
      }
    }
    os.flags(flags);
  } else if (value.is_map()) {
    os << "[map]";
  } else if (value.is_list()) {
    os << "[list]";
  }
}

std::ostream& operator<<(std::ostream& os, ConstObject object) {
  print_value(os, object, 0);
  if (object.is_primitive()) {
    return os;
  }

  struct State {
    ConstNodeHandle node;
    std::size_t depth;
    State(ConstNodeHandle node, std::size_t depth) : node(node), depth(depth) {}
  };
  std::stack<State> stack;
  stack.emplace(object.handle().child(), 1);

  while (!stack.empty()) {
    auto [node, depth] = stack.top();
    stack.pop();

    os << "\n";
    indent(os, depth);
    if (node.key().empty()) {
      os << "- ";
    } else {
      os << node.key() << ": ";
    }
    auto next = node.next();
    if (next) {
      stack.emplace(next, depth);
    }

    print_value(os, *node, depth);

    auto child = node.child();
    if (child) {
      stack.emplace(child, depth + 1);
    }
  }
  return os;
}

} // namespace datapack
