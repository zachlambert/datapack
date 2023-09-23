#pragma once

#include <iostream>
#include <vector>
#include <variant>
#include <cstdint>
#include <stack>
#include <cstdlib>
#include <cstring>
#include <assert.h>
#include <iostream>
#include <mutex>
#include <memory>
#include <limits>
#include <shared_mutex>
#include <type_traits>
#include <sstream>
#include <concepts>

#include <optional>
#include <unordered_map>
#include <map>
#include <array>


namespace parrot {

class Json {
    static constexpr size_t invalid_index = std::numeric_limits<size_t>::max();
    struct ObjectElement {
        size_t head;
        ObjectElement():
            head(invalid_index)
        {}
    };
    struct ObjectIter {
        std::string key;
        size_t value;
        size_t next;
        ObjectIter(const std::string_view& key, size_t value):
            key(key),
            value(value),
            next(invalid_index)
        {}
        ObjectIter(std::string_view&& key, size_t value):
            key(key),
            value(value),
            next(invalid_index)
        {}
    };
    struct ArrayElement {
        size_t head;
        ArrayElement():
            head(invalid_index)
        {}
    };
    struct ArrayIter {
        size_t index;
        size_t value;
        size_t next;
        ArrayIter(size_t index, size_t value):
            index(index),
            value(value),
            next(invalid_index)
        {}
    };

    // Primitives
    struct StringElement {
        typedef std::string value_t;
        std::string value;
        StringElement():
            value("")
        {}
    };
    struct NumberElement {
        typedef double value_t;
        double value;
        NumberElement():
            value(0)
        {}
    };
    struct BoolElement {
        typedef bool value_t;
        bool value;
        BoolElement():
            value(false)
        {}
    };
    struct NullElement {};

    // Used in diff
    struct SubtractElement {};

    using Element = std::variant<
        ObjectElement,
        ObjectIter,
        ArrayElement,
        ArrayIter,
        StringElement,
        NumberElement,
        BoolElement,
        NullElement,
        SubtractElement
    >;

    template <bool IsConst>
    class NodeTemplate {
        typedef std::conditional_t<IsConst, const Json*, Json*> parent_ptr_t;
        template <typename T>
        using ptr_t = std::conditional_t<IsConst, const T*, T*>;
        template <typename T>
        using ref_t = std::conditional_t<IsConst, const T&, T&>;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = NodeTemplate<IsConst>;
        using reference = NodeTemplate<IsConst>&;
        using pointer = NodeTemplate<IsConst>*;

        // Container methods: checks

        bool is_object()const {
            return std::get_if<ObjectElement>(&element());
        }
        bool is_array()const {
            return std::get_if<ArrayElement>(&element());
        }
        bool is_container()const {
            return is_object() || is_array();
        }
        void set_object()const {
            parent->template free_data<IsConst>(*this);
            element() = ObjectElement();
        }
        void set_array()const {
            parent->template free_data<IsConst>(*this);
            element() = ArrayElement();
        }

        // Container methods: iteration

        NodeTemplate begin()const {
            return head();
        }

        NodeTemplate end()const {
            assert(is_container());
            return invalid();
        }

        NodeTemplate<true> cbegin()const {
            if (auto value = std::get_if<ObjectElement>(&element())) {
                return NodeTemplate<true>(parent, value->head);
            }
            else if (auto value = std::get_if<ArrayElement>(&element())) {
                return NodeTemplate<true>(parent, value->head);
            }
            assert(false);
            return NodeTemplate<true>(parent, invalid_index);
        }

        NodeTemplate<true> cend()const {
            assert(is_container());
            return NodeTemplate<true>(parent, invalid_index);
        }

        // Iterator methods (for when NodeTemplate acts as an iterator)

        NodeTemplate operator*() {
            if (!valid()) return invalid();
            if (auto value = std::get_if<ObjectIter>(&element())) {
                return NodeTemplate(parent, value->value);
            }
            else if (auto value = std::get_if<ArrayIter>(&element())) {
                return NodeTemplate(parent, value->value);
            }
            assert(false);
            return NodeTemplate(parent, invalid_index);
        }

        NodeTemplate& operator++() {
            (*this) = next();
            return *this;
        }

        NodeTemplate operator++(int) {
            NodeTemplate iter = *this;
            (*this) = next();
            return iter;
        }

        template <bool OtherConst>
        bool operator==(const NodeTemplate<OtherConst>& other)const {
            return parent == other.parent && element_index == other.element_index;
        }

