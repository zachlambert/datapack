#include <datapack/format/binary.hpp>
#include <datapack/common.hpp>

// Packing structure
// [0-4] x (float)
// [4-8] padding
// [8-16] y (double)
// [16-20] z (float)
// [20-24] padding

// Every datatype within the struct must lie at an address that
// is a multiple of it's size.
// Implications of this for the Point struct:
// 1. There is padding after "x", such that "y" is at an offset of 8
// 2. The struct itself must have a size that is a multiple of 8, such that
//    the offset of 8 is at a correct address
// 3. There is padding after "z" to fill the size up to 24

struct Point {
    float x;
    double y;
    float z;
    Point() {
        // By default, the padding bytes in the struct are filled with garbage
        // This doesn't matter in most use cases, since these bytes are unused,
        // but in this case, comparing the raw binary data, so to avoid a false
        // match from the garbage data, ensure this is all set to zero by default
        memset(this, 0, sizeof(Point));
    }
};
void write(datapack::Writer& writer, const Point& value) {
    writer.object_begin();
    writer.value("x", value.x);
    writer.value("y", value.y);
    writer.value("z", value.z);
    writer.object_end();
}

void print_binary(const std::vector<std::uint8_t>& bytes) {
    for (std::size_t i = 0; i < bytes.size(); i++) {
        printf("%02x ", bytes[i]);
        if ((i + 1) % 8 == 0) {
            std::cout << std::endl;
        }
    }
    if (bytes.size() % 8 != 0) {
        std::cout << std::endl;
    }
}

bool compare(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

int main() {
    Point arr_test[2];
    std::cout << "Point size: " << sizeof(Point) << std::endl;
    std::cout << "Point stride: " << (std::uint8_t*)&arr_test[1] - (std::uint8_t*)&arr_test[0] << std::endl;

    std::vector<Point> points;
    for (std::size_t i = 0; i < 3; i++) {
        Point point;
        point.x = double(rand()) / RAND_MAX;
        point.y = double(rand()) / RAND_MAX;
        point.z = double(rand()) / RAND_MAX;
        points.push_back(point);
    }

    std::vector<std::uint8_t> a, b;
    datapack::BinaryWriter(a, true).value(points);
    datapack::BinaryWriter(b, false).value(points);

    std::cout << "Writing raw binary data:\n";
    print_binary(a);
    std::cout << "Writing element-by-element:\n";
    print_binary(b);

    // Reduce a and b to the data only, they will in that "a" stores the
    // size in bytes, "b" stores the size in elements
    for (std::size_t i = 0; i < 8; i++) {
        a.erase(a.begin());
        b.erase(b.begin());
    }

    std::cout << "Equal ? " << (compare(a, b) ? "yes" : "no") << std::endl;
}
