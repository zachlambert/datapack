#include <datapack/writer.hpp>
#include <datapack/reader.hpp>
#include <datapack/yaml.hpp>
#include <datapack/json.hpp>
#include <iostream>


struct Foo: public datapack::Writeable, public datapack::Readable {
    double a;
    double b;
    void write(datapack::Writer& writer) const override {
        writer.object_begin();
        writer.object_element("a");
        writer.value(a);
        writer.object_element("b");
        writer.value(b);
        writer.object_end();
    }
    void read(datapack::Reader& reader) override {
        reader.object_begin();
        reader.object_element("a");
        a = reader.value_like(a);
        reader.object_element("b");
        b = reader.value_like(b);
        reader.object_end();
    }
};

struct Bar {
    double x;
    double y;
};

void write(datapack::Writer& writer, const Bar& value) {
    writer.object_begin();
    writer.object_element("x");
    writer.value(value.x);
    writer.object_element("y");
    writer.value(value.y);
    writer.object_end();
}

void read(datapack::Reader& reader, Bar& value) {
    reader.object_begin();
    reader.object_element("x");
    value.x = reader.value_like(value.x);
    reader.object_element("y");
    value.y = reader.value_like(value.y);
    reader.object_end();
}

struct Both: public datapack::Writeable, public datapack::Readable {
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
    void read(datapack::Reader& reader) override {
        reader.object_begin();
        reader.object_element("foo");
        foo = reader.value_like(foo);
        reader.object_element("bar");
        bar = reader.value_like(bar);
        reader.object_end();
    }
};

int main() {
    const std::string json = R"({
    "foo": {
        "a": 5,
        "b": 10
    },
    "bar": {
        "x": 15,
        "y": 20
    }
})";

    auto object = datapack::parse(datapack::JsonParser(json));
    datapack::ObjectReader reader(object);

    Both value;
    value.read(reader);

    datapack::YamlWriter writer;
    writer.value(value);
    std::cout << writer.result() << std::endl;

    return 0;
}

