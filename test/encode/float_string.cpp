#include <datapack/encode/float_string.hpp>
#include <gtest/gtest.h>

TEST(Encode, String) {
  // f32 has 23 bits for fraction
  // Most precise value = 1 + 1 / 2^(23 - 1)
  {
    float in = 1.f + 1.f / float(long(2) << (23 - 1));
    std::string string = datapack::float_to_string(in);
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
    std::string string = datapack::float_to_string(in);
    double out = std::stod(string);
    EXPECT_NE(in, 1.0);
    EXPECT_EQ(in, out);

    double out_underflow = 1.f + 1.f / float(long(2) << (53 - 1));
    EXPECT_EQ(out_underflow, 1.f);
  }
}
