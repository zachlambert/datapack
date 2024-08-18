#pragma once

#include "datapack/reader.hpp"
#include "datapack/writer.hpp"
#include <optional>
#include <micro_types/optional.hpp>


namespace datapack {

template <readable T>
void pack(std::optional<T>& value, Reader& packer) {
    if (packer.optional_begin()) {
        value.emplace();
        packer.value(value.value());
        packer.optional_end();
    } else {
        value = std::nullopt;
    }
}

template <writeable T>
void pack(const std::optional<T>& value, Writer& packer) {
    packer.optional_begin(value.has_value());
    if (value.has_value()) {
        packer.value(value.value());
        packer.optional_end();
    }
}

template <readable T>
void pack(mct::optional<T>& value, Reader& packer) {
    if (packer.optional_begin()) {
        value.emplace();
        packer.value(value.value());
        packer.optional_end();
    } else {
        value = std::nullopt;
    }
}

template <writeable T>
void pack(const mct::optional<T>& value, Writer& packer) {
    packer.optional_begin(value.has_value());
    if (value.has_value()) {
        packer.value(value.value());
        packer.optional_end();
    }
}

} // namespace datapack
