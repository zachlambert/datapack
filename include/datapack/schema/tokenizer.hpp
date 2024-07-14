#pragma once
#ifndef EMBEDDED

#include "datapack/schema/token.hpp"

namespace datapack {

class Tokenizer: public Reader {
public:
    Tokenizer(std::vector<Token>& tokens);

    void value_i32(std::int32_t& value) override;
    void value_i64(std::int64_t& value) override;
    void value_u32(std::uint32_t& value) override;
    void value_u64(std::uint64_t& value) override;
    void value_f32(float& value) override;
    void value_f64(double& value) override;

    const char* value_string() override;
    void value_bool(bool& value) override;

    int enumerate(const std::span<const char*>& labels) override;

    bool optional_begin() override;
    void optional_end() override;

    void variant_begin(const std::span<const char*>& labels) override;

    bool variant_match(const char* label) override;

    void variant_end() override;

    std::tuple<const std::uint8_t*, std::size_t> binary_data() override;
    void trivial_begin(std::size_t size) override;
    void trivial_end(std::size_t size) override;

    void object_begin() override;
    void object_end() override;

    void object_next(const char* key) override;

    void tuple_begin() override;

    void tuple_end() override;

    void tuple_next() override;

    void list_begin() override;
    void list_end() override;
    bool list_next() override;

private:
    std::vector<Token>& tokens;
    bool first_element;
};

} // namespace datapack
#endif
