#include <datapack/object.hpp>
#include <iostream>

int main() {
    using namespace datapack;
    Object a, b;

    a.set_root(object::map_t());
    b.set_root(object::map_t());

    a.root().insert("x", 1.0);
    a.root().insert("y", 2.0);
    b.root().insert("y", 2.0);
    b.root().insert("x", 1.0);

    std::cout << "Object A:\n" << a;
    std::cout << "Object B:\n" << b;
    std::cout << "Equal ?: " << (compare(a, b) ? "true" : "false") << std::endl;

    return 0;
}
