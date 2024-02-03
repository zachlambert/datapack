#include <datapack/yaml.hpp>
#include <iostream>

int main() {
    datapack::YamlWriter writer;

    writer.start_object();
    writer.key("a");
    writer.i32(12);
    writer.key("b");
    writer.i32(24);
    writer.key("c");
    {
        writer.start_object();
        writer.key("foo");
        {
            writer.start_array();
            writer.next();
            writer.f64(1.23);
            writer.next();
            {
                writer.start_object();
                writer.key("one");
                writer.string("two");
                writer.key("three");
                writer.string("four");
                writer.end_object();
            }
            writer.next();
            writer.f64(1.23);
            writer.end_array();
        }
        writer.key("bar");
        writer.boolean(true);
        writer.end_object();
    }
    writer.end_object();

    std::cout << writer.result();
    return 0;
}