        template <bool OtherConst>
        bool operator!=(const NodeTemplate<OtherConst>& other)const {
            return !(*this == other);
        }

        std::string_view key()const {
            return std::get<ObjectIter>(element()).key;
        }

        size_t index()const {
            return std::get<ArrayIter>(element()).index;
        }

        bool has_key()const {
            return std::get_if<ObjectIter>(&element());
        }

        bool has_index()const {
            return std::get_if<ArrayIter>(&element());
        }

        bool is_iterator()const {
            return has_key() || has_index();
        }

        // Container methods: size/empty

        size_t size()const {
            size_t i = 0;
            for (auto iter = begin(); iter.valid(); iter++) i++;
            return i;
        }

        bool empty()const {
            return begin() == end();
        }

        // Container methods: operator[]
        // - Return the value at the given key or index
        // - If mutable, construct if it doesn't exist
        // - If current node is null, and mutable, construct the appropriate
        //   container automatically

        NodeTemplate operator[](const std::string_view& key)const {
            if (!valid()) return invalid();
            if constexpr(!IsConst) {
                if (std::get_if<NullElement>(&element())) {
                    element() = ObjectElement();
                }
            }
            assert(std::get_if<ObjectElement>(&element()));

            auto prev = end();
            for (auto iter = begin(); iter.valid(); iter++) {
                if (iter.key() == key) {
                    return *iter;
                }
                prev = iter;
            }
            if constexpr(!IsConst) {
                // Get a copy before allocating indices, since if the key comes
                // from another node, and is a small string so uses small-string
                // optimisation, it may be invalidated if the nodes vector
                // needs to be reallocated.
                // Can move the string so this isn't any extra cost.
                std::string key_copy = std::string(key);

                NodeTemplate iter = parent->template allocate_index<IsConst>();
                NodeTemplate value = parent->template allocate_index<IsConst>();
                if (prev != end()) {
                    prev.set_next(iter);
                } else {
                    set_head(iter);
                }
                iter.element() = ObjectIter(std::move(key_copy), value.element_index);
                return value;
            } else {
                return invalid();
            }
        }

        NodeTemplate operator[](size_t index)const {
            if (!valid()) return invalid();
            if constexpr(!IsConst) {
                if (std::get_if<NullElement>(&element())) {
                    element() = ArrayElement();
                }
            }
            assert(std::get_if<ArrayElement>(&element()));

            auto prev = end();
            for (auto iter = begin(); iter.valid(); iter++) {
                if (iter.index() == index) {
                    return *iter;
                }
                prev = iter;
            }
            if constexpr(!IsConst) {
                NodeTemplate new_iter = parent->template allocate_index<IsConst>();
                NodeTemplate new_value = parent->template allocate_index<IsConst>();
                if (prev != end()) {
                    prev.set_next(new_iter);
                } else {
                    set_head(new_iter);
                }
                new_iter.element() = ArrayIter(index, new_value.element_index);
                return new_value;
            } else {
                return invalid();
            }
        }

        template <bool OtherConst>
        NodeTemplate operator[](const NodeTemplate<OtherConst> compare)const {
            if (compare.has_key()) {
                return (*this)[compare.key()];
            }
            else if (compare.has_index()) {
                return (*this)[compare.index()];
            }
            else {
                assert(false);
                return invalid();
            }
        }

        // Container methods: find
        // - Return the node for the iterator, for the given key or index
        // - Never construct nodes automatically
        // - Return invalid if not found, or the current node is invalid
        //   but assert that it is a container if the node is valid.

        NodeTemplate find(std::string_view key)const
        {
            if (!valid()) return invalid();
            assert(is_object());
            for (auto iter = begin(); iter.valid(); iter++) {
                if (iter.key() == key) {
                    return iter;
                }
            }
            return invalid();
        }
        NodeTemplate find(size_t index)const
        {
            if (!valid()) return invalid();
            assert(is_array());
            for (auto iter = begin(); iter.valid(); iter++) {
                if (iter.index() == index) {
                    return iter;
                }
            }
            return invalid();
        }
        template <bool OtherConst>
        NodeTemplate find(const NodeTemplate<OtherConst> compare) {
            if (compare.has_key()) {
                return find(compare.key());
            }
            else if (compare.has_index()) {
                return find(compare.index());
            }
            else {
                assert(false);
                return invalid();
            }
        }

        // Container methods: mutation
        // - Return the node iterator for the value created

