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

TEST(Schema, SchemaMake) {
  using namespace datapack;

  std::vector<Token> tokens;
  {
    Entity dummy;
    datapack::Tokenizer(tokens).value(dummy);
  }

  std::vector<Token> expected = {
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

  ASSERT_EQ(tokens.size(), expected.size());
  for (std::size_t i = 0; i < tokens.size(); i++) {
    if (tokens[i] != expected[i]) {
      std::cerr << "Token[" << i << "]:\n" << datapack::debug(tokens[i]) << std::endl;
      std::cerr << "Expected:\n" << datapack::debug(expected[i]) << std::endl;
    }
    EXPECT_TRUE(tokens[i] == expected[i]);
  }
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
