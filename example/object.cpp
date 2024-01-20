#include <datapack/object.h>
#include <datapack/yaml.h>
#include <iostream>

int main()
{
    datapack::Object value;
    value.emplace("a", 1.2);
    value.emplace("b", datapack::Value(123));

    std::cout << datapack::YamlWriter()
        .value(value)
        .result();

    return 0;
}
