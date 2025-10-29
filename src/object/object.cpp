#include "datapack/object.hpp"
#include <iomanip>
#include <stack>

namespace datapack {

static void indent(std::ostream& os, std::size_t depth) {
  for (std::size_t i = 0; i < depth; i++) {
    os << "  ";
  }
}

static std::ostream& print_value(std::ostream& os, const Object& object, std::size_t depth) {
  if (auto x = object.number_if()) {
    os << *x;
  } else if (auto x = object.boolean_if()) {
    os << (*x ? "true" : "false");
  } else if (auto x = object.string_if()) {
    os << *x;
  } else if (auto x = object.binary_if()) {
    os << "[\n";
    indent(os, depth + 1);
    os << std::hex;
    for (std::size_t i = 0; i < x->size(); i++) {
      os << std::setfill('0') << std::setw(2) << (*x)[i] << " ";
      if ((i + 1) % 8 == 0) {
        os << "\n";
        indent(os, depth + 1);
      }
    }
    if (x->size() % 8 != 0) {
      os << "\n";
    }
    indent(os, depth);
    os << "]";
  } else if (object.is_map()) {
    os << "[map]";
  } else if (object.is_list()) {
    os << "[list]";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Object& object) {
  print_value(os, object, 0);
  if (object.is_primitive()) {
    return os;
  }

  struct State {
    ConstIterator iter;
    std::size_t depth;
    State(ConstIterator iter, std::size_t depth) : iter(iter), depth(depth) {}
  };
  std::stack<State> stack;
  stack.emplace(object.begin(), 0);

  while (!stack.empty()) {
    auto [iter, depth] = stack.top();
    stack.pop();

    os << "\n";
    indent(os, depth);
    if (iter->key().empty()) {
      os << "- ";
    } else {
      os << iter->key() << ": ";
    }
    auto next = iter;
    next++;
    if (next != object.end()) {
      stack.emplace(next, depth);
    }

    print_value(os, iter->value(), depth);

    auto child = iter->value().begin();
    if (child != object.end()) {
      stack.emplace(child, depth + 1);
    }
  }

  return os;
}

} // namespace datapack
