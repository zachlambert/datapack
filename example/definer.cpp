#include "example.hpp"
#include <iostream>

int main() {
    datapack::Definer definer;
    definer.value(Entity());
    std::cout << definer.tokens();
    return 0;
}
