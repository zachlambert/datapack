#include "datapack/object.hpp"
#include <stack>
#include <iostream>


namespace datapack {

#if 0
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
#endif

} // namespace datapack

std::ostream& operator<<(std::ostream& os, datapack::ConstObject object) {
    using namespace datapack;

    std::stack<datapack::ConstObject> nodes;
    nodes.push(object);
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
            if (!node.key().empty()) {
                os << node.key() << ": ";
            } else {
                os << "- ";
            }
        }

        if (node.get_if<Object::map_t>()) {
            os << "map:";
        }
        else if (node.get_if<Object::list_t>()) {
            os << "list:";
        }
        else if (auto value = node.get_if<Object::int_t>()) {
            os << *value << "";
        }
        else if (auto value = node.get_if<Object::float_t>()) {
            os << *value;
        }
        else if (auto value = node.get_if<Object::bool_t>()) {
            os << (*value ? "true" : "false");
        }
        else if (auto value = node.get_if<Object::str_t>()) {
            os << *value;
        }
        else if (node.get_if<Object::null_t>()) {
            os << "null";
        }
        else if (auto value = node.get_if<Object::binary_t>()) {
            os << "binary (size=" << value->size() << ")";
        }

        if (depth > 0) {
            nodes.push(node.next());
        }

        if (node.get_if<Object::map_t>() || node.get_if<Object::list_t>()) {
            nodes.push(node.child());
            depth++;
        }
    }
    return os;
}
