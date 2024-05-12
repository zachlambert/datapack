#include "datapack/util/random.hpp"

namespace datapack {

RandomReader::RandomReader():
    container_counter(0),
    next_binary_size(0)
{}

void RandomReader::value_i32(std::int32_t& value) {
    value = rand() % 100;
}

void RandomReader::value_i64(std::int64_t& value) {
    value = rand() % 1000;
}

void RandomReader::value_u32(std::uint32_t& value) {
    value = rand() & 100;
}

void RandomReader::value_u64(std::uint64_t& value) {
    value = rand() & 1000;
}


void RandomReader::value_f32(float& value) {
    value = (float)rand() / (float)RAND_MAX;
}

void RandomReader::value_f64(double& value) {
    value = (double)rand() / (double)RAND_MAX;
}


void RandomReader::value_string(std::string& value) {
    // Length: [4, 20]
    // Characters ~ { a, ..., z }
    value.resize(4 + rand() % 17);
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

bool RandomReader::optional() {
    return rand() % 2 == 1;
}

const char* RandomReader::variant(const std::vector<const char*>& labels) {
    return labels[rand() % labels.size()];
}


std::size_t RandomReader::binary_size(std::size_t expected_size) {
    if (expected_size == 0) {
        next_binary_size = rand() % 256;
    } else {
        next_binary_size = expected_size;
    }
    return expected_size;
}

void RandomReader::binary_data(std::uint8_t* data) {
    for (std::size_t i = 0; i < next_binary_size; i++) {
        data[i] = rand() % 256;
    }
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


void RandomReader::list_begin() {
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
