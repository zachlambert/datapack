#include <datapack/binary.hpp>
#include <datapack/debug.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/json.hpp>
#include <datapack/random.hpp>
#include <datapack/schema/schema.hpp>
#include <datapack/std/string.hpp>
#include <datapack/std/variant.hpp>
#include <datapack/std/vector.hpp>
#include <gtest/gtest.h>

TEST(Schema, Iterator) {
  using namespace datapack;
  {
    Schema schema = Schema::FromTokens({});
    EXPECT_EQ(schema.begin(), schema.end());
  }
  {
    Schema schema = Schema::FromTokens({
        token::ObjectBegin(),
        token::ObjectNext(),
        NumberType::F64,
        token::ObjectEnd() //
    });
    EXPECT_EQ(schema.begin().skip(), schema.end());

    auto child = schema.begin().next().next();
    auto last = child;
    EXPECT_EQ(child.skip(), last.next());
  }
  {
    Schema schema = Schema::FromTokens({
        token::ObjectBegin(),
        token::ObjectNext(),
        token::TupleBegin(),
        token::TupleNext(),
        std::string(),
        token::TupleNext(),
        std::string(),
        token::TupleEnd(),
        token::ObjectNext(),
        token::Optional(),
        token::ObjectNext(),
        token::ObjectBegin(),
        token::ObjectEnd(),
        token::ObjectNext(),
        NumberType::F32,
        token::ObjectEnd() //
    });
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
  using namespace datapack;

  Schema schema = Schema::Make<Entity>();

  std::vector<Token> tokens = {
      token::ObjectBegin(),
      token::ObjectNext("index"),
      NumberType::I32,
      token::ObjectNext("name"),
      std::string(),
      token::ObjectNext("enabled"),
      bool(),
      token::ObjectNext("pose"),
      token::ObjectBegin(),
      token::ObjectNext("x"),
      NumberType::F64,
      token::ObjectNext("y"),
      NumberType::F64,
      token::ObjectNext("angle"),
      NumberType::F64,
      token::ObjectEnd(),
      token::ObjectNext("physics"),
      token::Enumerate({"dynamic", "kinematic", "static"}),
      token::ObjectNext("hitbox"),
      token::Optional(),
      token::VariantBegin({"circle", "rect"}),
      token::VariantNext(0),
      token::ObjectBegin(),
      token::ObjectNext("radius"),
      NumberType::F64,
      token::ObjectEnd(),
      token::VariantNext(1),
      token::ObjectBegin(),
      token::ObjectNext("width"),
      NumberType::F64,
      token::ObjectNext("height"),
      NumberType::F64,
      token::ObjectEnd(),
      token::VariantEnd(),
      token::ObjectNext("sprite"),
      token::ObjectBegin(),
      token::ObjectNext("width"),
      NumberType::U64,
      token::ObjectNext("height"),
      NumberType::U64,
      token::ObjectNext("data"),
      token::Binary(),
      token::ObjectEnd(),
      token::ObjectNext("items"),
      token::List(),
      token::ObjectBegin(),
      token::ObjectNext("count"),
      NumberType::U64,
      token::ObjectNext("name"),
      std::string(),
      token::ObjectEnd(),
      token::ObjectNext("assigned_items"),
      token::TupleBegin(),
      token::TupleNext(),
      NumberType::I32,
      token::TupleNext(),
      NumberType::I32,
      token::TupleNext(),
      NumberType::I32,
      token::TupleEnd(),
      token::ObjectEnd()};
  auto expected = Schema::FromTokens(tokens);

  EXPECT_EQ(schema, expected);
}

TEST(Schema, SchemaApply) {
  Entity example = Entity::example();

  auto json_direct = datapack::write_json(example);

  auto schema = datapack::Schema::Make<Entity>();
  auto bytes = datapack::write_binary(example);
  datapack::Object object;

  schema.apply(datapack::BinaryReader(bytes), datapack::ObjectWriter(object));
  auto json_via_schema = datapack::dump_json(object);

  EXPECT_EQ(json_direct, json_via_schema);
}
