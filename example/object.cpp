#include <datapack/object.h>
#include <datapack/yaml.h>
#include <iostream>

int main()
{
    datapack::Object value;
    value.emplace("a", 1.2);
    value.emplace("b", datapack::Value(123));

    std::cout << datapack::YamlWriter()
        .start_object()
        .key("a").value(1.2)
        .end_object()
        .finish();

    return 0;
}
