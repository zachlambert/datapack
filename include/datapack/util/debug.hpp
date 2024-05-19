#pragma once

#include "datapack/writer.hpp"
#include <iostream>

namespace datapack {

class DebugWriter: public Writer {
public:
    DebugWriter(std::ostream& os);

    void value_i32(std::int32_t) override;
    void value_i64(std::int64_t) override;
    void value_u32(std::uint32_t) override;
    void value_u64(std::uint64_t) override;

    void value_f32(float) override;
    void value_f64(double) override;

    void value_string(const std::string&) override;
    void value_bool(bool) override;

    void enumerate(int value, const std::vector<const char*>& labels) override;
    void optional(bool has_value) override;
    void variant_begin(const char* label, const std::vector<const char*>& labels) override;
    void variant_end() override;

    void binary(std::size_t size, const std::uint8_t* data) override;

    void object_begin() override;
    void object_end() override;
    void object_next(const char* key) override;

    void tuple_begin() override;
    void tuple_end() override;
    void tuple_next() override;

    void map_begin() override;
    void map_end() override;
    void map_next(const std::string& key) override;

    void list_begin() override;
    void list_end() override;
    void list_next() override;

private:
    void indent();

    std::ostream& os;
    int depth;
};

} // namespace datapack
