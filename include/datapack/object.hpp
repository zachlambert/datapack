#pragma once

#include <vector>
#include <type_traits>
#include <string>
#include <optional>
#include <variant>
#include <stdexcept>
#include <memory>
#include <stack>


namespace datapack {

namespace _object {
    using int_t = std::int64_t;
    using float_t = double;
    using bool_t = bool;
    using str_t = std::string;
    using null_t = std::nullopt_t;
    using binary_t = std::vector<std::uint8_t>;
    struct map_t {};
    struct list_t {};

    using value_t = std::variant<
        int_t,
        float_t,
        bool_t,
        str_t,
        null_t,
        binary_t,
        map_t,
        list_t
    >;

    struct Node {
        value_t value;
        std::string key;
        int parent;
        int child;
        int prev;
        int next;
        Node(const value_t& value, const std::string& key, int parent, int prev):
            value(value), key(key), parent(parent), child(-1), prev(prev), next(-1)
        {}
    };

    struct State {
        std::vector<Node> nodes;
        std::stack<int> free;
    };
} // namespace object

template <bool IsConst>
class Object_ {
public:
    using int_t = _object::int_t;
    using float_t = _object::float_t;
    using bool_t = _object::bool_t;
    using str_t = _object::str_t;
    using null_t = _object::null_t;
    using binary_t = _object::binary_t;
    using map_t = _object::map_t;
    using list_t = _object::list_t;
    using value_t = _object::value_t;
    using Node = _object::Node;

private:
    using State = _object::State;
    using state_t = std::conditional_t<
        IsConst,
        std::shared_ptr<const State>,
        std::shared_ptr<State>
    >;

public:
    Object_():
        index(-1)
    {}

    Object_(const value_t& root_value):
        state(std::make_shared<State>()),
        index(0)
    {
        static_assert(!IsConst);
        state->nodes.push_back(Node(root_value, "", -1, -1));
    }

    template <bool OtherConst, typename = std::enable_if_t<IsConst || !OtherConst>>
    Object_(const Object_<OtherConst>& other):
        state(other.state),
        index(other.index)
    {}

    Object_ root() const {
        return Object_(state, 0);
    }
    Object_<true> const_root() const {
        return root();
    }

    operator bool() const {
        return index != -1;
    }
    std::conditional_t<IsConst, const value_t&, value_t&> value() const {
        return node().value;
    }
    const std::string& key() const {
        return node().key;
    }
    template <typename T>
    std::conditional_t<IsConst, const T*, T*> get_if() const {
        if (!(*this)) return nullptr;
        return std::get_if<T>(&node().value);
    }

    Object_ prev() const {
        return Object_(state, node().prev);
    }
    Object_ next() const {
        return Object_(state, node().next);
    }
    Object_ parent() const {
        return Object_(state, node().parent);
    }
    Object_ child() const {
        return Object_(state, node().child);
    }

    Object_ insert(const std::string& key, const value_t& value) const {
        if (!get_if<map_t>()) {
            throw std::runtime_error("Not in a map");
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

    Object_ operator[](const std::string& key) const {
        auto iter = child();
        while (iter && iter.key() != key) {
            iter = iter.next();
        }
        return iter;
    }
    Object_ operator[](const char* key) const {
        return (*this)[std::string(key)];
    }

    Object_ append(const value_t& value) const {
        if (!get_if<list_t>()) {
            throw std::runtime_error("Not in a list");
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

    Object_ operator[](std::size_t index) const {
        auto iter = child();
        std::size_t i = 0;
        while (iter && i != index) {
            iter = iter.next();
            i++;
        }
        return iter;
    }
    Object_ operator[](int index) const {
        return (*this)[(std::size_t)index];
    }

    std::size_t size() const {
        std::size_t count = 0;
        auto iter = child();
        while (iter) {
            iter = iter.next();
            count++;
        }
        return count;
    }

    Object_ clone() const {
        Object_ result(value());

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
                throw std::runtime_error("Unreachable code");
            }

            if (from.get_if<map_t>() || from.get_if<list_t>()) {
                from_stack.push(from.child());
                to_stack.push(new_to);
            }
        }

        return result;
    }

    void set(const value_t& value) {
        static_assert(!IsConst);
        clear();
        this->value() = value;
    }

    void erase() {
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

    void clear() {
        static_assert(!IsConst);
        auto iter = child();
        while (iter) {
            auto prev = iter;
            iter = iter.next();
            prev.erase();
        }
    }

private:
    Object_(state_t state, int index):
        state(state), index(index)
    {}

    std::conditional_t<IsConst, const Node&, Node&> node() const {
        return state->nodes[index];
    }

    Object_ create_node(const Node& node) const {
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

    state_t state;
    int index;

    template <bool OtherConst>
    friend class Object_;
};

using Object = Object_<false>;
using ConstObject = Object_<true>;

bool compare(const Object& lhs, const Object& rhs, double float_threshold=1e-12);

} // namespace datapack

std::ostream& operator<<(std::ostream& os, datapack::ConstObject object);
inline std::ostream& operator<<(std::ostream& os, datapack::Object object) {
    os << static_cast<datapack::ConstObject>(object);
    return os;
}