        NodeTemplate insert(std::string key)const
        {
            // Note: construct a key, in case the string passed to this comes
            // from another iterator, which may be moved on allocation of
            // another node.
            // Can call std::move on the key anyway, so this is no extra cost

            if constexpr(!IsConst) {
                if (std::get_if<NullElement>(&element())) {
                    element() = ObjectElement();
                }
            }
            assert(std::get_if<ObjectElement>(&element()));

            NodeTemplate new_iter = parent->template allocate_index<IsConst>();
            NodeTemplate new_value = parent->template allocate_index<IsConst>();

            auto iter = begin();
            auto prev = end();
            while (iter != end()) {
                prev = iter;
                iter++;
            }
            if (prev == end()) {
                set_head(new_iter);
            }
            else {
                prev.set_next(new_iter);
            }

            new_iter.element() = ObjectIter(std::move(key), new_value.element_index);
            return new_iter;
        }

        NodeTemplate push_back()const
        {
            if constexpr(!IsConst) {
                if (std::get_if<NullElement>(&element())) {
                    element() = ArrayElement();
                }
            }
            assert(std::get_if<ArrayElement>(&element()));

            NodeTemplate new_iter = parent->template allocate_index<IsConst>();
            NodeTemplate new_value = parent->template allocate_index<IsConst>();

            size_t i = 0;
            auto iter = begin();
            auto prev = end();
            while (iter != end()) {
                prev = iter;
                iter++;
                i++;
            }
            if (prev == end()) {
                set_head(new_iter);
            }
            else {
                prev.set_next(new_iter);
            }

            new_iter.element() = ArrayIter(i, new_value.element_index);
            return new_iter;
        }

        template <bool OtherConst>
        void erase(const NodeTemplate<OtherConst>& compare)
        {
            if (!compare.valid()) return;

            auto prev = invalid();
            for (auto iter = begin(); iter.valid(); iter++) {
                if (iter == compare) {
                    if (prev.valid()) {
                        prev.set_next(iter.next());
                    } else {
                        set_head(iter.next());
                    }
                    parent->template free_data<IsConst>(*iter);
                    parent->template free_index<IsConst>(*iter);
                    parent->template free_index<IsConst>(iter);
                    return;
                }
                prev = iter;
            }
            assert(false);
        }

        // Primitive methods: get values
        // - primitive() returns a references. If not valid, constructs if mutable,
        //   fails if const.
        // - primitive_if() returns pointers. If not valid, returns nullptr.

        bool is_primitive()const
        {
            return string_if() || number_if() || boolean_if() || is_null() || is_subtract();
        }

        ref_t<std::string> string()const { return primitive<StringElement>(); }
        ptr_t<std::string> string_if()const { return primitive_if<StringElement>(); }

        ref_t<double> number()const { return primitive<NumberElement>(); }
        ptr_t<double> number_if()const { return primitive_if<NumberElement>(); }

        ref_t<bool> boolean()const { return primitive<BoolElement>(); }
        ptr_t<bool> boolean_if()const { return primitive_if<BoolElement>(); }

        // Primitive methods: Set and query flag-like elements

        void set_null()const {
            parent->template free_data<IsConst>(*this);
            element() = NullElement();
        }
        bool is_null()const {
            return std::get_if<NullElement>(&element());
        }

        void set_subtract()const {
            parent->template free_data<IsConst>(*this);
            element() = SubtractElement();
        }
        bool is_subtract()const {
            return std::get_if<SubtractElement>(&element());
        }

        // Primitive methods: Assignment by value, as well as corresponding
        // insert and push_back

        template <typename T>
        NodeTemplate operator=(const T& value);

        // Special case
        NodeTemplate operator=(const char* value) {
            string() = value;
            return *this;
        }
        template <size_t N>
        NodeTemplate operator=(const char (&value)[N]) {
            string() = value;
            return *this;
        }

        template <typename T>
        NodeTemplate insert(const std::string_view& key, const T& value) {
            auto iter = insert(key);
            *iter = value;
            return iter;
        }

        template <typename T>
        NodeTemplate push_back(const T& value) {
            auto iter = push_back();
            *iter = value;
            return iter;
        }

        // Read methods: Read into a reference and return true if successfuly

        template <typename T>
        bool read(T& value);

        // Other comparison methods

