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
    class Iterator_;

    template <bool IsConst>
    class Reference_ {
        using object_t = std::conditional_t<IsConst, const Object*, Object*>;

    public:
        const Reference_& operator=(const value_t& value) const {
            static_assert(!IsConst);
            object->node_assign(index, value);
            return *this;
        }

        Reference_ operator[](const std::string& key) const {
            static_assert(!IsConst);
            if (is_null()) {
                object->node_assign(index, map_t());
            } else if (!is_map()) {
                throw ObjectException("Tried to access value by key on a non-map node");
            }

            int result = object->map_access(index, key);
            if (result == -1) {
                result = object->add_child(index, key);
            }
            return Reference_(object, result);
        }

        Reference_ operator[](std::size_t list_index) const {
            return Reference_(object, object->list_access(index, list_index));
        }

        Iterator_<IsConst> find(const std::string key) const;
        Reference_<IsConst> at(const std::string key) const {
            if (!is_map()) {
                throw ObjectException("Tried to access value by key on a non-map node");
            }
            int child_index = object->map_access(index, key);
            if (child_index == -1) {
                throw ObjectException("Could not find key '" + key + "'");
            }
            return ConstReference(this, child_index);
        }

        Reference_ insert(const std::string& key, const value_t& value) const {
            static_assert(!IsConst);
            if (is_null()) {
                object->node_assign(index, map_t());
            } else if (!is_map()) {
                throw ObjectException("Tried to access value by key on a non-map node");
            }

            int value_index = object->add_child(index, key);
            object->node_assign(value_index, value);
            return Reference_(object, value_index);
        }

        Reference_ append(const value_t& value) const {
            static_assert(!IsConst);
            if (is_null()) {
                object->node_assign(index, list_t());
            } else if (!is_list()) {
                throw ObjectException("Tried to append to a non-list node");
            }

            int value_index = object->add_child(index);
            object->node_assign(value_index, value);
            return Reference_(object, value_index);
        }

        void erase() const {
            static_assert(!IsConst);
            object->node_erase(index);
        }

        Object clone() const {
            return object->clone(index);
        }

        std::conditional_t<IsConst, const value_t&, value_t&> value() const {
            return object->nodes[index].value;
        }
        const std::string& key() const {
            return object->nodes[index].key;
        }


        bool is_map() const { return get_value<map_t>(); }
        bool is_list() const { return get_value<list_t>(); }
        bool is_null() const { return get_value<null_t>(); }
        std::conditional_t<IsConst, const int_t*, int_t>
            get_int() const { return get_value<int_t>(); }
        std::conditional_t<IsConst, const float_t*, float_t*>
            get_float() const { return get_value<float_t>(); }
        std::conditional_t<IsConst, const bool_t*, bool_t*>
            get_bool() const { return get_value<bool_t>(); }
        std::conditional_t<IsConst, const str_t*, str_t*>
            get_string() const { return get_value<str_t>(); }
        std::conditional_t<IsConst, const binary_t*, binary_t*>
            get_binary() const { return get_value<binary_t>(); }

        template <typename T>
        std::conditional_t<IsConst, const T*, T*> get_value() const {
            return object->template get_value<T>(index);
        }

        Iterator_<IsConst> iter() const;

        template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
        Reference_(const Reference_<OtherConst>& other):
            object(other.object), index(other.index)
        {}

    private:
        Reference_(object_t object, int index, bool from_iter = false):
            object(object), index(index)
        {
            if (!from_iter) {
                assert(index != -1);
            }
        }
        object_t object;
        int index;

        template <bool OtherConst>
        friend class Iterator_;
        friend class Object;
    };

    template <bool IsConst>
    class Iterator_ {
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
        Iterator_ parent() const {
            if (!object) return Iterator_();
            return Iterator_(object, object->get_parent(index));
        }
        Iterator_ child() const {
            if (!object) return Iterator_();
            return Iterator_(object, object->get_child(index));
        }
        Iterator_ prev() const {
            if (!object) return Iterator_();
            return Iterator_(object, object->get_prev(index));
        }
        Iterator_ next() const {
            if (!object) return Iterator_();
            return Iterator_(object, object->get_next(index));
        }

        template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
        Iterator_(const Iterator_<OtherConst>& other):
            object(other.object), index(other.index)
        {}

        Iterator_():
            object(nullptr), index(-1), reference(nullptr, -1, true)
        {}

    private:
        std::conditional_t<IsConst, const Node&, Node&> node() {
            return object->nodes[index];
        }

        Iterator_(object_t object, int index):
            object(object), index(index), reference(object, index, true)
        {}
        object_t object;
        int index;
        Reference_<IsConst> reference;
        friend class Object;
    };

