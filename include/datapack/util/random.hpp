#pragma once

#include <datapack/editor.hpp>
#include <vector>
#include <stack>


namespace datapack {

class RandomReader: public Editor {
public:
    RandomReader();

    void primitive(Primitive primitive, void* value) override;
    const char* string(const char* value) override;
    int enumerate(int value, const std::span<const char*>& labels) override;

    bool optional_begin(bool current_has_value) override;
    void optional_end() override;

    int variant_begin(int value, const std::span<const char*>& labels) override;
    void variant_end() override {}

    void binary_data(
        std::uint8_t* data,
        std::size_t length,
        std::size_t stride,
        bool fixed_length) override;

    void object_begin(std::size_t) override {}
    void object_end(std::size_t) override {}
    void object_next(const char* key) override {}

    void tuple_begin(std::size_t) override {}
    void tuple_end(std::size_t) override {}
    void tuple_next() override {}

    void list_begin(bool) override {}
    void list_next() override {}
    ContainerAction list_end() override { return ContainerAction::None; }

private:
    std::string string_temp;
};

template <readable T>
T random() {
    T result;
    RandomReader().value(result);
    return result;
}

} // namespace datapack
