#include <datapack/object.h>
#include <datapack/yaml.h>
#include <iostream>

int main() {
#if 0
    datapack::Object value;
    value.emplace("a", 1.2);
    value.emplace("b", datapack::Value(123));

    std::cout << datapack::YamlWriter()
        .value(value)
        .result();
#endif
    return 0;
}
