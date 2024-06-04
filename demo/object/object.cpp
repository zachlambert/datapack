#include <datapack/object.hpp>
#include <stack>
#include <iostream>


int main() {
    using namespace datapack;

    Object object = Object(Object::map_t());

    object.insert("a", 1.1);
    object.insert("b", 2.2);

    auto c = object.insert("c", Object::list_t());
    c.append("hello");
    c.append("world");

    object.insert("d", true);
    auto e = object.insert("e", Object::map_t());
    e.insert("foo", "foo");
    e.insert("bar", "bar");
    e.insert("and", std::nullopt);

    std::cout << "object: " << object << std::endl;

    std::cout << "object[\"a\"]: " << object["a"] << std::endl;

    std::cout << "object[\"c\"] (before): " << c << std::endl;

    c[0].set("goodbye");
    std::cout << "object[\"c\"] (after): " << c << std::endl;

    std::cout << "object (after): " << c << std::endl;

    auto e_clone = object["e"].clone();
    e_clone["and"].set("and");
    std::cout << "object[\"e\"] (cloned, edited): " << e_clone << std::endl;

    std::cout << "object (original, unaffected): " << object["e"] << std::endl;

    object["c"].set(3.3);
    object["e"].set(false);
    std::cout << "object (overwrite \"c\" and \"e\"): " << object << std::endl;

    return 0;
}