        template <bool OtherConst>
        bool compare(const NodeTemplate<OtherConst>& other)const
        {
            if (auto value = string_if()) {
                auto value2 = other.string_if();
                return value2 && *value == *value2;
            }
            else if (auto value = number_if()) {
                auto value2 = other.number_if();
                return value2 && *value == *value2;
            }
            else if (boolean_if() && other.boolean_if()) {
                auto value2 = other.number_if();
                return value2 && *value == *value2;
            }
            else if (is_null() && other.is_null()) {
                return true;
            }
            else if (is_object()) {
                return other.is_object();
            }
            else if (is_array()) {
                return other.is_array();
            }
            return false;
        }

        bool valid()const
        {
            return element_index != invalid_index;
        }

        template <bool OtherConst, typename = typename std::enable_if_t<IsConst && !OtherConst>>
        NodeTemplate(NodeTemplate<OtherConst> other):
            parent(other.parent),
            element_index(other.element_index)
        {}

    private:
        NodeTemplate head()const
        {
            if (auto container = std::get_if<ObjectElement>(&element())) {
                return NodeTemplate(parent, container->head);
            }
            if (auto container = std::get_if<ArrayElement>(&element())) {
                return NodeTemplate(parent, container->head);
            }
            assert(false);
            return invalid();
        }
        void set_head(const NodeTemplate& node)const
        {
            if (auto container = std::get_if<ObjectElement>(&element())) {
                container->head = node.element_index;
                return;
            }
            if (auto container = std::get_if<ArrayElement>(&element())) {
                container->head = node.element_index;
                return;
            }
            assert(false);
        }

        NodeTemplate next()const {
            if (auto value = std::get_if<ObjectIter>(&element())) {
                return NodeTemplate(parent, value->next);
            }
            if (auto value = std::get_if<ArrayIter>(&element())) {
                return NodeTemplate(parent, value->next);
            }
            assert(false);
            return invalid();
        }
        void set_next(const NodeTemplate& node)const
        {
            if (auto iter = std::get_if<ObjectIter>(&element())) {
                iter->next = node.element_index;
                return;
            }
            if (auto iter = std::get_if<ArrayIter>(&element())) {
                iter->next = node.element_index;
                return;
            }
            assert(false);
        }

        typedef std::conditional_t<IsConst, const Element&, Element&> element_ref;
        element_ref element()const
        {
            return parent->elements[element_index];
        }

        template <typename T>
        std::conditional_t<IsConst, const typename T::value_t&, typename T::value_t&> primitive()const
        {
            auto primitive = std::get_if<T>(&element());
            if constexpr(!IsConst) {
                if (!primitive) {
                    parent->free_data(*this);
                    element() = T();
                    return std::get_if<T>(&element())->value;
                }
            }
            assert(primitive);
            return primitive->value;
        }

        template <typename T>
        std::conditional_t<IsConst, const typename T::value_t*, typename T::value_t*> primitive_if()const
        {
            if (!valid()) return nullptr;
            auto primitive = std::get_if<T>(&element());
            if (primitive) {
                return &primitive->value;
            } else {
                return nullptr;
            }
        }

        NodeTemplate invalid()const
        {
            return NodeTemplate(parent, invalid_index);
        }

        NodeTemplate(parent_ptr_t parent, size_t element_index):
            parent(parent),
            element_index(element_index)
        {}
        parent_ptr_t parent;
        size_t element_index;

        friend class Json;
    };

    template <bool IsConst>
    NodeTemplate<IsConst> find_pointer(const std::string_view& pointer)
    {
        if (!pointer.empty() && pointer[0] != '/') return NodeTemplate<IsConst>(this, invalid_index);

        auto node = get();
        if (!node.is_object()) {
            return NodeTemplate<IsConst>(this, invalid_index);
        }

        size_t pos1 = 0;
        size_t pos2;

        while (pos1 != pointer.size() && node.valid()) {
            pos2 = pointer.find('/', pos1+1);
            if (pos2 == std::string::npos) pos2 = pointer.size();
            node = node.find(pointer.substr(pos1+1, pos2-(pos1+1)));
            if (!node.valid()) return node.invalid();
            node = *node;
            pos1 = pos2;
        }
        return node;
    }

public:
    Json()
    {
        elements.emplace_back();
    }
    Json(const Json& other):
        elements(other.elements),
        free_elements(other.free_elements)
    {}
    Json(Json&& other):
        elements(other.elements),
        free_elements(other.free_elements)
    {}

    typedef NodeTemplate<true> ConstNode;
    typedef NodeTemplate<false> Node;

    ConstNode get()const
    {
        return ConstNode(this, 0);
    }
    Node get()
    {
        return Node(this, 0);
    }
    ConstNode get(const std::string_view& pointer)const
    {
        return const_cast<Json*>(this)->find_pointer<true>(pointer);
    }
    Node get(const std::string_view& pointer)
    {
        return find_pointer<false>(pointer);
    }

