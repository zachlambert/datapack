#include "datapack/random.h"
#include "datapack/binary.h"
#include "datapack/debug.h"
#include <time.h>


using namespace datapack;

struct Foo: public WriteableObject, ReadableObject {
    double x;
    std::optional<double> y;

    Foo():
        x(0)
    {}

    void write(Writer& writer) const override {
        writer.object_next("x");
        writer.value(x);
        writer.object_next("y");
        writer.value(y);
    }

    void read(Reader& reader) override {
        reader.object_next("x");
        reader.value(x);
        reader.object_next("y");
        reader.value(y);
    }
};

int main() {
    srand(time(nullptr));

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
