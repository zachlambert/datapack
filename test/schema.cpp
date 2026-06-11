#include <datapack/binary.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/json.hpp>
#include <datapack/random.hpp>
#include <datapack/schema/schema.hpp>
#include <datapack/std/string.hpp>
#include <datapack/std/variant.hpp>
#include <datapack/std/vector.hpp>
#include <gtest/gtest.h>

TEST(Schema, Iterator) {
  using namespace dpack;
  {
    Schema schema = Schema::from_tokens({});
    EXPECT_EQ(schema.begin(), schema.end());
  }
  {
    // clang-format off
    Schema schema = Schema::from_tokens({
      token::ObjectBegin(),
        token::ObjectNext(),
          token::Number::F64(),
        token::ObjectEnd()
    });
    // clang-format on
    EXPECT_EQ(schema.begin().skip(), schema.end());

    auto child = schema.begin().next().next();
    auto last = child;
    EXPECT_EQ(child.skip(), last.next());
  }
  {
    // clang-format off
    Schema schema = Schema::from_tokens({
      token::ObjectBegin(),
        token::ObjectNext(),
          token::TupleBegin(),
            token::TupleNext(),
              token::String(),
            token::TupleNext(),
              token::String(),
          token::TupleEnd(),
        token::ObjectNext(),
          token::Optional(),
        token::ObjectNext(),
          token::ObjectBegin(),
          token::ObjectEnd(),
        token::ObjectNext(),
          token::Number::F32(),
      token::ObjectEnd() //
    });
    // clang-format on
    EXPECT_EQ(schema.begin().skip(), schema.end());

    auto first_child = schema.begin().next().next();
    auto last = first_child;
    for (std::size_t i = 0; i < 5; i++) {
      last = last.next();
    }
    EXPECT_EQ(first_child.skip(), last.next());
  }
}

TEST(Schema, SchemaMake) {
  using namespace dpack;

  Schema schema = Schema::make<Entity>();

  // clang-format off
  auto expected = Schema::from_tokens({
    token::ObjectBegin(),
      token::ObjectNext("index"),
        token::Number::I32(),
      token::ObjectNext("name"),
        token::String(),
      token::ObjectNext("enabled"),
        token::Boolean(),
      token::ObjectNext("pose"),
        token::ObjectBegin(),
          token::ObjectNext("x"),
            token::Number::F64(),
          token::ObjectNext("y"),
            token::Number::F64(),
          token::ObjectNext("angle"),
            token::Number::F64(),
          token::ObjectEnd(),
      token::ObjectNext("physics"),
        token::Enumerate({"dynamic", "kinematic", "static"}),
      token::ObjectNext("hitbox"),
        token::Optional(),
          token::VariantBegin({"circle", "rect"}),
            token::VariantNext(0),
              token::ObjectBegin(),
                token::ObjectNext("radius"),
                  token::Number::F64(),
              token::ObjectEnd(),
            token::VariantNext(1),
              token::ObjectBegin(),
                token::ObjectNext("width"),
                  token::Number::F64(),
                token::ObjectNext("height"),
                  token::Number::F64(),
              token::ObjectEnd(),
            token::VariantEnd(),
      token::ObjectNext("sprite"),
        token::ObjectBegin(),
          token::ObjectNext("width"),
            token::Number::U64(),
          token::ObjectNext("height"),
            token::Number::U64(),
          token::ObjectNext("data"),
            token::Binary(),
          token::ObjectEnd(),
      token::ObjectNext("items"),
        token::List(),
          token::ObjectBegin(),
            token::ObjectNext("count"),
              token::Number::U64(),
            token::ObjectNext("name"),
              token::String(),
            token::ObjectEnd(),
      token::ObjectNext("assigned_items"),
        token::TupleBegin(),
          token::TupleNext(),
            token::Number::I32(),
          token::TupleNext(),
            token::Number::I32(),
          token::TupleNext(),
            token::Number::I32(),
          token::TupleEnd(),
      token::ObjectEnd()
  });
  // clang-format on

  EXPECT_EQ(schema, expected);
}

TEST(Schema, SchemaApply) {
  Entity example = Entity::example();

  auto json_direct = dpack::to_json(example);

  auto schema = dpack::Schema::make<Entity>();
  auto bytes = dpack::to_binary(example);
  dpack::Object object;

  schema.apply(dpack::BinaryReader(bytes), dpack::ObjectWriter(object));
  auto json_via_schema = dpack::dump_json(object);

  EXPECT_EQ(json_direct, json_via_schema);
}

struct WithLimit {
  double number;
  DPACK_CLASS_INLINE_CUSTOM({
    packer.hint(dpack::HintRange(0.0, 1.0));
    packer.description("Number in the range [0, 1]");
    packer.value(number);
  })
};

TEST(Schema, SchemaWithHints) {
  auto schema = dpack::Schema::make<WithLimit>();
  auto iter = schema.begin();

  static_assert(dpack::serializable<dpack::Schema>);

  auto hint = iter.hint();
  iter = iter.next();
  ASSERT_TRUE(hint);
  auto hint_range = std::get_if<dpack::HintRange>(hint);
  ASSERT_TRUE(hint_range);
  EXPECT_EQ(hint_range->lower, 0.0);
  EXPECT_EQ(hint_range->upper, 1.0);

  auto description = iter.description();
  iter = iter.next();
  ASSERT_TRUE(description);
  EXPECT_EQ(*description, "Number in the range [0, 1]");

  auto number = iter.number();
  iter = iter.next();
  ASSERT_TRUE(number);

  EXPECT_EQ(iter, schema.end());
}