    operator ConstNode()const
    {
        return get();
    }
    operator Node()
    {
        return get();
    }

    Json& operator=(const std::string_view& string);
    Json& operator=(const Json& other);
    Json& operator=(const ConstNode& node);

private:
    template <bool IsConst>
    NodeTemplate<IsConst> allocate_index()
    {
        std::scoped_lock<std::mutex> free_lock(free_mutex);
        if (!free_elements.empty()) {
            size_t index = free_elements.back();
            free_elements.pop_back();
            std::shared_lock<std::shared_mutex> lock(elements_mutex);
            elements[index] = NullElement();
            return NodeTemplate<IsConst>(this, index);
        }
        std::scoped_lock<std::shared_mutex> lock(elements_mutex);
        elements.emplace_back(NullElement());
        return NodeTemplate<IsConst>(this, elements.size() - 1);
    }

    template <bool IsConst>
    void free_index(const NodeTemplate<IsConst> node)
    {
        std::scoped_lock<std::mutex> free_lock(free_mutex);
        if (node.element_index == elements.size() - 1) {
            elements.pop_back();
        } else {
            free_elements.push_back(node.element_index);
        }
    }

    template <bool IsConst>
    void free_data(const NodeTemplate<IsConst> node)
    {
        assert(node.valid());
        if (!node.is_container()) return;

        std::stack<NodeTemplate<IsConst>> stack;
        stack.push(node.begin());

        while (!stack.empty()) {
            auto node = stack.top();
            if (!node.valid()) {
                stack.pop();
                continue;
            }
            stack.top()++;

            if ((*node).is_container()) {
                stack.push((*node).begin());
            }
            free_index(node);
            free_index(*node);
        }
    }

