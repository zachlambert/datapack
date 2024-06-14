#include "datapack/util/random.hpp"
#include <cstring>

namespace datapack {

RandomReader::RandomReader():
    container_counter(0),
    next_variant_label(nullptr)
{}

void RandomReader::value_i32(std::int32_t& value) {
    if (auto c = constraint<RangeConstraint>()) {
        value = std::int32_t(c->lower) + rand() % std::int32_t(c->upper - c->lower);
        return;
    }
    value = rand() % 100;
}

void RandomReader::value_i64(std::int64_t& value) {
    if (auto c = constraint<RangeConstraint>()) {
        value = std::int64_t(c->lower) + rand() % std::int64_t(c->upper - c->lower);
        return;
    }
    value = rand() % 100;
}

void RandomReader::value_u32(std::uint32_t& value) {
    if (auto c = constraint<RangeConstraint>()) {
        if (c->lower < 0 || c->upper < 0) {
            error("Invalid range constraint");
        }
        value = std::uint32_t(c->lower) + rand() % std::uint32_t(c->upper - c->lower);
        return;
    }
    value = rand() & 100;
}

void RandomReader::value_u64(std::uint64_t& value) {
    if (auto c = constraint<RangeConstraint>()) {
        if (c->lower < 0 || c->upper < 0) {
            error("Invalid range constraint");
        }
        value = std::uint64_t(c->lower) + rand() % std::uint64_t(c->upper - c->lower);
        return;
    }
    value = rand() & 100;
}


void RandomReader::value_f32(float& value) {
    if (auto c = constraint<RangeConstraint>()) {
        value = c->lower + ((float)rand() / (float)RAND_MAX) * (c->upper - c->lower);
        return;
    }
    value = (float)rand() / (float)RAND_MAX;
}

void RandomReader::value_f64(double& value) {
    if (auto c = constraint<RangeConstraint>()) {
        value = c->lower + ((double)rand() / (double)RAND_MAX) * (c->upper - c->lower);
        return;
    }
    value = (double)rand() / (double)RAND_MAX;
}


void RandomReader::value_string(std::string& value) {
    // Length: [4, 20]
    // Characters ~ { a, ..., z }
    if (auto c = constraint<LengthConstraint>()) {
        value.resize(c->length);
    } else {
        value.resize(4 + rand() % 17);
    }
    for (auto& c: value) {
        c = 'a' + rand() % 26;
    }
}

void RandomReader::value_bool(bool& value) {
    value = rand() % 2 == 1;
}


int RandomReader::enumerate(const std::vector<const char*>& labels) {
    return rand() % labels.size();
}

bool RandomReader::optional_begin() {
    return rand() % 2 == 1;
}

void RandomReader::optional_end() {
    // Do nothing
}

void RandomReader::variant_begin(const std::vector<const char*>& labels) {
    next_variant_label = labels[rand() % labels.size()];
}

bool RandomReader::variant_match(const char* label) {
    if (std::strcmp(label, next_variant_label) == 0) {
        return true;
    }
    return false;
}

void RandomReader::variant_end() {

}

std::tuple<const std::uint8_t*, std::size_t> RandomReader::binary_data() {
    std::size_t size = rand() % 256;
    data_temp.resize(size);
    for (std::size_t i = 0; i < size; i++) {
        data_temp[i] = rand() % 256;
    }
    return std::make_tuple(data_temp.data(), data_temp.size());
}

void RandomReader::object_begin() {
    // Do nothing
}

void RandomReader::object_end() {
    // Do nothing
}

void RandomReader::object_next(const char* key) {
    // Do nothing
}


void RandomReader::tuple_begin() {
    // Do nothing
}

void RandomReader::tuple_end() {
    // Do nothing
}

void RandomReader::tuple_next() {
    // Do nothing
}


void RandomReader::map_begin() {
    container_counter = rand() % 10;
}

void RandomReader::map_end() {

}

bool RandomReader::map_next(std::string& key) {
    if (container_counter == 0) {
        return false;
    }
    container_counter--;
    key.resize(1 + rand() % 10);
    for (auto& c: key) {
        c = 'a' + rand() % 26;
    }
    return true;
}


void RandomReader::list_begin(bool is_array) {
    if (auto c = constraint<LengthConstraint>()) {
        container_counter = c->length;
        return;
    }
    container_counter = rand() % 10;
}

void RandomReader::list_end() {

}

bool RandomReader::list_next() {
    if (container_counter == 0) {
        return false;
    }
    container_counter--;
    return true;
}


} // namespace datapack
