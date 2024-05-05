#include "datapack/random.h"

namespace datapack {

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

bool RandomReader::variant_begin(const char* label) {
    // Note: Can't give a meaningful result here without the full schema
    return random() % 2;
}

void RandomReader::variant_end() {

}

void RandomReader::object_begin() {

}

void RandomReader::object_end() {

}

void RandomReader::object_next(const char* key) {

}


} // namespace datapack
