#pragma once

#include "datapack/datapack.hpp"
#include <micro_types/string.hpp>


namespace datapack {

inline void read(Reader& reader, std::int32_t& value) {
    reader.value_i32(value);
}

inline void read(Reader& reader, std::int64_t& value) {
    reader.value_i64(value);
}

inline void read(Reader& reader, std::uint32_t& value) {
    reader.value_u32(value);
}

inline void read(Reader& reader, std::uint64_t& value) {
    reader.value_u64(value);
}

inline void read(Reader& reader, float& value) {
    reader.value_f32(value);
}

inline void read(Reader& reader, double& value) {
    reader.value_f64(value);
}

inline void read(Reader& reader, std::string& value) {
    if (const char* value_cstr = reader.value_string(value.c_str())) {
        value = value_cstr;
    } else {
        value.clear();
    }
}

inline void read(Reader& reader, mct::string& value) {
    if (const char* value_cstr = reader.value_string(value.c_str())) {
        value = value_cstr;
    } else {
        value.clear();
    }
}

inline void read(Reader& reader, bool& value) {
    reader.value_bool(value);
}

inline void write(Writer& writer, std::int32_t value) {
    writer.value_i32(value);
}

inline void write(Writer& writer, std::int64_t value) {
    writer.value_i64(value);
}

inline void write(Writer& writer, std::uint32_t value) {
    writer.value_u32(value);
}

inline void write(Writer& writer, std::uint64_t value) {
    writer.value_u64(value);
}

inline void write(Writer& writer, float value) {
    writer.value_f32(value);
}

inline void write(Writer& writer, double value) {
    writer.value_f64(value);
}

inline void write(Writer& writer, const std::string& value) {
    writer.value_string(value.c_str());
}

inline void write(Writer& writer, const mct::string& value) {
    writer.value_string(value.c_str());
}

inline void write(Writer& writer, const bool& value) {
    writer.value_bool(value);
}

inline bool validate(const std::int32_t& value, const RangeConstraint& c) {
    return c.validate(value);
}

inline bool validate(const std::int64_t& value, const RangeConstraint& c) {
    return c.validate(value);
}

inline bool validate(const std::uint32_t& value, const RangeConstraint& c) {
    return c.validate(value);
}

inline bool validate(const std::uint64_t& value, const RangeConstraint& c) {
    return c.validate(value);
}

inline bool validate(const float& value, const RangeConstraint& c) {
    return c.validate(value);
}

inline bool validate(const double& value, const RangeConstraint& c) {
    return c.validate(value);
}

} // namespace datapack