public:
    using Reference = Reference_<false>;
    using ConstReference = Reference_<true>;
    using Iterator = Iterator_<false>;
    using ConstIterator = Iterator_<true>;

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
        node_assign(root_index, value);
        return Reference(this, root_index);
    }

    Reference operator[](const std::string& key) {
        if (is_null()) {
            node_assign(root_index, map_t());
        } else if (!is_map()) {
            throw ObjectException("Tried to access value by key on a non-map node");
        }

        int result = map_access(root_index, key);
        if (result == -1) {
            result = add_child(root_index, key);
        }
        return Reference(this, result);
    }

    ConstReference operator[](std::size_t list_index) const {
        if (!get_value<list_t>(root_index)) {
            throw ObjectException("Tried to access value by index on a non-list node");
        }
        return ConstReference(this, list_access(root_index, list_index));
    }

    ConstIterator find(const std::string key) const {
        if (!is_map()) {
            throw ObjectException("Tried to access value by key on a non-map node");
        }
        return ConstIterator(this, map_access(root_index, key));
    }

    ConstReference at(const std::string key) const {
        if (!is_map()) {
            throw ObjectException("Tried to access value by key on a non-map node");
        }
        int child_index = map_access(root_index, key);
        if (child_index == -1) {
            throw ObjectException("Could not find key '" + key + "'");
        }
        return ConstReference(this, child_index);
    }

    Reference at(const std::string key) {
        if (!is_map()) {
            throw ObjectException("Tried to access value by key on a non-map node");
        }
        int child_index = map_access(root_index, key);
        if (child_index == -1) {
            throw ObjectException("Could not find key '" + key + "'");
        }
        return Reference(this, child_index);
    }

    Reference insert(const std::string& key, const value_t& value) {
        if (is_null()) {
            node_assign(root_index, map_t());
        } else if (!is_map()) {
            throw ObjectException("Tried to insert value on non-map node");
        }
        int value_index = add_child(root_index, key);
        node_assign(value_index, value);
        return Reference(this, value_index);
    }

    Reference append(const value_t& value) {
        if (is_null()) {
            node_assign(root_index, list_t());
        } else if (!is_list()) {
            throw ObjectException("Tried to append to a non-list node");
        }
        int value_index = add_child(root_index);
        node_assign(value_index, value);
        return Reference(this, value_index);
    }

    void erase() {
        node_erase(root_index);
    }

    Object clone() const {
        return clone(root_index);
    }

    bool is_map() const { return get_value<map_t>(root_index); }
    bool is_list() const { return get_value<list_t>(root_index); }
    bool is_null() const { return get_value<null_t>(root_index); }
    int_t* get_int() { return get_value<int_t>(root_index); }
    const int_t* get_int() const { return get_value<int_t>(root_index); }
    float_t* get_float() { return get_value<float_t>(root_index); }
    const float_t* get_float() const { return get_value<float_t>(root_index); }
    bool_t* get_bool() { return get_value<bool_t>(root_index); }
    const bool_t* get_bool() const { return get_value<bool_t>(root_index); }
    str_t* get_string() { return get_value<str_t>(root_index); }
    const str_t* get_string() const { return get_value<str_t>(root_index); }
    binary_t* get_binary() { return get_value<binary_t>(root_index); }
    const binary_t* get_binary() const { return get_value<binary_t>(root_index); }

    Iterator iter() {
        return Iterator(this, root_index);
    }
    ConstIterator iter() const {
        return ConstIterator(this, root_index);
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

    int add_child(int parent, const std::string& key = "") {
        int last_child = get_last_child(parent);
        int node = add_node(Node(null, key, parent, last_child));
        if (last_child == -1) {
            nodes[parent].child = node;
        } else {
            nodes[last_child].next = node;
        }
        return node;
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

    void node_assign(int index, const value_t& value) {
        node_clear(index);
        nodes[index].value = value;
    }

    void node_erase(int index) {
        node_clear(index);
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

    void node_clear(int index) {
        int iter = get_child(index);
        while (iter != -1) {
            int prev = iter;
            iter = get_next(iter);
            node_erase(prev);
        }
    }

    Object clone(int index) const {
        Object result;
        result = nodes[index].value;

        if (!get_value<map_t>(index) && !get_value<list_t>(index)) {
            return result;
        }
        if (get_child(index) == -1) {
            return result;
        }

        std::stack<ConstIterator> from_stack;
        from_stack.push(ConstIterator(this, get_child(index)));
        std::stack<Iterator> to_stack;
        to_stack.push(result.iter());

        while (!from_stack.empty()) {
            auto from = from_stack.top();
            from_stack.pop();
            if (!from){
                to_stack.pop();
                continue;
            }
            from_stack.push(from.next());

            auto to = to_stack.top();
            Iterator new_to;
            if (to->is_map()) {
                new_to = to->insert(from->key(), from->value()).iter();
            }
            else if (to->is_list()) {
                new_to = to->append(from->value()).iter();
            }
            else {
                assert(false);
            }

            if (from->is_map() || from->is_list()) {
                from_stack.push(from.child());
                to_stack.push(new_to);
            }
        }

        return result;
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
Object::Iterator_<IsConst> Object::Reference_<IsConst>::iter() const {
    return Iterator_<IsConst>(object, index);
}

template <bool IsConst>
Object::Iterator_<IsConst> Object::Reference_<IsConst>::find(const std::string key) const {
    return Iterator_<IsConst>(object, object->map_access(index, key));
}

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

Object merge(Object::ConstReference base, Object::ConstReference diff);
Object diff(Object::ConstReference base, Object::ConstReference modified);

bool operator==(const Object::ConstReference& lhs, const Object::ConstReference& rhs);
inline bool operator==(const Object::Reference& lhs, const Object::Reference& rhs) {
    return Object::ConstReference(lhs) == Object::ConstReference(rhs);
}
inline bool operator==(const Object& lhs, const Object& rhs) {
    return Object::ConstReference(lhs) == Object::ConstReference(rhs);
}
std::ostream& operator<<(std::ostream& os, Object::ConstReference object);

} // namespace datapack
