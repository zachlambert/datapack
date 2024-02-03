#include <datapack/writer.hpp>
#include <datapack/yaml.hpp>
#include <iostream>


struct Foo: public datapack::Writeable {
    int a;
    int b;
    void write(datapack::Writer& writer) const override {
        writer.object_begin();
        writer.object_element("a");
        writer.value(a);
        writer.object_element("b");
        writer.value(b);
        writer.object_end();
    }
};

struct Bar {
    int x;
    int y;
};

void write(datapack::Writer& writer, const Bar& value) {
    writer.object_begin();
    writer.object_element("x");
    writer.value(value.x);
    writer.object_element("y");
    writer.value(value.y);
    writer.object_end();
}

struct Both: public datapack::Writeable {
    Foo foo;
    Bar bar;

    void write(datapack::Writer& writer) const override {
        writer.object_begin();
        writer.object_element("foo");
        writer.value(foo);
        writer.object_element("bar");
        writer.value(bar);
        writer.object_end();
    }
};

int main() {
    Both value;
    value.foo.a = 5;
    value.foo.b = 10;
    value.bar.x = 15;
    value.bar.y = 20;
    {
        datapack::YamlWriter writer;
        writer.value(value);
        std::cout << writer.result();
    }

    std::vector<Both> values;
    for (std::size_t i = 0; i < 4; i++) {
        values.push_back(value);
    }
    {
        datapack::YamlWriter writer;
        writer.value(values);
        std::cout << writer.result();
    }

    return 0;
}

