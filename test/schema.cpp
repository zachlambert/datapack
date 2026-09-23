#include <datapack/binary.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/examples/templated.hpp>
#include <datapack/json.hpp>
#include <datapack/random.hpp>
#include <datapack/schema/schema.hpp>
#include <datapack/std/array.hpp>
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

// The tokenizer stores the type's debug name on ObjectBegin, and hints and descriptions are
// attached by the type itself, but all three are metadata: they describe the data rather than
// define its structure, so they must not take part in schema equality or in the schema hash.
// Otherwise editing a description would invalidate every already stored file of that type.
TEST(Schema, HashIgnoresMetadata) {
  using namespace dpack;

  Schema schema = Schema::make<Pose>();

  auto object_begin = schema.begin().object_begin();
  ASSERT_TRUE(object_begin);
  EXPECT_EQ(object_begin->debug_name, "Pose");

  // clang-format off
  auto make_pose_tokens = [](const char* debug_name) {
    return std::vector<Token>{
      token::ObjectBegin(debug_name),
        token::ObjectNext("x"),
          token::Number::F64(),
        token::ObjectNext("y"),
          token::Number::F64(),
        token::ObjectNext("angle"),
          token::Number::F64(),
      token::ObjectEnd()
    };
  };
  // clang-format on

  for (const char* debug_name : {"", "Pose", "SomethingElse"}) {
    auto other = Schema::from_tokens(make_pose_tokens(debug_name));
    EXPECT_EQ(schema, other);
    EXPECT_EQ(schema.hash(), other.hash());
  }

  // clang-format off
  auto make_annotated_tokens = [](const Hint& hint, const std::string& description) {
    return std::vector<Token>{
      token::ObjectBegin("Annotated"),
        token::ObjectNext("value"),
          token::Hint{hint},
          token::Description{description},
          token::Number::F64(),
      token::ObjectEnd()
    };
  };
  // clang-format on

  auto annotated = Schema::from_tokens(make_annotated_tokens(HintRange(0.0, 1.0), "A number"));
  for (const auto& hint : std::vector<Hint>{
           HintRange(0.0, 1.0),
           HintRange(-5.0, 5.0),
           HintChoices{{"a", "b"}},
           HintPositive(true),
           HintColor{}}) {
    for (const std::string& description : {"", "A number", "Something else entirely"}) {
      auto other = Schema::from_tokens(make_annotated_tokens(hint, description));
      EXPECT_EQ(annotated, other);
      EXPECT_EQ(annotated.hash(), other.hash());
    }
  }
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

// Structs used by the hash tests below. Each pair differs only in the way the property
// under test says it should: field types, field order, or nesting.
namespace hash_test {

struct TwoDoubles {
  double x;
  double y;
  DPACK_CLASS_INLINE(x, y)
};

struct TwoStrings {
  std::string x;
  std::string y;
  DPACK_CLASS_INLINE(x, y)
};

struct TwoInts {
  std::int32_t x;
  std::int32_t y;
  DPACK_CLASS_INLINE(x, y)
};

struct TwoBools {
  bool x;
  bool y;
  DPACK_CLASS_INLINE(x, y)
};

struct DoubleThenInt {
  double x;
  std::int32_t y;
  DPACK_CLASS_INLINE(x, y)
};

struct IntThenDouble {
  std::int32_t x;
  double y;
  DPACK_CLASS_INLINE(x, y)
};

struct Inner {
  double b;
  DPACK_CLASS_INLINE(b)
};

struct Nested {
  Inner a;
  DPACK_CLASS_INLINE(a)
};

struct Flat {
  double b;
  DPACK_CLASS_INLINE(b)
};

} // namespace hash_test

// The hash must depend on the type of every field. An accumulator that can cancel
// (eg. xor) loses the types of any field type that appears an even number of times.
TEST(Schema, HashDistinguishesFieldTypes) {
  using namespace dpack;
  using namespace hash_test;

  const std::vector<Schema> schemas = {
      Schema::make<TwoDoubles>(),
      Schema::make<TwoStrings>(),
      Schema::make<TwoInts>(),
      Schema::make<TwoBools>()};

  for (std::size_t i = 0; i < schemas.size(); i++) {
    for (std::size_t j = i + 1; j < schemas.size(); j++) {
      EXPECT_FALSE(schemas[i] == schemas[j]) << "schemas " << i << " and " << j;
      EXPECT_NE(schemas[i].hash(), schemas[j].hash()) << "schemas " << i << " and " << j;
    }
  }
}

// Two structs holding the same field types and keys in a different order have different
// binary layouts, so their hashes must differ too
TEST(Schema, HashIsOrderSensitive) {
  using namespace dpack;
  using namespace hash_test;

  auto a = Schema::make<DoubleThenInt>();
  auto b = Schema::make<IntThenDouble>();
  EXPECT_FALSE(a == b);
  EXPECT_NE(a.hash(), b.hash());

  // The same, with only the keys permuted
  // clang-format off
  auto make_keyed = [](const char* first, const char* second) {
    return Schema::from_tokens({
      token::ObjectBegin(),
        token::ObjectNext(first),
          token::Number::F64(),
        token::ObjectNext(second),
          token::Number::F64(),
      token::ObjectEnd()
    });
  };
  // clang-format on

  auto xy = make_keyed("x", "y");
  auto yx = make_keyed("y", "x");
  EXPECT_FALSE(xy == yx);
  EXPECT_NE(xy.hash(), yx.hash());
}

TEST(Schema, HashDistinguishesNesting) {
  using namespace dpack;
  using namespace hash_test;

  auto nested = Schema::make<Nested>();
  auto flat = Schema::make<Flat>();
  EXPECT_FALSE(nested == flat);
  EXPECT_NE(nested.hash(), flat.hash());
}

// A schema containing no strings must still use the full width of the hash. std::hash of
// an integer is the identity on the major standard libraries, so an implementation that
// feeds token indexes through it leaves every structural signal in the bottom few bits.
TEST(Schema, HashUsesFullWidth) {
  using namespace dpack;

  using IntArray = std::array<int, 3>;
  using DoubleArray = std::array<double, 3>;
  EXPECT_GT(Schema::make<IntArray>().hash(), 0xffffull);
  EXPECT_GT(Schema::make<DoubleArray>().hash(), 0xffffull);

  // An empty schema is hashed too, so it isn't confused with an unhashed one
  EXPECT_NE(Schema::from_tokens({}).hash(), 0ull);
}

TEST(Schema, NumberTypesAreEnforced) {
  using namespace dpack;

  EXPECT_FALSE(Token(token::Number::F64()) == Token(token::Number::I32()));
  EXPECT_TRUE(Token(token::Number::F64()) == Token(token::Number::F64()));

  // clang-format off
  auto make_number = [](const token::Number& number) {
    return Schema::from_tokens({
      token::ObjectBegin(),
        token::ObjectNext("x"),
          number,
      token::ObjectEnd()
    });
  };
  // clang-format on

  const std::vector<token::Number> numbers = {
      token::Number::I32(),
      token::Number::I64(),
      token::Number::U32(),
      token::Number::U64(),
      token::Number::F32(),
      token::Number::F64(),
      token::Number::U8()};

  for (std::size_t i = 0; i < numbers.size(); i++) {
    for (std::size_t j = i + 1; j < numbers.size(); j++) {
      auto a = make_number(numbers[i]);
      auto b = make_number(numbers[j]);
      EXPECT_FALSE(a == b) << "number types " << i << " and " << j;
      EXPECT_NE(a.hash(), b.hash()) << "number types " << i << " and " << j;
    }
  }
}

// A Schema read back from serialized form must recompute its hash, otherwise every
// deserialized schema reports 0 and they all collide
TEST(Schema, SerializedSchemaKeepsHash) {
  using namespace dpack;

  auto schema = Schema::make<Entity>();
  ASSERT_NE(schema.hash(), 0ull);

  auto restored = from_binary<Schema>(to_binary(schema));
  EXPECT_EQ(restored, schema);
  EXPECT_EQ(restored.hash(), schema.hash());

  auto restored_object = from_object<Schema>(to_object(schema));
  EXPECT_EQ(restored_object, schema);
  EXPECT_EQ(restored_object.hash(), schema.hash());
}

// templated.hpp only names Pair<int, double>; this instantiation needs its own name
DPACK_TYPE_NAMES((Pair<int, std::string>), "StringPair", "string_pair");

using IntStringPair = Pair<int, std::string>;
using IntStringFooBar = FooBar<int, std::string>;

TEST(Schema, HashesAreDistinct) {
  using namespace dpack;

  const std::vector<std::pair<const char*, std::uint64_t>> hashes = {
      {"Entity", Schema::make<Entity>().hash()},
      {"Pose", Schema::make<Pose>().hash()},
      {"Circle", Schema::make<Circle>().hash()},
      {"Rect", Schema::make<Rect>().hash()},
      {"Sprite", Schema::make<Sprite>().hash()},
      {"Item", Schema::make<Item>().hash()},
      {"vector<int>", Schema::make<std::vector<int>>().hash()},
      {"string", Schema::make<std::string>().hash()},
      {"Point<double>", Schema::make<Point<double>>().hash()},
      {"Pair<int, string>", Schema::make<IntStringPair>().hash()},
      {"FooBar<int, string>", Schema::make<IntStringFooBar>().hash()}};

  for (std::size_t i = 0; i < hashes.size(); i++) {
    for (std::size_t j = i + 1; j < hashes.size(); j++) {
      EXPECT_NE(hashes[i].second, hashes[j].second)
          << hashes[i].first << " collides with " << hashes[j].first;
    }
  }
}

// The hash is written into every .dpack chunk and checked when reading it back, so it is
// part of the on-disk format. Changing this value means existing files no longer load:
// only update it deliberately, alongside a format change.
TEST(Schema, HashIsStable) {
  using IntVector = std::vector<int>;
  EXPECT_EQ(dpack::Schema::make<Entity>().hash(), 0x66656d2614b00921ull);
  EXPECT_EQ(dpack::Schema::make<Pose>().hash(), 0xa5a3360449febcacull);
  EXPECT_EQ(dpack::Schema::make<IntVector>().hash(), 0x43188e01b34db6eaull);
}
