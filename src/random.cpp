#include "datapack/random.hpp"

namespace datapack {

RandomReader::RandomReader()
{}

void RandomReader::value_f64(double& value) {
    value = (double)rand() / RAND_MAX;
}

void RandomReader::value_i32(int& value) {
    value = rand() % 100;
}

bool RandomReader::optional_begin() {
    return random() % 2 == 0;
}

void RandomReader::optional_end() {

}

void RandomReader::variant_begin(const std::vector<std::string>& types) {
    next_variant = types[random() % types.size()];
}

bool RandomReader::variant_match(const char* label) {
    return next_variant == label;
}

void RandomReader::variant_end() {
    next_variant = "";
}

void RandomReader::object_begin() {

}

void RandomReader::object_end() {

}

void RandomReader::object_next(const char* key) {

}


} // namespace datapack
