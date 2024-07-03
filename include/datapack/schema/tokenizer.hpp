#pragma once

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

    void value_string(std::string& value) override;
    void value_bool(bool& value) override;

    int enumerate(const std::vector<const char*>& labels) override;

    bool optional_begin() override;
    void optional_end() override;

    void variant_begin(const std::vector<const char*>& labels) override;

    bool variant_match(const char* label) override;

    void variant_end() override;

    std::tuple<const std::uint8_t*, std::size_t> binary_data() override;

    void object_begin() override;
    void object_end() override;

    void object_next(const char* key) override;

    void tuple_begin() override;

    void tuple_end() override;

    void tuple_next() override;

    void map_begin() override;
    void map_end() override;
    bool map_next(std::string& key) override;

    void list_begin(bool is_array) override;
    void list_end() override;
    bool list_next() override;

private:
    std::vector<Token>& tokens;
    bool first_element;
};

} // namespace datapack
