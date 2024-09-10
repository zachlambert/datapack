#include "datapack/util/random.hpp"
#include <cstring>

namespace datapack {

RandomReader::RandomReader()
{}

void RandomReader::primitive(Primitive primitive, void* value) {
    if (primitive == Primitive::BOOL) {
        *(bool*)value = rand() % 2;
        return;
    }

    if (auto c = constraint<RangeConstraint>()) {
        double value_double = c->lower + (double)rand() * (c->upper - c->lower) / RAND_MAX;
        switch (primitive) {
            case Primitive::I32:
                *(std::int32_t*)value = value_double;
                break;
            case Primitive::I64:
                *(std::int64_t*)value = value_double;
                break;
            case Primitive::U32:
                *(std::uint32_t*)value = value_double;
                break;
            case Primitive::U64:
                *(std::uint64_t*)value = value_double;
                break;
            case Primitive::F32:
                *(float*)value = value_double;
                break;
            case Primitive::F64:
                *(double*)value = value_double;
                break;
            case Primitive::U8:
                *(std::uint8_t*)value = value_double;
                break;
            case Primitive::BOOL:
                break;
        }
        return;
    }

    switch (primitive) {
        case Primitive::I32:
            *(std::int32_t*)value = rand() % 100;
            break;
        case Primitive::I64:
            *(std::int64_t*)value = rand() % 100;
            break;
        case Primitive::U32:
            *(std::uint32_t*)value = rand() % 100;
            break;
        case Primitive::U64:
            *(std::uint64_t*)value = rand() % 100;
            break;
        case Primitive::F32:
            *(float*)value = (float)rand() / (float)RAND_MAX;
            break;
        case Primitive::F64:
            *(double*)value = (double)rand() / (double)RAND_MAX;
            break;
        case Primitive::U8:
            *(std::uint8_t*)value = (rand() % 256);
            break;
        case Primitive::BOOL:
            break;
    }
}

const char* RandomReader::string(const char*) {
    // Length: [4, 20]
    // Characters ~ { a, ..., z }
    if (auto c = constraint<LengthConstraint>()) {
        string_temp.resize(c->length);
    } else {
        string_temp.resize(4 + rand() % 17);
    }
    for (auto& c: string_temp) {
        c = 'a' + rand() % 26;
    }
    return string_temp.c_str();
}

int RandomReader::enumerate(int value, const std::span<const char*>& labels) {
    return rand() % labels.size();
}

bool RandomReader::optional_begin(bool) {
    return rand() % 2 == 1;
}

void RandomReader::optional_end() {
    // Do nothing
}

int RandomReader::variant_begin(int value, const std::span<const char*>& labels) {
    return rand() % labels.size();
}

void RandomReader::binary_data(
    std::uint8_t* data,
    std::size_t length,
    std::size_t stride,
    bool fixed_length)
{
    std::size_t size = length * stride;
    for (std::size_t i = 0; i < size; i++) {
        data[i] = rand() % 256;
    }
}

} // namespace datapack
