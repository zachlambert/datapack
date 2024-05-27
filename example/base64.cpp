#include <datapack/encode/base64.hpp>
#include <iostream>

std::vector<std::uint8_t> random_data(std::size_t length) {
    std::vector<std::uint8_t> data(length);
    for (std::size_t i = 0; i < length; i++) {
        data[i] = std::uint8_t(rand() % 256);
    }
    return data;
}

bool compare(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

int main() {
    auto in = random_data(16);
    auto text = datapack::base64_encode(in);
    auto out = datapack::base64_decode(text);

    std::cout << "In:\n";
    for (std::size_t i = 0; i < in.size(); i++) {
        printf("0x%02x ", in[i]);
        if ((i+1)%8 == 0) {
            printf("\n");
        }
    }
    if (in.size() % 8 != 0) printf("\n");

    std::cout << "Encoded:\n" << text << std::endl;

    std::cout << "Out:\n";
    for (std::size_t i = 0; i < out.size(); i++) {
        printf("0x%02x ", out[i]);
        if ((i+1)%8 == 0) {
            printf("\n");
        }
    }
    if (out.size() % 8 != 0) printf("\n");

    std::cout << "EQUAL ? " << (compare(in, out) ? "yes" : "no") << std::endl;
    return 0;
}
