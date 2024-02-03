#include <datapack/yaml.hpp>
#include <iostream>

int main() {
    datapack::YamlWriter writer;

    writer.object_begin();
    writer.object_element("a");
    writer.i32(12);
    writer.object_element("b");
    writer.i32(24);
    writer.object_element("c");
    {
        writer.object_begin();
        writer.object_element("foo");
        {
            writer.array_begin();
            writer.array_element();
            writer.f64(1.23);
            writer.array_element();
            {
                writer.object_begin();
                writer.object_element("one");
                writer.string("two");
                writer.object_element("three");
                writer.string("four");
                writer.object_end();
            }
            writer.array_element();
            writer.f64(1.23);
            writer.array_end();
        }
        writer.object_element("bar");
        writer.boolean(true);
        writer.object_end();
    }
    writer.object_end();

    std::cout << writer.result();
    return 0;
}
