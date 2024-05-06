#include "datapack/random.hpp"
#include "datapack/binary.hpp"
#include "datapack/debug.hpp"
#include "datapack/variant.hpp"
#include <chrono>


using namespace datapack;

struct Apple: public WriteableObject, ReadableObject {
    int a;
    Apple():
        a(0)
    {}
    void write(Writer& writer) const override {
        writer.object_next("a");
        writer.value(a);
    }
    void read(Reader& reader) override {
        reader.object_next("a");
        reader.value(a);
    }
};

struct Banana: public WriteableObject, ReadableObject {
    double b;
    Banana():
        b(0)
    {}
    void write(Writer& writer) const override {
        writer.object_next("b");
        writer.value(b);
    }
    void read(Reader& reader) override {
        reader.object_next("b");
        reader.value(b);
    }
};

using Fruit = std::variant<Apple, Banana>;

template <>
const char* datapack::variant_label<Fruit, Apple>() { return "apple"; }
template <>
const char* datapack::variant_label<Fruit, Banana>() { return "banana"; }

template <>
const std::vector<std::string>& datapack::variant_labels<Fruit>() {
    static const std::vector<std::string> data = { "apple", "banana" };
    return data;
}

struct Foo: public WriteableObject, ReadableObject {
    double x;
    std::optional<double> y;
    Fruit fruit;

    Foo():
        x(0),
        fruit(Apple())
    {}

    void write(Writer& writer) const override {
        writer.object_next("x");
        writer.value(x);
        writer.object_next("y");
        writer.value(y);
        writer.object_next("fruit");
        writer.value(fruit);
    }

    void read(Reader& reader) override {
        reader.object_next("x");
        reader.value(x);
        reader.object_next("y");
        reader.value(y);
        reader.object_next("fruit");
        reader.value(fruit);
    }
};

int main() {
    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count() % std::numeric_limits<unsigned int>::max());

    Foo in, out;

    RandomReader random_reader;
    in.read(random_reader);

    std::vector<std::uint8_t> data;

    BinaryWriter writer(data);
    writer.value(in);

    BinaryReader reader(data);
    out.read(reader);

    DebugWriter debug_writer(std::cout);
    std::cout << "In:\n";
    debug_writer.value(in);
    std::cout << "Out:\n";
    debug_writer.value(out);

    return 0;
}
