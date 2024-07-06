#pragma once
#ifndef EMBEDDED

#include <vector>
#include <type_traits>
#include <string>
#include <optional>
#include <variant>
#include <stdexcept>
#include <memory>
#include <stack>


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

namespace _object {
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

} // namespace _object


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

    Object_ insert(const std::string& key, const value_t& value) const;
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

    Object_ create_node(const Node& node) const;

    state_t state;
    int index;

    template <bool OtherConst>
    friend class Object_;
};

using Object = Object_<false>;
using ConstObject = Object_<true>;

bool compare(const ConstObject& lhs, const ConstObject& rhs, double float_threshold=1e-12);

} // namespace datapack

std::ostream& operator<<(std::ostream& os, datapack::ConstObject object);
inline std::ostream& operator<<(std::ostream& os, datapack::Object object) {
    os << static_cast<datapack::ConstObject>(object);
    return os;
}

#endif
