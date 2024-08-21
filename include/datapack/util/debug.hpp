#pragma once

#include "datapack/writer.hpp"
#include <iostream>
#include <vector>


namespace datapack {

class DebugWriter: public Writer {
public:
    DebugWriter(std::ostream& os);

    void primitive(Primitive primitive, const void* value) override;
    void string(const char* value) override;
    void enumerate(int value, const std::span<const char*>& labels) override;

    void optional_begin(bool has_value) override;
    void optional_end() override;

    void variant_begin(const char* label, const std::span<const char*>& labels) override;
    void variant_end() override;

    void binary_data(const std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override;

    void tuple_begin(std::size_t size) override;
    void tuple_end(std::size_t size) override;
    void tuple_next() override;

    void list_begin(bool is_trivial) override;
    void list_end() override;
    void list_next() override;

private:
    void indent();

    std::ostream& os;
    int depth;
};

template <writeable T>
class debug {
public:
    debug(const T& value):
        value(value)
    {}
    std::ostream& operator()(std::ostream& os) const {
        DebugWriter(os).value(value);
        return os;
    }
private:
    const T& value;
};

} // namespace datapack

template <datapack::writeable T>
std::ostream& operator<<(std::ostream& os, datapack::debug<T> manipulator) {
    return manipulator(os);
}
