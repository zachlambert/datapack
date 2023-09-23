#include "datapack/object.hpp"
#include <iomanip>
#include <stack>

namespace datapack::object {

static bool binary_match(const binary_t& lhs, const binary_t& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.size(); i++) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

bool operator==(ConstObject lhs_root, ConstObject rhs_root) {
  std::stack<std::pair<ConstObject::Ptr, ConstObject::Ptr>> stack;
  stack.emplace(lhs_root.ptr(), rhs_root.ptr());
  bool at_root = true;

  while (!stack.empty()) {
    auto [lhs, rhs] = stack.top();
    stack.pop();

    if (!at_root) {
      if (lhs.key() != rhs.key()) {
        return false;
      }
    }

    if (auto lhs_number = lhs->number_if()) {
      auto rhs_number = rhs->number_if();
      if (!rhs_number || *lhs_number != *rhs_number) {
        return false;
      }
    }
    if (auto lhs_string = lhs->string_if()) {
      auto rhs_string = rhs->string_if();
      if (!rhs_string || *lhs_string != *rhs_string) {
        return false;
      }
    }
    if (auto lhs_boolean = lhs->boolean_if()) {
      auto rhs_boolean = rhs->boolean_if();
      if (!rhs_boolean || *lhs_boolean != *rhs_boolean) {
        return false;
      }
    }
    if (auto lhs_binary = lhs->binary_if()) {
      auto rhs_binary = rhs->binary_if();
      if (!rhs_binary || !binary_match(*lhs_binary, *rhs_binary)) {
        return false;
      }
    }

    if (!at_root) {
      if (lhs.next()) {
        if (!rhs.next()) {
          return false;
        }
        stack.emplace(lhs.next(), rhs.next());
      } else if (rhs.next()) {
        return false;
      }
    }

    if (lhs.child()) {
      if (!rhs.child()) {
        return false;
      }
      stack.emplace(lhs.child(), rhs.child());
    } else if (rhs.child()) {
      return false;
    }

    at_root = false;
  }
  return true;
}

#if 0
bool operator==(ConstObject lhs, const object::primitive_t& rhs) {
  if (auto rhs_int = std::get_if<int>(&rhs)) {
    auto lhs_number = lhs.number_if();
    return lhs_number && *lhs_number == *rhs_int;
  }
  if (auto rhs_double = std::get_if<double>(&rhs)) {
    auto lhs_number = lhs.number_if();
    return lhs_number && *lhs_number == *rhs_double;
  }
  if (auto rhs_string = std::get_if<std::string>(&rhs)) {
    auto lhs_string = lhs.string_if();
    return lhs_string && *lhs_string == *rhs_string;
  }
  if (auto rhs_boolean = std::get_if<bool>(&rhs)) {
    auto lhs_boolean = lhs.boolean_if();
    return lhs_boolean && *lhs_boolean == *rhs_boolean;
  }
  if (auto rhs_binary = std::get_if<object::binary_t>(&rhs)) {
    auto lhs_binary = lhs.binary_if();
    return lhs_binary && binary_match(*lhs_binary, *rhs_binary);
  }
  // Unreachable
  assert(false);
}
#endif

std::ostream& operator<<(std::ostream& os, ConstObject object) {
  struct State {
    ConstObject::Ptr node;
    std::size_t depth;
    State(ConstObject::Ptr node, std::size_t depth) : node(node), depth(depth) {}
  };
  std::stack<State> stack;
  stack.emplace(object.ptr(), 0);
  bool is_root = true;

  auto indent = [&os](std::size_t depth) {
    for (std::size_t i = 0; i < depth; i++) {
      os << "  ";
    }
  };

  while (!stack.empty()) {
    auto [node, depth] = stack.top();
    stack.pop();

    if (!is_root) {
      os << "\n";
      indent(depth);
      if (node.key().empty()) {
        os << "- ";
      } else {
        os << node.key() << ": ";
      }
      auto next = node.next();
      if (next) {
        stack.emplace(next, depth);
      }
    }

    if (auto x = node->number_if()) {
      os << *x;
    } else if (auto x = node->boolean_if()) {
      os << (*x ? "true" : "false");
    } else if (auto x = node->string_if()) {
      os << *x;
    } else if (auto x = node->binary_if()) {
      if (x->size() > 0) {
        os << "[binary]\n";
        indent(depth + 1);
        const auto flags = os.flags();
        os << std::hex << std::uppercase << std::setfill('0') << std::setw(2);
        for (std::size_t i = 0; i < x->size(); i++) {
          os << int((*x)[i]);
          if (i != x->size() - 1) {
            if ((i + 1) % 8 == 0) {
              os << "\n";
              indent(depth + 1);
            } else {
              os << " ";
            }
          }
        }
        os.flags(flags);
      } else {
        os << "[binary]";
      }
    } else if (node->is_map()) {
      os << "[map]";
    } else if (node->is_list()) {
      os << "[list]";
    }

    auto child = node.child();
    if (child) {
      stack.emplace(child, depth + 1);
    }

    is_root = false;
  }
  return os;
}

} // namespace datapack::object

