#pragma once

#include <vector>
#include <type_traits>
#include <string>
#include <optional>
#include <variant>
#include <stdexcept>
#include <memory>
#include <stack>
#include <assert.h>


namespace datapack {

class LoadException: public std::exception {
public:
    LoadException(const std::string& message):
        message(message)
    {}
private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

class DumpException: public std::exception {
public:
    DumpException(const std::string& message):
        message(message)
    {}
private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

class ObjectException: public std::exception {
public:
    ObjectException(const std::string& message):
        message(message)
    {}
private:
    const char* what() const noexcept override {
        return message.c_str();
    }
    std::string message;
};

class Object {
public:
    using int_t = std::int64_t;
    using float_t = double;
    using bool_t = bool;
    using str_t = std::string;
    using null_t = std::nullopt_t;
    static constexpr null_t null = std::nullopt;
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

private:
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

    template <bool IsConst>
    class Pointer_;

    template <bool IsConst>
    class Reference_ {
        using parent_t = std::conditional_t<IsConst, const Object*, Object*>;

    public:
        const Reference_& operator=(const value_t& value) const {
            static_assert(!IsConst);
            parent->assign(index, value);
            return *this;
        }

        Reference_ operator[](const std::string& key) const {
            static_assert(!IsConst);
            if (parent->template get_value<null_t>(index)) {
                parent->assign(index, map_t());
            } else if (!parent->template get_value<map_t>(index)){
                throw ObjectException("Tried to access value by key on a non-map node");
            }

            int result = parent->map_access(index, key);
            if (result == -1) {
                result = parent->node_insert(index, key);
            }
            return Reference_(parent, result);
        }

        Reference_ operator[](std::size_t list_index) const {
            int result = parent->list_access(index, list_index);
            return Reference(parent, result);
        }

        Reference_ at(const std::string key) {
            return parent->map_access(index, key);
        }

        Reference_ insert(const std::string& key, const value_t& value) {
            int value_index = parent->node_insert(index, key);
            parent->assign(value_index, value);
        }

        Reference_ append(const value_t& value) {
            if (parent->template get_value<null_t>(index)) {
                parent->assign(index, list_t());
            } else {
                throw ObjectException("Tried to append to a non-list node");
            }

            int value_index = parent->node_append(index, index);
            parent->assign(value_index, value);
        }

        const std::string& key() const {
            return parent->nodes[index].key;
        }

        template <typename T>
        std::conditional_t<IsConst, const T*, T*> get_if() const {
            return parent->template get_value<T>(index);
        }

        Pointer_<IsConst> ptr() const;

        template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
        Reference_(const Reference_<OtherConst>& other):
            parent(other.parent), index(other.index)
        {}

    private:
        Reference_(parent_t parent, int index):
            parent(parent), index(index)
        {}
        parent_t parent;
        int index;

        template <bool OtherConst>
        friend class Pointer_;
        friend class Object;
    };

    template <bool IsConst>
    class Pointer_ {
        using object_t = std::conditional_t<IsConst, const Object*, Object*>;

    public:
        Reference_<IsConst> operator*() const {
            return Reference_<IsConst>(object, index);
        }
        const Reference_<IsConst>* operator->() const {
            return &reference;
        }

        operator bool() const {
            return index != -1;
        }
        Pointer_ parent() const {
            return Pointer_(object, object->get_parent(index));
        }
        Pointer_ child() const {
            return Pointer_(object, object->get_child(index));
        }
        Pointer_ prev() const {
            return Pointer_(object, object->get_prev(index));
        }
        Pointer_ next() const {
            return Pointer_(object, object->get_next(index));
        }

        template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
        Pointer_(const Pointer_<OtherConst>& other):
            object(other.object), index(other.index)
        {}

    private:
        std::conditional_t<IsConst, const Node&, Node&> node() {
            return object->nodes[index];
        }