    std::vector<Element> elements;
    std::vector<size_t> free_elements;
    std::shared_mutex elements_mutex;
    std::mutex free_mutex;
};

typedef Json::ConstNode JsonConstNode;
typedef Json::Node JsonNode;

std::ostream& operator<<(std::ostream& os, JsonConstNode node);
std::istream& operator>>(std::istream& is, JsonNode node);

void copy(JsonConstNode from, JsonNode to);
void diff(JsonConstNode from, JsonConstNode to, JsonNode diff);
void apply(JsonConstNode diff, JsonNode to);


template <typename T>
struct json_functions {};

template <typename T>
concept readable_json_nonmember = requires(JsonConstNode const_node, T& value) {
    { json_functions<T>::from_json(const_node, value) } -> std::convertible_to<bool>;
};
template <typename T>
concept readable_json_member = requires(JsonConstNode const_node, T& value) {
    { value.from_json(const_node) } -> std::convertible_to<bool>;
};
template <typename T>
concept readable_json = readable_json_nonmember<T> || readable_json_member<T>;

template <typename T>
concept writable_json_nonmember = requires(const T& value, JsonNode node) {
    { json_functions<T>::to_json(value, node) };
};
template <typename T>
concept writable_json_member = requires(const T& value, JsonNode node) {
    { value.to_json(node) };
};
template <typename T>
concept writable_json = writable_json_nonmember<T> || writable_json_member<T>;

template <bool IsConst>
template <typename T>
bool Json::NodeTemplate<IsConst>::read(T& value) {
    assert(!is_iterator());
    static_assert(readable_json<T>);
    if constexpr(readable_json_nonmember<T>) {
        return json_functions<T>::from_json(*this, value);
    }
    if constexpr(readable_json_member<T>) {
        static_assert(!readable_json_nonmember<T>);
        return value.from_json(*this);
    }
    return false;
}

template <bool IsConst>
template <typename T>
Json::NodeTemplate<IsConst> Json::NodeTemplate<IsConst>::operator=(const T& value) {
    static_assert(!IsConst);
    static_assert(writable_json<T>);
    if constexpr(writable_json_nonmember<T>) {
        json_functions<T>::to_json(value, *this);
    }
    if constexpr(writable_json_member<T>) {
        static_assert(!writable_json_nonmember<T>);
        value.to_json(*this);
    }
    return *this;
}


template <>
struct json_functions<std::string> {
    static bool from_json(JsonConstNode node, std::string& value) {
        if (auto pointer = node.string_if()) {
            value = *pointer;
            return true;
        }
        return false;
    }
    static void to_json(const std::string& value, JsonNode node) {
        node.string() = value;
    }
};

template <>
struct json_functions<bool> {
    static bool from_json(parrot::JsonConstNode node, bool& value) {
        if (auto pointer = node.boolean_if()) {
            value = *pointer;
            return true;
        }
        return false;
    }
    static void to_json(const bool& value, JsonNode node) {
        node.boolean() = value;
    }
};

template <typename T>
concept is_proper_integer = std::is_integral_v<T> && !std::is_same_v<bool, T>;

template <is_proper_integer T>
struct json_functions<T> {
    static bool from_json(parrot::JsonConstNode node, T& value) {
        if (auto pointer = node.number_if()) {
            value = *pointer;
            return true;
        }
        return false;
    }
    static void to_json(const T& value, JsonNode node) {
        node.number() = value;
    }
};

template <std::floating_point T>
struct json_functions<T> {
    static bool from_json(parrot::JsonConstNode node, T& value) {
        if (auto pointer = node.number_if()) {
            value = *pointer;
            return true;
        }
        return false;
    }
    static void to_json(const T& value, JsonNode node) {
        node.number() = value;
    }
};

template <>
struct json_functions<char*> {
    static void to_json(const char*& value, JsonNode node) {
        node.string() = value;
    }
};

template <>
struct json_functions<std::string_view> {
    static void to_json(const std::string_view& value, JsonNode node) {
        node.string() = value;
    }
};

template <typename T>
struct json_functions<std::optional<T>> {
    static bool from_json(JsonConstNode node, std::optional<T>& optional) {
        static_assert(readable_json<T>);
        if (node.is_null()) {
            optional = std::nullopt;
            return true;
        }
        else {
            optional = T();
            return node.read(optional.value());
        }
    }
    static void to_json(const std::optional<T>& optional, JsonNode node) {
        static_assert(writable_json<T>);
        if (optional.has_value()) {
            node = optional.value();
        }
        else {
            node.set_null();
        }
    }
};

template <typename T>
struct json_functions<std::vector<T>> {
    static bool from_json(JsonConstNode node, std::vector<T>& vector) {
        static_assert(readable_json<T>);
        if (!node.is_array()) {
            return false;
        }
        vector.resize(node.size());
        for (size_t i = 0; i < node.size(); ++i) {
            if (!node[i].read(vector[i])) {
                return false;
            }
        }
        return true;
    }
    static void to_json(const std::vector<T>& vector, parrot::JsonNode node) {
        static_assert(writable_json<T>);
        node.set_array();
        for (size_t i = 0; i < vector.size(); ++i) {
            auto value = node.push_back();
            *value = vector[i];
        }
    }
};

template <typename T, std::size_t N>
struct json_functions<std::array<T, N>> {
    static bool from_json(JsonConstNode node, std::array<T, N>& array) {
        static_assert(readable_json<T>);
        if (!node.is_array()) return false;
        if (node.size() != N) return false;
        for (std::size_t i = 0; i < N; ++i) {
            if (!node[i].read(array[i])) return false;
        }
        return true;
    }
    static void to_json(const std::array<T, N>& array, JsonNode node) {
        static_assert(writable_json<T>);
        node.set_array();
        for (std::size_t i = 0; i < array.size(); i++) {
            node[i] = array[i];
        }
    }
};

template <typename T>
struct json_functions<std::unordered_map<std::string, T>> {
    static bool from_json(parrot::JsonConstNode node, std::unordered_map<std::string, T>& map) {
        if (!node.is_object()) return false;
        bool valid = true;
        for (auto iter = node.begin(); iter != node.end(); iter++) {
            valid &= (*iter).read(map[std::string(iter.key())]);
        }
        return valid;
    }
    static bool to_json(const std::unordered_map<std::string, T>& map, parrot::JsonNode node) {
        node.set_object();
        for (const auto& pair: map) {
            node[pair.first] = pair.second;
        }
        return true;
    }
};

template <typename T>
struct json_functions<std::map<std::string, T>> {
    bool from_json(parrot::JsonConstNode node, std::map<std::string, T>& map) {
        if (!node.is_object()) return false;
        bool valid = true;
        for (auto iter = node.begin(); iter != node.end(); iter++) {
            valid &= (*iter).read(map[std::string(iter.key())]);
        }
        return valid;
    }
    bool to_json(const std::map<std::string, T>& map, parrot::JsonNode node) {
        node.set_object();
        for (const auto& pair: map) {
            node[pair.first] = pair.second;
        }
        return true;
    }
};

} // namespace core