namespace datapack {

void object_prune(Object object) {
  if (!object.is_map()) {
    return;
  }

  struct State {
    Object::Ptr node;
    bool processed;
    State(Object::Ptr node) : node(node), processed(false) {}
  };
  std::stack<State> stack;
  stack.push(object.ptr());

  while (!stack.empty()) {
    auto& state = stack.top();
    auto node = state.node;
    if (state.processed) {
      stack.pop();
      if (node->size() == 0) {
        node->erase();
      }
      continue;
    }
    state.processed = true;

    auto child = node.child();
    while (child) {
      if (child->is_null()) {
        auto next = child.next();
        child->erase();
        child = next;
        continue;
      }
      if (child->is_map()) {
        stack.push(child);
      }
      child = child.next();
    }
  }
}

Object object_merge(ConstObject base_root, ConstObject diff_root) {
  Object merged_root;

  struct State {
    ConstObject::Ptr base;
    ConstObject::Ptr diff;
    Object::Ptr merged;
  };
  std::stack<State> stack;
  stack.emplace(base_root.ptr(), diff_root.ptr(), merged_root.ptr());

  while (!stack.empty()) {
    auto [base, diff, merged] = stack.top();
    stack.pop();

    if (!base->is_map() || !diff->is_map()) {
      *merged = *diff;
      continue;
    }
    merged->to_map();

    for (auto iter = base->items().begin(); iter != base->items().end(); iter++) {
      auto [key, value] = *iter;
      if (!diff->contains(key)) {
        (*merged)[key] = value;
      } else {
        stack.emplace(value.ptr(), diff->find(key), (*merged)[key].ptr());
      }
    }
    for (auto [key, value] : diff->items()) {
      if (!base->contains(key)) {
        (*merged)[key] = value;
      }
    }
  }

  object_prune(merged_root);

  return merged_root;
}

Object object_diff(ConstObject base_root, ConstObject modified_root) {
  Object diff_root;

  struct State {
    ConstObject::Ptr base;
    ConstObject::Ptr modified;
    Object::Ptr diff;
  };
  std::stack<State> stack;
  stack.emplace(base_root.ptr(), modified_root.ptr(), diff_root.ptr());

  while (!stack.empty()) {
    auto [base, modified, diff] = stack.top();
    stack.pop();

    if (!base->is_map() || !modified->is_map()) {
      if (*base != *modified) {
        *diff = *modified;
      }
      continue;
    }
    diff->to_map();

    for (auto [key, value] : modified->items()) {
      if (auto base_ptr = base->find(key)) {
        stack.emplace(base_ptr, value.ptr(), (*diff)[key].ptr());
      } else {
        (*diff)[key] = value;
      }
    }
    for (auto [key, value] : base->items()) {
      if (!modified->contains(key)) {
        throw object::UsageError("Cannot evaluate object_diff(base, modified) where there are keys "
                                 "in base that aren't in modified ");
      }
    }
  }

  object_prune(diff_root);

  return diff_root;
}

} // namespace datapack
