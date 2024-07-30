#include "datapack/object.hpp"
#include <stack>
#include <assert.h>

namespace datapack {

Object merge(const Object::ConstReference& base, const Object::ConstReference& diff) {
    Object merged;

    std::stack<Object::ConstPointer> base_nodes;
    std::stack<Object::ConstPointer> diff_nodes;
    std::stack<Object::Pointer> merged_nodes;

    diff_nodes.push(diff.ptr());

    while (!diff_nodes.empty()) {

    }

    return merged;
}


Object diff(const Object::ConstReference& base, const Object::ConstReference& modified) {
    Object diff;
    return diff;
}


bool operator==(const Object::ConstReference& lhs, const Object::ConstReference& rhs) {
    static constexpr double float_threshold = 1e-12;
    std::stack<Object::ConstPointer> nodes_lhs;
    std::stack<Object::ConstPointer> nodes_rhs;
    nodes_lhs.push(lhs.ptr());
    nodes_rhs.push(rhs.ptr());

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

        bool values_equal = std::visit([&rhs](const auto& lhs_value) -> bool {
            using T = std::decay_t<decltype(lhs_value)>;
            auto rhs_value_ptr = rhs->get_value<T>();
            if (!rhs_value_ptr) {
                return false;
            }
            const auto& rhs_value = *rhs_value_ptr;

            if constexpr(std::is_same_v<Object::int_t, T>) {
                return (rhs_value == lhs_value);
            }
            if constexpr(std::is_same_v<Object::float_t, T>) {
                return std::abs(rhs_value - lhs_value) < float_threshold;
            }
            if constexpr(std::is_same_v<Object::bool_t, T>) {
                return (rhs_value == lhs_value);
            }
            if constexpr(std::is_same_v<Object::str_t, T>) {
                return (rhs_value == lhs_value);
            }
            if constexpr(std::is_same_v<Object::null_t, T>) {
                return true;
            }
            if constexpr(std::is_same_v<Object::binary_t, T>) {
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
            if constexpr(std::is_same_v<Object::map_t, T>) {
                return true; // Unreachable
            }
            if constexpr(std::is_same_v<Object::list_t, T>) {
                return true; // Unreachable
            }
        }, lhs->value());
        if (!values_equal) {
            return false;
        }
    }
    return true;
}


std::ostream& operator<<(std::ostream& os, Object::ConstReference object) {
    std::stack<datapack::Object::ConstPointer> nodes;
    nodes.push(object.ptr());
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
        }
        else if (node->is_list()) {
            os << "list:";
        }
        else if (auto value = node->get_value<Object::int_t>()) {
            os << *value << "";
        }
        else if (auto value = node->get_value<Object::float_t>()) {
            os << *value;
        }
        else if (auto value = node->get_value<Object::bool_t>()) {
            os << (*value ? "true" : "false");
        }
        else if (auto value = node->get_value<Object::str_t>()) {
            os << *value;
        }
        else if (node->is_null()) {
            os << "null";
        }
        else if (auto value = node->get_value<Object::binary_t>()) {
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

} // namespace datapack
