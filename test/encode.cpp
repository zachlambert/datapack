#include <datapack/encode/base64.hpp>
#include <datapack/encode/floating_string.hpp>
#include <gtest/gtest.h>

static std::vector<std::uint8_t> string_to_bytes(const std::string& value) {
  std::vector<std::uint8_t> bytes;
  for (char c : value) {
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
      string_to_bytes("light w")};
  std::vector<std::string> examples_text =
      {"bGlnaHQgd29yay4=", "bGlnaHQgd29yaw==", "bGlnaHQgd29y", "bGlnaHQgd28=", "bGlnaHQgdw=="};

  for (std::size_t i = 0; i < examples_bytes.size(); i++) {
    const auto& bytes = examples_bytes[i];
    const auto& text = examples_text[i];

    ASSERT_EQ(datapack::base64_encode(bytes), text);
    ASSERT_EQ(datapack::base64_decode(text), bytes);
  }
}

TEST(Encode, String) {
  // f32 has 23 bits for fraction
  // Most precise value = 1 + 1 / 2^(23 - 1)
  {
    float in = 1.f + 1.f / float(long(2) << (23 - 1));
    std::string string = datapack::floating_to_string(in);
    float out = std::stof(string);
    EXPECT_NE(in, 1.f);
    EXPECT_EQ(in, out);

    float in_underflow = 1.f + 1.f / float(long(2) << (24 - 1));
    EXPECT_EQ(in_underflow, 1.f);
  }

  // f32 has 52 bits for fraction
  // Most precise value = 1 + 1 / 2^(52 - 1)
  {
    double in = 1.0 + 1.0 / double(long(2) << (52 - 1));
    std::string string = datapack::floating_to_string(in);
    double out = std::stod(string);
    EXPECT_NE(in, 1.0);
    EXPECT_EQ(in, out);

    double out_underflow = 1.f + 1.f / float(long(2) << (53 - 1));
    EXPECT_EQ(out_underflow, 1.f);
  }
}
