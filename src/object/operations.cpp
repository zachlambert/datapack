#include "datapack/object.hpp"
#include <iomanip>
#include <iostream>
#include <stack>

namespace datapack {

#if 0
Object merge(const Object::ConstReference& base, const Object::ConstReference& diff) {
  Object merged;

  if (base.is_primitive()) {
    merged = diff;
    return merged;
  }

  std::stack<Object::ConstIterator> base_nodes;
  std::stack<Object::ConstIterator> diff_nodes;
  std::stack<Object::Iterator> merged_nodes;

  diff_nodes.push(diff.iter());

  while (!diff_nodes.empty()) {
    auto diff_iter = diff_nodes.top();
    auto base_iter = base_nodes.top();
    diff_iter->is_map();
  }

  return merged;
}

Object diff(const Object::ConstReference& base, const Object::ConstReference& modified) {
  Object diff;
  return diff;
}

bool operator==(const Object::ConstReference& lhs, const Object::ConstReference& rhs) {
  static constexpr double float_threshold = 1e-12;
  std::stack<Object::ConstIterator> nodes_lhs;
  std::stack<Object::ConstIterator> nodes_rhs;
  nodes_lhs.push(lhs.iter());
  nodes_rhs.push(rhs.iter());

  while (!nodes_lhs.empty()) {
    auto lhs = nodes_lhs.top();
    auto rhs = nodes_rhs.top();
    nodes_lhs.pop();
    nodes_rhs.pop();

    if (bool(lhs) != bool(rhs)) {
      return false;
    }
    if (!lhs) { // && !rhs
      continue;
    }

    if (!nodes_lhs.empty() && lhs.parent() && lhs.parent()->is_map()) {
      auto lhs_next = lhs.next();
      nodes_lhs.push(lhs_next);
      nodes_rhs.push(rhs.parent()->find(lhs_next->key()));
    }
    if (!nodes_lhs.empty() && lhs.parent() && lhs.parent()->is_list()) {
      nodes_lhs.push(lhs.next());
      nodes_rhs.push(rhs.next());
    }

    if (lhs->value().index() != rhs->value().index()) {
      return false;
    }

    if (lhs->is_map()) {
      auto lhs_child = lhs.child();
      auto rhs_child = rhs->find(lhs_child->key());
      nodes_lhs.push(lhs_child);
      nodes_rhs.push(rhs_child);
      continue;
    }
    if (lhs->is_list()) {
      nodes_lhs.push(lhs.child());
      nodes_rhs.push(rhs.child());
      continue;
    }

    bool values_equal = std::visit(
        [&rhs](const auto& lhs_value) -> bool {
          using T = std::decay_t<decltype(lhs_value)>;
          auto rhs_value_iter = std::get_if<T>(&rhs->value());
          if (!rhs_value_iter) {
            return false;
          }
          const auto& rhs_value = *rhs_value_iter;

          if constexpr (std::is_same_v<Object::number_t, T>) {
            return std::abs(rhs_value - lhs_value) < float_threshold;
          }
          if constexpr (std::is_same_v<bool, T>) {
            return (rhs_value == lhs_value);
          }
          if constexpr (std::is_same_v<std::string, T>) {
            return (rhs_value == lhs_value);
          }
          if constexpr (std::is_same_v<Object::null_t, T>) {
            return true;
          }
          if constexpr (std::is_same_v<Object::binary_t, T>) {
            if (lhs_value.size() != rhs_value.size()) {
              return false;
            }
            for (std::size_t i = 0; i < lhs_value.size(); i++) {
              if (lhs_value[i] != rhs_value[i]) {
                return false;
              }
            }
            return true;
          }
          if constexpr (std::is_same_v<Object::map_t, T>) {
            return true; // Unreachable
          }
          if constexpr (std::is_same_v<Object::list_t, T>) {
            return true; // Unreachable
          }
        },
        lhs->value());
    if (!values_equal) {
      return false;
    }
  }
  return true;
}

std::ostream& operator<<(std::ostream& os, Object::ConstReference object) {
  std::stack<datapack::Object::ConstIterator> nodes;
  nodes.push(object.iter());
  int depth = 0;
  bool first = true;

  while (!nodes.empty()) {
    auto node = nodes.top();
    nodes.pop();

    if (!node) {
      depth--;
      continue;
    }
    if (!first) {
      os << "\n";
    }
    first = false;

    for (int i = 0; i < depth; i++) {
      os << "    ";
    }
    if (depth > 0) {
      if (!node->key().empty()) {
        os << node->key() << ": ";
      } else {
        os << "- ";
      }
    }

    if (node->is_map()) {
      os << "map:";
    } else if (node->is_list()) {
      os << "list:";
    } else if (auto value = node->number_if()) {
      os << *value;
    } else if (auto value = node->boolean_if()) {
      os << (*value ? "true" : "false");
    } else if (auto value = node->string_if()) {
      os << *value;
    } else if (node->is_null()) {
      os << "null";
    } else if (auto value = node->binary_if()) {
      os << "binary (size=" << value->size() << ")";
    }

    if (depth > 0) {
      nodes.push(node.next());
    }

    if (node->is_map() | node->is_list()) {
      nodes.push(node.child());
      depth++;
    }
  }
  return os;
}
#endif

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
    ConstPtr node;
    std::size_t depth;
    State(ConstPtr node, std::size_t depth) : node(node), depth(depth) {}
  };
  std::stack<State> stack;
  stack.emplace(object.ptr().child(), 1);

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
