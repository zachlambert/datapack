#pragma once
#ifndef EMBEDDED

#include <datapack/reader.hpp>
#include <vector>


namespace datapack {

class RandomReader: public Reader {
public:
    RandomReader();

    void value_i32(std::int32_t& value) override;
    void value_i64(std::int64_t& value) override;
    void value_u32(std::uint32_t& value) override;
    void value_u64(std::uint64_t& value) override;

    void value_f32(float& value) override;
    void value_f64(double& value) override;

    const char* value_string(const char* current_value) override;
    void value_bool(bool& value) override;

    int enumerate(const std::span<const char*>& labels) override;
    bool optional_begin(bool current_has_value) override;
    void optional_end() override;
    void variant_begin(const std::span<const char*>& labels) override;
    bool variant_match(const char* label) override;
    void variant_end() override;

    std::tuple<const std::uint8_t*, std::size_t> binary_data(std::size_t length, std::size_t stride) override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override;

    void tuple_begin(std::size_t size) override;
    void tuple_end(std::size_t size) override;
    void tuple_next() override;

    void list_begin(bool is_trivial) override;
    void list_end() override;
    ListNext list_next(bool has_next) override;

private:
    int list_counter;
    const char* next_variant_label;
    std::vector<std::uint8_t> data_temp;
    std::string string_temp;
};

template <readable T>
T random() {
    T result;
    RandomReader().value(result);
    return result;
}

} // namespace datapack
#endif
