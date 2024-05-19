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
        const std::string key;
        int parent;
        int child;
        int prev;
        int next;
        Node(const value_t& value, const std::string& key, int parent, int prev):
            value(value), key(key), parent(parent), child(-1), prev(prev), next(-1)
        {}
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
    using nodes_t = std::conditional_t<IsConst,
        std::shared_ptr<const std::vector<Node>>,
        std::shared_ptr<std::vector<Node>>
    >;

public:
    Object_():
        index(-1)
    {}

    Object_(const value_t& root_value):
        nodes(std::make_shared<std::vector<Node>>()),
        index(0)
    {
        static_assert(!IsConst);
        nodes->push_back(Node(root_value, "", -1, -1));
    }

    template <bool OtherConst, typename = std::enable_if_t<IsConst || !OtherConst>>
    Object_(const Object_<OtherConst>& other):
        nodes(other.nodes),
        index(other.index)
    {}

    Object_ root() const {
        return Object_(nodes, 0);
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
        return Object_(nodes, node().prev);
    }
    Object_ next() const {
        return Object_(nodes, node().next);
    }
    Object_ parent() const {
        return Object_(nodes, node().parent);
    }
    Object_ child() const {
        return Object_(nodes, node().child);
    }

    Object_ insert(const std::string& key, const value_t& value) const {
        if (!get_if<map_t>()) {
            throw std::runtime_error("Not in a map");
        }
        auto iter = child();
        int new_index = nodes->size();
        int prev = -1;
        if (!iter) {
            node().child = new_index;
        } else {
            while (iter.next()) {
                iter = iter.next();
            }
            iter.node().next = new_index;
            prev = iter.index;
        }
        nodes->push_back(Node(value, key, index, prev));
        return Object_(nodes, new_index);
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
        auto iter = child();
        int new_index = nodes->size();
        int prev = -1;
        if (!iter) {
            node().child = new_index;
        } else {
            while (iter.next()) {
                iter = iter.next();
            }
            iter.node().next = new_index;
            prev = iter.index;
        }
        nodes->push_back(Node(value, "", index, prev));
        return Object_(nodes, new_index);
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

private:
    Object_(nodes_t nodes, int index):
        nodes(nodes), index(index)
    {}

    std::conditional_t<IsConst, const Node&, Node&> node() const {
        return (*nodes)[index];
    }

    nodes_t nodes;
    int index;

    template <bool OtherConst>
    friend class Object_;
};

using Object = Object_<false>;
using ConstObject = Object_<true>;


#if 0

namespace object {

struct list_t {};
struct map_t {};
using int_t = std::int64_t;
using float_t = double;
using binary_t = std::vector<std::uint8_t>;
using null_t = std::nullopt_t;
using str_t = std::string;
using bool_t = bool;

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
    int next;
    int child;
    Node(const value_t& value, const std::string& key, int parent):
        value(value),
        key(key),
        parent(parent),
        next(-1),
        child(-1)
    {}
};


template <bool IsConst>
class Pointer_ {
    using nodes_ptr_t = std::conditional_t<
        IsConst,
        const std::vector<Node>*,
        std::vector<Node>*
    >;
public:
    Pointer_(int index, nodes_ptr_t nodes):
        index(index),
        nodes(nodes)
    {}

    operator bool() const {
        return index != -1;
    }

    const std::string& key() const {
        return node().key;
    }

    std::conditional_t<IsConst, const value_t&, value_t&> value() const {
        return node().value;
    }

    template <typename T>
    std::conditional_t<IsConst, const T*, T*> get_if() const {
        if (index == -1) return nullptr;
        return std::get_if<T>(&node().value);
    }

    Pointer_ operator[](const std::string& key) const {
        if (index == -1) {
            return null();
        }
        if (!get_if<map_t>()) {
            return null();
        }
        auto iter = this->child();
        while (iter) {
            if (iter.key() == key) {
                return iter;
            }
            iter = iter.next();
        }
        return null();
    }

    Pointer_ insert(const std::string& key, const value_t& value) const {
        static_assert(!IsConst);
        if (index == -1 || !get_if<map_t>()) {
            throw std::runtime_error("Cannot call insert on non-map node");
        }

        int new_index = nodes->size();
        nodes->emplace_back(value, key, index);

        auto iter = child();
        if (!iter) {
            node().child = new_index;
            return child();
        }

        while (iter.next()) {
            iter = iter.next();
        }
        iter.node().next = new_index;

        return iter.next();
    }

    Pointer_ operator[](std::size_t index) const {
        if (index == -1) {
            return null();
        }
        if (!get_if<list_t>()) {
            return null();
        }
        auto iter = this->child();
        for (int i = 0; i < index; i++) {
            iter = iter.next();
        }
        return iter;
    }

    Pointer_ append(const value_t& value) const {
        static_assert(!IsConst);
        if (index == -1 || !get_if<list_t>()) {
            throw std::runtime_error("Cannot call insert on non-list node");
        }

        int new_index = nodes->size();
        nodes->emplace_back(value, "", index);

        auto iter = child();
        if (!iter) {
            node().child = new_index;
            return child();
        }

        while (iter.next()) {
            iter = iter.next();
        }
        iter.node().next = new_index;
        return iter.next();
    }

    std::size_t size() const {
        auto iter = this->child();
        std::size_t result = 0;
        while (iter) {
            iter = iter.next();
            result++;
        }
        return result;
    }

    Pointer_ next() const {
        if (index == -1) {
            return null();
        }
        return Pointer_(node().next, nodes);
    }

    Pointer_ child() const {
        if (index == -1) {
            return null();
        }
        return Pointer_(node().child, nodes);
    }

    Pointer_ parent() const {
        if (index == -1) {
            return null();
        }
        return Pointer_(node().parent, nodes);
    }

    template <bool FromConst, typename = std::enable_if_t<IsConst || !FromConst>>
    Pointer_(const Pointer_<FromConst>& from):
        index(from.index),
        nodes(from.nodes)
    {}

private:
    std::conditional_t<IsConst, const Node&, Node&> node() const {
        return (*nodes)[index];
    }

    Pointer_ null() const {
        return Pointer_(-1, nodes);
    }

    int index;
    nodes_ptr_t nodes;

    template <bool OtherConst>
    friend class Pointer_;
};

class Object {
public:
    using Pointer = Pointer_<false>;
    using ConstPointer = Pointer_<true>;

    Object():
        root_(-1)
    {}

    Pointer root() {
        return Pointer(root_, &nodes);
    }

    ConstPointer root() const {
        return ConstPointer(root_, &nodes);
    }

    void set_root(const value_t& value) {
        nodes.clear();
        root_ = 0;
        nodes.emplace_back(value, "", -1);
    }

    Pointer null() {
        return Pointer(-1, &nodes);
    }

    ConstPointer null() const {
        return ConstPointer(-1, &nodes);
    }

private:
    int root_;
    std::vector<Node> nodes;
};

} // namespace object

using Object = object::Object;

#endif

// bool compare(const Object& lhs, const Object& rhs, double float_threshold=1e-12);

} // namespace datapack

std::ostream& operator<<(std::ostream& os, datapack::ConstObject object);
std::ostream& operator<<(std::ostream& os, datapack::Object object) {
    os << static_cast<datapack::ConstObject>(object);
    return os;
}
