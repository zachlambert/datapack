#include <datapack/object.h>
#include <datapack/yaml.h>

int main()
{
    datapack::Object value;
    value.emplace("a", 1.2);
    return 0;
}
