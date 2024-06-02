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
void read(datapack::Reader& reader, Point& value) {
    reader.object_begin();
    reader.value("x", value.x);
    reader.value("y", value.y);
    reader.value("z", value.z);
    reader.object_end();
}

void print_points(const std::vector<Point>& points) {
    for (std::size_t i = 0; i < points.size(); i++) {
        const auto& p = points[i];
        printf("%f, %f, %f\n", p.x, p.y, p.z);
    }
}

bool compare(const std::vector<Point>& a, const std::vector<Point>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); i++) {
        if (a[i].x != b[i].x) return false;
        if (a[i].y != b[i].y) return false;
        if (a[i].z != b[i].z) return false;
    }
    return true;
}

int main() {
    std::vector<Point> points;
    for (std::size_t i = 0; i < 3; i++) {
        Point point;
        point.x = double(rand()) / RAND_MAX;
        point.y = double(rand()) / RAND_MAX;
        point.z = double(rand()) / RAND_MAX;
        points.push_back(point);
    }

    auto data = datapack::write_binary(points);

    std::vector<Point> a, b;

    std::cout << "Reading raw binary data:\n";
    datapack::BinaryReader(data, false).value(a);
    print_points(a);

    std::cout << "Reading element-by-element:\n";
    datapack::BinaryReader(data, true).value(b);
    print_points(b);

    std::cout << "Equal ? " << (compare(a, b) ? "yes" : "no") << std::endl;
}
