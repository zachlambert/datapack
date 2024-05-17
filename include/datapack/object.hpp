#pragma once

#include <vector>
#include <type_traits>
#include <string>
#include <optional>
#include <variant>
#include <stdexcept>


namespace datapack {

struct ObjectList {};
struct ObjectMap {};

using ObjectValue = std::variant<
    double,
    std::nullopt_t,
    std::string,
    bool,
    ObjectList,
    ObjectMap
>;

struct ObjectNode {
    ObjectValue value;
    std::string key;
    int parent;
    int next;
    int child;
    ObjectNode(const ObjectValue& value, const std::string& key, int parent):
        value(value),
        key(key),
        parent(parent),
        next(-1),
        child(-1)
    {}
};


template <bool IsConst>
class ObjectPointer_ {
    using nodes_ptr_t = std::conditional_t<
        IsConst,
        const std::vector<ObjectNode>*,
        std::vector<ObjectNode>*
    >;
public:
    ObjectPointer_(int index, nodes_ptr_t nodes):
        index(index),
        nodes(nodes)
    {}

    operator bool() const {
        return index != -1;
    }

    const std::string& key() const {
        return node().key;
    }

    template <typename T>
    std::conditional_t<IsConst, const T*, T*> get_if() const {
        if (index == -1) return nullptr;
        return std::get_if<T>(&node().value);
    }

    ObjectPointer_ operator[](const std::string& key) const {
        if (index == -1) {
            return null();
        }
        if (!get_if<ObjectMap>()) {
            return null();
        }
        auto iter = this->child();
        while (iter) {
            if (iter.key() == key) {
                return iter;
            }
        }
        return null();
    }

    ObjectPointer_ insert(const std::string& key, const ObjectValue& value) const {
        static_assert(!IsConst);
        if (index == -1 || !get_if<ObjectMap>()) {
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

    ObjectPointer_ operator[](std::size_t index) const {
        if (index == -1) {
            return null();
        }
        if (!get_if<ObjectList>()) {
            return null();
        }
        auto iter = this->child();
        for (int i = 0; i < index; i++) {
            iter = iter.next();
        }
        return iter;
    }

    ObjectPointer_ append(const ObjectValue& value) const {
        static_assert(!IsConst);
        if (index == -1 || !get_if<ObjectList>()) {
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

    ObjectPointer_ next() const {
        if (index == -1) {
            return null();
        }
        return ObjectPointer_(node().next, nodes);
    }

    ObjectPointer_ child() const {
        if (index == -1) {
            return null();
        }
        return ObjectPointer_(node().child, nodes);
    }

    template <bool FromConst, typename = std::enable_if_t<IsConst || !FromConst>>
    ObjectPointer_(const ObjectPointer_<FromConst>& from):
        index(from.index),
        nodes(from.nodes)
    {}

private:
    std::conditional_t<IsConst, const ObjectNode&, ObjectNode&> node() const {
        return (*nodes)[index];
    }

    ObjectPointer_ null() const {
        return ObjectPointer_(-1, nodes);
    }

    int index;
    nodes_ptr_t nodes;

    template <bool OtherConst>
    friend class ObjectPointer_;
};

using ObjectPointer = ObjectPointer_<false>;
using ObjectConstPointer = ObjectPointer_<true>;


class Object {
public:
    Object():
        root_(-1)
    {}

    ObjectPointer root() {
        return ObjectPointer(root_, &nodes);
    }

    ObjectConstPointer root() const {
        return ObjectConstPointer(root_, &nodes);
    }

    void set_root(const ObjectValue& value) {
        nodes.clear();
        root_ = 0;
        nodes.emplace_back(value, "", -1);
    }

private:
    int root_;
    std::vector<ObjectNode> nodes;
};

} // namespace datapack
