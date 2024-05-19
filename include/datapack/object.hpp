#pragma once

#include <vector>
#include <type_traits>
#include <string>
#include <optional>
#include <variant>
#include <stdexcept>


namespace datapack {

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

bool compare(const Object& lhs, const Object& rhs, double float_threshold=1e-12);

} // namespace datapack

std::ostream& operator<<(std::ostream& os, const datapack::Object& object);
