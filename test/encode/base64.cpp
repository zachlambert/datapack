#include <gtest/gtest.h>
#include <datapack/encode/base64.hpp>

static std::vector<std::uint8_t> string_to_bytes(const std::string& value) {
    std::vector<std::uint8_t> bytes;
    for (char c: value) {
        bytes.push_back(c);
    }
    return bytes;
}

TEST(Encode, Base64) {
    // Examples taken from:
    // https://en.wikipedia.org/wiki/Base64

    std::vector<std::vector<std::uint8_t>> examples_bytes = {
        string_to_bytes("light work."),
        string_to_bytes("light work"),
        string_to_bytes("light wor"),
        string_to_bytes("light wo"),
        string_to_bytes("light w")
    };
    std::vector<std::string> examples_text = {
        "bGlnaHQgd29yay4=",
        "bGlnaHQgd29yaw==",
        "bGlnaHQgd29y",
        "bGlnaHQgd28=",
        "bGlnaHQgdw=="
    };

    for (std::size_t i = 0; i < examples_bytes.size(); i++) {
        const auto& bytes = examples_bytes[i];
        const auto& text = examples_text[i];

        ASSERT_EQ(datapack::base64_encode(bytes), text);
        ASSERT_EQ(datapack::base64_decode(text), bytes);
    }
}
