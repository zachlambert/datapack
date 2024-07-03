#include "datapack/object.hpp"
#include <stack>
#include <assert.h>
#include <iostream> // TEMP


namespace datapack {


template <bool IsConst>
Object_<IsConst> Object_<IsConst>::insert(const std::string& key, const value_t& value) const {
    if (!get_if<map_t>()) {
        throw ObjectException("Not in a map");
    }
    auto new_node = create_node(Node(value, key, index, -1));

    auto iter = child();
    if (!iter) {
        node().child = new_node.index;
    } else {
        while (iter.next()) {
            iter = iter.next();
        }
        iter.node().next = new_node.index;
        new_node.node().prev = iter.index;
    }
    return new_node;
}
template Object Object::insert(const std::string&, const value_t&) const;


template <bool IsConst>
Object_<IsConst> Object_<IsConst>::operator[](const std::string& key) const {
    auto iter = child();
    while (iter && iter.key() != key) {
        iter = iter.next();
    }
    return iter;
}
template Object Object::operator[](const std::string&) const;


template <bool IsConst>
Object_<IsConst> Object_<IsConst>::append(const value_t& value) const {
    if (!get_if<list_t>()) {
        throw ObjectException("Not in a list");
    }
    auto new_node = create_node(Node(value, "", index, -1));

    auto iter = child();
    if (!iter) {
        node().child = new_node.index;
    } else {
        while (iter.next()) {
            iter = iter.next();
        }
        iter.node().next = new_node.index;
        new_node.node().prev = iter.index;
    }
    return new_node;
}
template Object Object::append(const value_t&) const;


template <bool IsConst>
Object_<IsConst> Object_<IsConst>::operator[](std::size_t index) const {
    auto iter = child();
    std::size_t i = 0;
    while (iter && i != index) {
        iter = iter.next();
        i++;
    }
    return iter;
}
template Object Object::operator[](std::size_t) const;
template ConstObject ConstObject::operator[](std::size_t) const;


template <bool IsConst>
std::size_t Object_<IsConst>::size() const {
    std::size_t count = 0;
    auto iter = child();
    while (iter) {
        iter = iter.next();
        count++;
    }
    return count;
}
template std::size_t Object::size() const;
template std::size_t ConstObject::size() const;


template <bool IsConst>
Object_<IsConst> Object_<IsConst>::clone() const {
    Object_<false> result(value());

    if (!get_if<map_t>() && !get_if<list_t>()) {
        return result;
    }
    if (!child()) {
        return result;
    }

    std::stack<Object_<true>> from_stack;
    from_stack.push(child());
    std::stack<Object_<false>> to_stack;
    to_stack.push(result.root());

    while (!from_stack.empty()) {
        auto from = from_stack.top();
        from_stack.pop();
        if (!from){
            to_stack.pop();
            continue;
        }
        from_stack.push(from.next());

        auto to = to_stack.top();
        Object_<false> new_to;
        if (to.get_if<map_t>()) {
            new_to = to.insert(from.key(), from.value());
        }
        else if (to.get_if<list_t>()) {
            new_to = to.append(from.value());
        }
        else {
            assert(false);
        }

        if (from.get_if<map_t>() || from.get_if<list_t>()) {
            from_stack.push(from.child());
            to_stack.push(new_to);
        }
    }

    return result;
}
template Object Object::clone() const;
template ConstObject ConstObject::clone() const;


template <bool IsConst>
void Object_<IsConst>::set(const value_t& value) const {
    static_assert(!IsConst);
    clear();
    this->value() = value;
}
template void Object::set(const value_t&) const;


template <bool IsConst>
void Object_<IsConst>::erase() const {
    static_assert(!IsConst);
    clear();
    if (auto prev = this->prev()) {
        prev.node().next = node().next;
    }
    if (auto next = this->next()) {
        next.node().prev = node().prev;
    }
    if (auto parent = this->parent()) {
        if (parent.node().child == index) {
            parent.node().child = node().next;
        }
    }

    if (index == state->nodes.size() - 1) {
        state->nodes.pop_back();
    } else {
        state->free.push(index);
    }
}
template void Object::erase() const;


template <bool IsConst>
void Object_<IsConst>::clear() const {
    static_assert(!IsConst);
    auto iter = child();
    while (iter) {
        auto prev = iter;
        iter = iter.next();
        prev.erase();
    }
}
template void Object::clear() const;


template <bool IsConst>
Object_<IsConst> Object_<IsConst>::create_node(const Node& node) const {
    static_assert(!IsConst);
    int index;
    if (state->free.empty()) {
        state->nodes.push_back(node);
        index = state->nodes.size() - 1;
    } else {
        index = state->free.top();
        state->free.pop();
        state->nodes[index] = node;
    }
    return Object_(state, index);
}
template Object Object::create_node(const Node&) const;


bool compare(const ConstObject& lhs, const ConstObject& rhs, double float_threshold) {
    std::stack<ConstObject> nodes_lhs;
    std::stack<ConstObject> nodes_rhs;
    nodes_lhs.push(lhs);
    nodes_rhs.push(rhs);

    while (!nodes_lhs.empty()) {
        ConstObject lhs = nodes_lhs.top();
        ConstObject rhs = nodes_rhs.top();
        nodes_lhs.pop();
        nodes_rhs.pop();

        if (bool(lhs) != bool(rhs)) {
            return false;
        }
        if (!lhs) { // && !rhs
            continue;
        }

        if (!nodes_lhs.empty() && lhs.parent() && lhs.parent().get_if<Object::map_t>()) {
            auto lhs_next = lhs.next();
            nodes_lhs.push(lhs_next);
            nodes_rhs.push(rhs.parent()[lhs_next.key()]);
        }
        if (!nodes_lhs.empty() && lhs.parent() && lhs.parent().get_if<Object::list_t>()) {
            nodes_lhs.push(lhs.next());
            nodes_rhs.push(rhs.next());
        }

        if (lhs.value().index() != rhs.value().index()) {
            return false;
        }

        if (lhs.get_if<Object::map_t>()) {
            auto lhs_child = lhs.child();
            auto rhs_child = rhs[lhs_child.key()];
            nodes_lhs.push(lhs_child);
            nodes_rhs.push(rhs_child);
            continue;
        }
        if (lhs.get_if<Object::list_t>()) {
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
        }, lhs.value());
        if (!values_equal) {
            return false;
        }
    }
    return true;
}

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
