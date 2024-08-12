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

class Object {
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

public:
    class LookupException: public std::exception {
    public:
        LookupException(const std::string& message):
            message(message)
        {}
    private:
        const char* what() const noexcept override {
            return message.c_str();
        }
        std::string message;
    };

    class ValueException: public std::exception {
    public:
        ValueException(const std::string& message):
            message(message)
        {}
    private:
        const char* what() const noexcept override {
            return message.c_str();
        }
        std::string message;
    };

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
            object->assign(index, value);
            return *this;
        }

        Reference_ operator[](const std::string& key) const {
            static_assert(!IsConst);
            return Reference_(object, object->map_access_mutable(index, key));
        }

        Reference_ operator[](std::size_t list_index) const {
            return Reference_(object, object->list_access(index, list_index));
        }

        Iterator_<IsConst> find(const std::string key) const;

        Reference_<IsConst> at(const std::string key) const {
            return Reference_(object, object->map_access(index, key, false));
        }

        Reference_ insert(const std::string& key, const value_t& value) const {
            return Reference_(object, object->insert(index, key, value));
        }

        Reference_ append(const value_t& value) const {
            return Reference_(object, object->append(index, value));
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

        bool is_map() const { return object->is_map(index); }
        bool is_list() const { return object->is_list(index); }
        bool is_null() const { return object->is_null(index); }
        std::conditional_t<IsConst, const int_t*, int_t*>
            get_int() { return object->get_int(index); }
        std::conditional_t<IsConst, const float_t*, float_t*>
            get_float() { return object->get_float(index); }
        std::conditional_t<IsConst, const bool_t*, bool_t*>
            get_bool() { return object->get_bool(index); }
        std::conditional_t<IsConst, const str_t*, str_t*>
            get_string() { return object->get_int(index); }
        std::conditional_t<IsConst, const binary_t*, binary_t*>
            get_binary() { return object->get_binary(index); }

        Iterator_<IsConst> iter() const;

        template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
        Reference_(const Reference_<OtherConst>& other):
            object(other.object), index(other.index)
        {}

    private:
        Reference_(object_t object, int index):
            object(object), index(index)
        {}
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
        const Reference_<IsConst>& operator*() const {
            return ref;
        }
        const Reference_<IsConst>* operator->() const {
            return &ref;
        }

        operator bool() const {
            return valid();
        }

        Iterator_ parent() const {
            if (!valid()) return Iterator_();
            return Iterator_(ref.object, node().parent);
        }
        Iterator_ child() const {
            if (!valid()) return Iterator_();
            return Iterator_(ref.object, node().child);
        }
        Iterator_ prev() const {
            if (!valid()) return Iterator_();
            return Iterator_(ref.object, node().prev);
        }
        Iterator_ next() const {
            if (!valid()) return Iterator_();
            return Iterator_(ref.object, node().next);
        }

        template <bool OtherConst, typename = std::enable_if_t<!OtherConst || IsConst>>
        Iterator_(const Iterator_<OtherConst>& other):
            ref(other.ref)
        {}

        Iterator_():
            ref(nullptr, 0)
        {}

    private:
        bool valid() const {
            return ref.object != nullptr && ref.index != -1;
        }
        const Node& node() const {
            return ref.object->nodes[ref.index];
        }

        Iterator_(object_t object, int index):
            ref(object, index)
        {}
        const Reference_<IsConst> ref;
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
        assign(root_index, value);
        return Reference(this, root_index);
    }

    Reference operator[](const std::string& key) {
        return Reference(this, map_access_mutable(root_index, key));
    }

    ConstReference operator[](std::size_t list_index) const {
        return ConstReference(this, list_access(root_index, list_index));
    }

    Reference operator[](std::size_t list_index) {
        return Reference(this, list_access(root_index, list_index));
    }

    ConstIterator find(const std::string key) const {
        return ConstIterator(this, map_access(root_index, key, false));
    }

    Iterator find(const std::string key) {
        return Iterator(this, map_access(root_index, key, false));
    }

    ConstReference at(const std::string key) const {
        return ConstReference(this, map_access(root_index, key, true));
    }

    Reference at(const std::string key) {
        return Reference(this, map_access(root_index, key, true));
    }

    Reference insert(const std::string& key, const value_t& value) {
        return Reference(this, insert(root_index, key, value));
    }

    Reference append(const value_t& value) {
        return Reference(this, append(root_index, value));
    }

    void erase() {
        erase(root_index);
    }

    Object clone() const {
        return clone(root_index);
    }

    bool is_map() const { return is_map(root_index); }
    bool is_list() const { return is_list(root_index); }
    bool is_null() const { return is_null(root_index); }
    int_t* get_int() { return get_int(root_index); }
    const int_t* get_int() const { return get_int(root_index); }
    float_t* get_float() { return get_float(root_index); }
    const float_t* get_float() const { return get_float(root_index); }
    bool_t* get_bool() { return get_bool(root_index); }
    const bool_t* get_bool() const { return get_bool(root_index); }
    str_t* get_string() { return get_string(root_index); }
    const str_t* get_string() const { return get_string(root_index); }
    binary_t* get_binary() { return get_binary(root_index); }
    const binary_t* get_binary() const { return get_binary(root_index); }

    Iterator iter() {
        return Iterator(this, root_index);
    }
    ConstIterator iter() const {
        return ConstIterator(this, root_index);
    }

