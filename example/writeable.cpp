#include <datapack/writer.h>
#include <datapack/yaml.h>
#include <iostream>


struct Foo: public datapack::Writeable {
    int a;
    int b;
    void write(datapack::Writer& writer) const override {
        writer
            .start_object()
            .key("a").value(a)
            .key("b").value(b)
            .end_object();
    }
};

struct Bar {
    int x;
    int y;
};

void write(datapack::Writer& writer, const Bar& value) {
    writer.start_object();
    writer.key("x").value(value.x);
    writer.key("y").value(value.y);
    writer.end_object();
}

struct Both: public datapack::Writeable {
    Foo foo;
    Bar bar;

    void write(datapack::Writer& writer) const override {
        writer.
            start_object()
                .key("foo").value(foo)
                .key("bar").value(bar)
            .end_object();
    }
};

int main() {
    Both value;
    value.foo.a = 5;
    value.foo.b = 10;
    value.bar.x = 15;
    value.bar.y = 20;
    std::cout << datapack::YamlWriter().value(value).result();

    std::vector<Both> values;
    for (std::size_t i = 0; i < 4; i++) {
        values.push_back(value);
    }
    std::cout << datapack::YamlWriter().value(values).result();

    return 0;
}

