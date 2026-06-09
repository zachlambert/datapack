#include <datapack/debug.hpp>
#include <datapack/json.hpp>
#include <datapack/polymorphic.hpp>
#include <datapack/schema/schema.hpp>
#include <gtest/gtest.h>

class Fruit {
public:
  virtual std::string describe() const = 0;
};

class Apple : public Fruit {
public:
  Apple() : color("red") {}
  Apple(std::string color) : color(color) {}

  std::string describe() const override {
    return "apple has the color " + color;
  }

  DATAPACK_CLASS_INLINE(color);

private:
  std::string color;
};

class Banana : public Fruit {
public:
  Banana() : length(0) {}
  Banana(double length) : length(length) {}

  std::string describe() const override {
    return "banana has length " + std::to_string(length);
  }

  DATAPACK_CLASS_INLINE(length);

private:
  int length;
};

class Pear : public Fruit {
public:
  std::string describe() const override {
    return "pear";
  }

  DATAPACK_CLASS_INLINE();
};

template <>
void datapack::register_polymorphic_defaults<Fruit>() {
  register_polymorphic<Fruit, Apple>("apple");
  register_polymorphic<Fruit, Banana>("banana");
}

TEST(Poly, WriteRead) {
  using namespace datapack;

  // Support "static" implementations (defined in register_polymorphic_defaults)
  // as well as those defined at runtime
  register_polymorphic<Fruit, Pear>("pear");

  std::unique_ptr<Fruit> apple = std::make_unique<Apple>("green");
  std::unique_ptr<Fruit> banana = std::make_unique<Banana>(15);
  std::unique_ptr<Fruit> pear = std::make_unique<Pear>();

  EXPECT_EQ(to_json(apple), R"""({
    "type": "apple",
    "value_apple": {
        "color": "green"
    }
})""");

  EXPECT_EQ(to_json(banana), R"""({
    "type": "banana",
    "value_banana": {
        "length": 15
    }
})""");

  EXPECT_EQ(to_json(pear), R"""({
    "type": "pear",
    "value_pear": {}
})""");

  auto fruit = from_json<std::unique_ptr<Fruit>>(R"""({
    "type": "banana",
    "value_banana": {
        "length": 20
    }
})""");
  EXPECT_TRUE(dynamic_cast<const Banana*>(fruit.get()));
  EXPECT_EQ(fruit->describe(), "banana has length 20");

  // clang-format off
  Schema expected_schema = Schema::from_tokens({
    token::VariantBegin({"apple", "banana", "pear"}),
      token::VariantNext(0),
        token::ObjectBegin(),
          token::ObjectNext("color"),
            token::String(),
          token::ObjectEnd(),
      token::VariantNext(1),
        token::ObjectBegin(),
          token::ObjectNext("length"),
            token::Number(NumberType::F64),
          token::ObjectEnd(),
      token::VariantNext(2),
        token::ObjectBegin(),
          token::ObjectEnd(),
    token::VariantEnd()
  });
  // clang-format on

  auto schema1 = Schema::make<std::unique_ptr<Fruit>>();
  auto schema2 = Schema::make<std::shared_ptr<Fruit>>();

  EXPECT_EQ(expected_schema, schema1);
  EXPECT_EQ(expected_schema, schema2);
}