private:
    bool is_map(int index) const {
        return get_value<map_t>(index);
    }
    bool is_list(int index) const {
        return get_value<list_t>(index);
    }
    bool is_null(int index) const {
        return get_value<null_t>(index);
    }
    int_t* get_int(int index) {
        return get_value<int_t>(index);
    }
    const int_t* get_int(int index) const {
        return get_value<int_t>(index);
    }
    float_t* get_float(int index) {
        return get_value<float_t>(index);
    }
    const float_t* get_float(int index) const {
        return get_value<float_t>(index);
    }
    bool_t* get_bool(int index) {
        return get_value<bool_t>(index);
    }
    const bool_t* get_bool(int index) const {
        return get_value<bool_t>(index);
    }
    str_t* get_string(int index) {
        return get_value<str_t>(index);
    }
    const str_t* get_string(int index) const {
        return get_value<str_t>(index);
    }
    binary_t* get_binary(int index) {
        return get_value<binary_t>(index);
    }
    const binary_t* get_binary(int index) const {
        return get_value<binary_t>(index);
    }

    template <typename T>
    const T* get_value(std::size_t index) const {
        return get_value<T>(index);
    }

    template <typename T>
    T* get_value(std::size_t index) {
        return get_value<T>(index);
    }

    int get_last_child(int node) const {
        int iter = nodes[node].child;
        int last_child = iter;
        while (iter != -1) {
            last_child = iter;
            iter = nodes[iter].next;
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

    int map_access(int parent, const std::string& key, bool required) const {
        if (!std::get_if<map_t>(&nodes[parent].value)) {
            throw ValueException("Tried to access value by key on a non-map node");
        }
        int child = nodes[parent].child;
        while (child != -1) {
            if (nodes[child].key == key) {
                return child;
            }
            child = nodes[child].next;
        }
        if (required) {
            throw LookupException("Could not find key '" + key + "'");
        }
        return -1;
    }

    int map_access_mutable(int parent, const std::string& key) {
        if (std::get_if<null_t>(&nodes[parent].value)) {
            nodes[parent].value = map_t();
        } else if (!std::get_if<map_t>(&nodes[parent].value)) {
            throw ValueException("Tried to access value by key on a non-map node");
        }

        int child_index = map_access(parent, key, false);
        if (!child_index) {
            child_index = add_child(parent, key);
        }
        return child_index;
    }

    int list_access(int parent, std::size_t index) const {
        assert(get_value<list_t>(parent));
        int iter = nodes[parent].child;
        std::size_t i = 0;
        while (iter != -1 && i != index) {
            iter = nodes[iter].next;
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
        if (int prev = nodes[index].prev; prev != -1) {
            nodes[prev].next = nodes[index].next;
        }
        if (int next = nodes[index].next; next != -1) {
            nodes[next].prev = nodes[index].prev;
        }
        if (int parent = nodes[index].parent; parent != -1
            && nodes[parent].child == index)
        {
            nodes[parent].child = nodes[index].next;
        }

        if (index == nodes.size() - 1) {
            nodes.pop_back();
        } else {
            free.push(index);
        }
    }

    void clear(int index) {
        int iter = nodes[index].child;
        while (iter != -1) {
            int prev = iter;
            iter = nodes[iter].next;
            erase(prev);
        }
    }

    int insert(int parent, const std::string& key, const value_t& value) {
        if (std::get_if<null_t>(&nodes[parent].value)) {
            assign(parent, map_t());
        } else if (!std::get_if<map_t>()) {
            throw ValueException("Tried to insert with a non-map node");
        }
        int child = add_child(parent, key);
        assign(child, value);
        return child;
    }

    int append(int parent, const value_t& value) {
        if (std::get_if<null_t>(&nodes[parent].value)) {
            assign(parent, list_t());
        } else if (!is_list()) {
            throw ValueException("Tried to append with a non-list node");
        }
        int child = add_child(parent);
        assign(child, value);
        return child;
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
