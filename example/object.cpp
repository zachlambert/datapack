#include <datapack/object.hpp>
#include <stack>
#include <iostream>


int main() {
    using namespace datapack;

    Object object;
    object.set_root(object::map_t());

    auto root = object.root();
    root.insert("a", 1.0);
    root.insert("b", 2.0);

    auto c = root.insert("c", object::list_t());
    c.append("hello");
    c.append("world");

    root.insert("d", true);
    auto e = root.insert("e", object::map_t());
    e.insert("foo", "foo");
    e.insert("bar", "bar");
    e.insert("and", std::nullopt);

    std::cout << object;

    return 0;
}
