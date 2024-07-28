#include <datapack/object.hpp>
#include <iostream>

int main() {
    using namespace datapack;

    Object object;
    object["a"] = 1.0;
    object["a"] = 2.0;
    object["b"] = "hello";

    std::cout << object << std::endl;
}
