#include "datapack/format/binary_writer.hpp"


namespace datapack {

template <bool Dynamic>
void BinaryWriter_<Dynamic>::integer(IntType type, const void* value) {
    switch (type) {
        case IntType::I32:
            value_number(*(std::int32_t*)value);
            break;
        case IntType::I64:
            value_number(*(std::int64_t*)value);
            break;
        case IntType::U32:
            value_number(*(std::uint32_t*)value);
            break;
        case IntType::U64:
            value_number(*(std::uint64_t*)value);
            break;
        case IntType::U8:
            value_number(*(std::uint8_t*)value);
            break;
    }
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::floating(FloatType type, const void* value) {
    switch (type) {
        case FloatType::F32:
            value_number(*(float*)value);
            break;
        case FloatType::F64:
            value_number(*(double*)value);
            break;
    }
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::boolean(bool value) {
    value_bool(value);
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::string(const char* value) {
    std::size_t size = std::strlen(value) + 1;
    if (!resize(pos + size)) {
        return;
    }
    strncpy((char*)&data[pos], value, size);
    pos += size;
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::enumerate(int value, const char* label) {
    value_number(value);
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::optional_begin(bool has_value) {
    value_bool(has_value);
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::variant_begin(int value, const char* label) {
    value_number(value);
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::binary(
    const std::uint8_t* input_data,
    std::size_t length,
    std::size_t stride,
    bool fixed_length)
{
    std::size_t size = length * stride;
    if (!fixed_length) {
        value_number(std::uint64_t(length));
    }
    if (!resize(pos + size)) {
        return;
    }
    std::memcpy(&data[pos], input_data, size);
    pos += size;
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::object_begin(std::size_t size) {
    if (size == 0){
        return;
    }
    if (binary_depth == 0) {
        binary_start = pos;
    }
    pad(size);
    binary_depth++;
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::object_end(std::size_t size) {
    if (size == 0) {
        return;
    }
    pad(size);
    binary_depth--;
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::list_begin(bool is_trivial) {
    if (binary_depth != 0) {
        assert(false);
        return;
    }
    if (!is_trivial) {
        return;
    }

    trivial_list_length = 0;
    value_number(std::uint64_t(0)); // Placeholder

    binary_depth++;
    binary_start = pos;
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::list_end() {
    if (binary_depth == 0) {
        value_bool(false);
        return;
    }

    *(std::uint64_t*)&data[binary_start - sizeof(std::uint64_t)] = trivial_list_length;
    binary_depth--;
    assert(binary_depth == 0);
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::list_next() {
    if (binary_depth == 0) {
        value_bool(true);
        return;
    }
    trivial_list_length++;
}

template <bool Dynamic>
bool BinaryWriter_<Dynamic>::pad(std::size_t size) {
    if ((pos-binary_start) % size != 0) {
        pos += (size - (pos-binary_start) % size);
        return resize(pos);
    }
    return true;
}

template <bool Dynamic>
bool BinaryWriter_<Dynamic>::resize(std::size_t new_size) {
    if constexpr(Dynamic) {
        data.resize(new_size);
        return true;
    }
    if constexpr(!Dynamic) {
        return data.resize(new_size);
    }
}

template <bool Dynamic>
template <typename T>
void BinaryWriter_<Dynamic>::value_number(T value) {
    if (binary_depth > 0) {
        if (!pad(sizeof(T))) {
            return;
        }
    }
    if (!resize(pos + sizeof(T))) {
        return;
    }

    *((T*)&data[pos]) = value;
    pos += sizeof(T);
}

template <bool Dynamic>
void BinaryWriter_<Dynamic>::value_bool(bool value) {
    if (!resize(pos + 1)) {
        return;
    }
    data[pos] = (value ? 0x01 : 0x00);
    pos++;
}

template class BinaryWriter_<false>;
template class BinaryWriter_<true>;

} // namespace datapack
