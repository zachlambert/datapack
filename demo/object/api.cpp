#include <datapack/object.hpp>
#include <iostream>

int main() {
    using namespace datapack;

    Object object;
    object["a"] = 1.0;
    object["a"] = 2.0;
    object["b"] = "hello";
    object["c"]["first"] = "first";
    object["c"]["second"] = "second";
    object["d"].push_back(100);

    std::cout << object << std::endl;
}
