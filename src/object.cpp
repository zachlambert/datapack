#include "datapack/object.hpp"
#include <stack>
#include <iostream>


namespace datapack {

bool compare(const Object& lhs, const Object& rhs, double float_threshold) {
    std::stack<Object::ConstPointer> nodes_lhs;
    std::stack<Object::ConstPointer> nodes_rhs;
    nodes_lhs.push(Object::ConstPointer(lhs.root()));
    nodes_rhs.push(Object::ConstPointer(rhs.root()));

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

        if (lhs.parent() && lhs.parent().get_if<object::map_t>()) {
            auto lhs_next = lhs.next();
            nodes_lhs.push(lhs_next);
            nodes_rhs.push(rhs.parent()[lhs_next.key()]);
        }
        if (lhs.parent() && lhs.parent().get_if<object::list_t>()) {
            nodes_lhs.push(lhs.next());
            nodes_rhs.push(rhs.next());
        }

        if (lhs.value().index() != rhs.value().index()) {
            return false;
        }

        if (lhs.get_if<object::map_t>()) {
            auto lhs_child = lhs.child();
            auto rhs_child = rhs[lhs_child.key()];
            nodes_lhs.push(lhs_child);
            nodes_rhs.push(rhs_child);
            continue;
        }
        if (lhs.get_if<object::list_t>()) {
            nodes_lhs.push(lhs.child());
            nodes_rhs.push(rhs.child());
            continue;
        }

        bool values_equal = std::visit([&rhs, float_threshold](const auto& lhs_value) -> bool {
            using T = std::decay_t<decltype(lhs_value)>;
            auto rhs_value_ptr = rhs.get_if<T>();
            if (!rhs_value_ptr) {
                return false;
            }
            const auto& rhs_value = *rhs_value_ptr;

            if constexpr(std::is_same_v<object::int_t, T>) {
                return (rhs_value == lhs_value);
            }
            if constexpr(std::is_same_v<object::float_t, T>) {
                return std::abs(rhs_value - lhs_value) < float_threshold;
            }
            if constexpr(std::is_same_v<object::bool_t, T>) {
                return (rhs_value == lhs_value);
            }
            if constexpr(std::is_same_v<object::str_t, T>) {
                return (rhs_value == lhs_value);
            }
            if constexpr(std::is_same_v<object::null_t, T>) {
                return true;
            }
            if constexpr(std::is_same_v<object::binary_t, T>) {
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
            if constexpr(std::is_same_v<object::map_t, T>) {
                return true; // Unreachable
            }
            if constexpr(std::is_same_v<object::list_t, T>) {
                return true; // Unreachable
            }
        }, lhs.value());
        if (!values_equal) {
            return false;
        }
    }
    return true;
}

} // namespace datapack

std::ostream& operator<<(std::ostream& os, const datapack::Object& object) {
    using namespace datapack;

    std::stack<Object::ConstPointer> nodes;
    nodes.push(Object::ConstPointer(object.root()));

    int depth = 0;
    while (!nodes.empty()) {
        auto node = nodes.top();
        nodes.pop();
        if (!node) {
            depth--;
            continue;
        }
        nodes.push(node.next());

        for (int i = 0; i < depth; i++) {
            os << "    ";
        }

        if (!node.key().empty()) {
            os << node.key() << ": ";
        }

        if (node.get_if<object::map_t>()) {
            os << "map:\n";
            nodes.push(node.child());
            depth++;
            continue;
        }
        else if (node.get_if<object::list_t>()) {
            os << "list:\n";
            depth++;
            nodes.push(node.child());
            continue;
        }

        if (auto value = node.get_if<double>()) {
            os << *value << "\n";
        }
        else if (auto value = node.get_if<std::string>()) {
            os << *value << "\n";
        }
        else if (auto value = node.get_if<bool>()) {
            os << (*value ? "true" : "false") << "\n";
        }
        else if (node.get_if<std::nullopt_t>()) {
            os << "null\n";
        }
    }
    return os;
}
