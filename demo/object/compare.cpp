#include <datapack/object.hpp>
#include <iostream>

int main() {
    using namespace datapack;

    Object a = Object(Object::map_t());
    Object b = Object(Object::map_t());

    a.insert("x", 1.0);
    a.insert("y", 2.0);
    b.insert("y", 2.0);
    b.insert("x", 1.0);

    std::cout << "Object A: " << a << std::endl;;
    std::cout << "Object B: " << b << std::endl;;
    std::cout << "Equal ? : " << (compare(a, b) ? "yes" : "no") << std::endl;

    return 0;
}