        Pointer_(object_t object, int index):
            object(object), index(index), reference(object, index)
        {}
        object_t object;
        int index;
        Reference_<IsConst> reference;
        friend class Object;
    };

public:
    using Reference = Reference_<false>;
    using ConstReference = Reference_<true>;
    using Pointer = Pointer_<false>;
    using ConstPointer = Pointer_<true>;

    Object():
        root_index(0)
    {
        nodes.push_back(Node(null, "", -1, -1));
    }

    operator Reference() {
        return Reference(this, root_index);
    }
    operator ConstReference() const {
        return ConstReference(this, root_index);
    }

    Reference operator=(const value_t& value) {
        assign(root_index, value);
        return Reference(this, root_index);
    }

    Reference operator[](const std::string& key) {
        if (get_value<null_t>(root_index)) {
            assign(root_index, map_t());
        } else if (!get_value<map_t>(root_index)) {
            throw ObjectException("Tried to access value by key on a non-map node");
        }

        int result = map_access(root_index, key);
        if (result == -1) {
            result = node_insert(root_index, key);
        }
        return Reference(this, result);
    }

    ConstReference operator[](std::size_t list_index) const {
        if (!get_value<list_t>(root_index)) {
            throw ObjectException("Tried to access value by index on a non-list node");
        }
        return ConstReference(this, list_access(root_index, list_index));
    }

    ConstReference at(const std::string key) {
        if (!get_value<map_t>(root_index)) {
            throw ObjectException("Tried to access value by key on a non-map node");
        }
        return ConstReference(this, map_access(root_index, key));
    }

    Reference insert(const std::string& key, const value_t& value) {
        int value_index = node_insert(root_index, key);
        assign(value_index, value);
        return Reference(this, value_index);
    }

    Reference append(const value_t& value) {
        int value_index = node_append(root_index);
        assign(value_index, value);
        return Reference(this, value_index);
    }

private:
    int get_parent(int node) const {
        if (node == -1) {
            return -1;
        }
        return nodes[node].parent;
    }

    int get_child(int node) const {
        if (node == -1) {
            return -1;
        }
        return nodes[node].child;
    }

    int get_next(int node) const {
        if (node == -1) {
            return -1;
        }
        return nodes[node].next;
    }

    int get_prev(int node) const {
        if (node == -1) {
            return -1;
        }
        return nodes[node].prev;
    }

    int get_last_child(int node) const {
        int iter = get_child(node);
        int last_child = iter;
        while (iter != -1) {
            last_child = iter;
            iter = get_next(iter);
        }
        return last_child;
    }

    int node_insert(int parent, const std::string& key) {
        return add_node(Node(null, key, parent, get_last_child(parent)));
    }

    int node_append(int parent) {
        return add_node(Node(null, "", parent, get_last_child(parent)));
    }

    int map_access(int parent, const std::string& key) const {
        assert(get_value<map_t>(parent));
        int iter = get_child(parent);
        while (iter != -1) {
            if (nodes[iter].key == key) {
                return iter;
            }
            iter = get_next(iter);
        }
        return -1;
    }

    int list_access(int parent, std::size_t index) const {
        assert(get_value<list_t>(parent));
        int iter = get_child(parent);
        std::size_t i = 0;
        while (iter != -1 && i != index) {
            iter = get_next(iter);
            i++;
        }
        return iter;
    }

    void assign(int index, const value_t& value) {
        clear(index);
        nodes[index].value = value;
    }

    void erase(int index) {
        clear(index);
        if (int prev = get_prev(index); prev != -1) {
            nodes[prev].next = get_next(index);
        }
        if (int next = get_next(index); next != -1) {
            nodes[next].prev = get_prev(index);
        }
        if (int parent = get_parent(index); parent != -1 && nodes[parent].child == index) {
            nodes[parent].child = get_next(index);
        }

        if (index == nodes.size() - 1) {
            nodes.pop_back();
        } else {
            free.push(index);
        }
    }

