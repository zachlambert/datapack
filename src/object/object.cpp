#include "datapack/object.hpp"
#include <iomanip>
#include <stack>

namespace datapack {

static void indent(std::ostream& os, std::size_t depth) {
  for (std::size_t i = 0; i < depth; i++) {
    os << "  ";
  }
}

static void print_value(std::ostream& os, ConstRef value, std::size_t depth) {
  if (auto x = value.number_if()) {
    os << *x;
  } else if (auto x = value.boolean_if()) {
    os << (*x ? "true" : "false");
  } else if (auto x = value.string_if()) {
    os << *x;
  } else if (auto x = value.binary_if()) {
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
  } else if (value.is_map()) {
    os << "[map]";
  } else if (value.is_list()) {
    os << "[list]";
  }
}

static void print_ptr(std::ostream& os, ConstPtr ptr) {
  print_value(os, *ptr, 0);
  if (ptr->is_primitive()) {
    return;
  }

  struct State {
    ConstPtr ptr;
    std::size_t depth;
    State(ConstPtr ptr, std::size_t depth) : ptr(ptr), depth(depth) {}
  };
  std::stack<State> stack;
  stack.emplace(ptr.child(), 0);

  while (!stack.empty()) {
    auto [iter, depth] = stack.top();
    stack.pop();

    os << "\n";
    indent(os, depth);
    if (ptr.key().empty()) {
      os << "- ";
    } else {
      os << ptr.key() << ": ";
    }
    auto next = ptr.next();
    if (next) {
      stack.emplace(next, depth);
    }

    print_value(os, *ptr, depth);

    auto child = ptr.child();
    if (child) {
      stack.emplace(child, depth + 1);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Object& object) {
  print_ptr(os, object.ptr());
  return os;
}

std::ostream& operator<<(std::ostream& os, const ConstRef& ref) {
  print_ptr(os, ref.ptr());
  return os;
}

} // namespace datapack