    void clear(int index) {
        int iter = get_child(index);
        while (iter != -1) {
            int prev = iter;
            iter = get_next(iter);
            erase(prev);
        }
    }

    template <typename T>
    const T* get_value(int index) const {
        if (index == -1) {
            return nullptr;
        }
        return std::get_if<T>(&nodes[index].value);
    }

    template <typename T>
    T* get_value(int index) {
        if (index == -1) {
            return nullptr;
        }
        return std::get_if<T>(&nodes[index].value);
    }

    int add_node(const Node& node) {
        if (!free.empty()) {
            int result = free.top();
            free.pop();
            return result;
        }
        int result = nodes.size();
        nodes.push_back(node);
        return result;
    }

    std::vector<Node> nodes;
    std::stack<int> free;
    int root_index;
};

template <bool IsConst>
Object::Pointer_<IsConst> Object::Reference_<IsConst>::ptr() const {
    return Pointer_<IsConst>(parent, index);
}


// ================================= OLD ============================

#if 0

template <bool IsConst>
class Object_ {
public:
    using int_t = _object::int_t;
    using float_t = _object::float_t;
    using bool_t = _object::bool_t;
    using str_t = _object::str_t;
    using null_t = _object::null_t;
    static constexpr null_t null = _object::null;
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
    std::conditional_t<IsConst, const T&, T&> get() const {
        return std::get<T>(node().value);
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

    Object_ insert(const std::string& key, const value_t& value);
    Object_ operator[](const std::string& key) const;
    Object_ operator[](const char* key) const {
        return (*this)[std::string(key)];
    }

    Object_ append(const value_t& value) const;
    Object_ operator[](std::size_t index) const;
    Object_ operator[](int index) const {
        return (*this)[(std::size_t)index];
    }

    std::size_t size() const;

    Object_ clone() const;

    void set(const value_t& value) const;
    void erase() const;
    void clear() const;

private:
    Object_(state_t state, int index):
        state(state), index(index)
    {}

    std::conditional_t<IsConst, const Node&, Node&> node() const {
        return state->nodes[index];
    }

    Object_ create_node(const Node& node);

    state_t state;
    int index;

    template <bool OtherConst>
    friend class Object_;
};

using Object = Object_<false>;
using ConstObject = Object_<true>;
#endif

bool operator==(Object::ConstReference lhs, Object::ConstReference rhs);

// How merge and diff work:
// - merge(base, diff) applies "diff" on top of "base"
// - diff(base, modified) returns the difference between the two, such that
//   merge(base, diff(base, modified)) == modified
//   AND
//   diff(base, merge(base, diff)) == diff
//   (assuming diff doesn't contain redundant changes)
// - A diff object has the following properties:
//   - Value at the same address as the base overwrites this value
//   - If a key is present in the diff but not base, it is appended,
//     including parent maps.
//   - For the base/modified, a null value is treated equivalent to if the key
//     is not present. Similarly, if a map only contains null values, it is
//     treated the same as if it wasn't present.
//   - When a null value is in the diff, this means the value is overwritten to
//     null, but instead of setting to null, erases the key/value, since this is
//     treated as equivalent.
//   - Null values in lists will be retained if they aren't at the end of the list
//   - For lists, the diff will contain a map with the following properties:
//     - The keys are the indices in the base object
//     - If an index doesn't have a matching key, then it isn't modified
//     - Indices past the end of the original list will append. If there are gaps,
//       these will be set to null.
//     - If the modified list has fewer elements, then corresponding keys
//       for the erased indices will have null values.
//     - If an element is erased not at the end of the list, this is equivalent
//       to modifying all the subsequent elements to shift them down, and erasing
//       the last element. There is no wasy for diff(base, modified) to distinguish
//       between the two.

Object object_merge(Object::ConstReference base, Object::ConstReference diff);
Object object_diff(Object::ConstReference base, Object::ConstReference modified);

} // namespace datapack

std::ostream& operator<<(std::ostream& os, datapack::Object::ConstReference object);
