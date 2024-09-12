#include <gtest/gtest.h>
#include <datapack/schema/tokenizer.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/util/debug.hpp>
#include <datapack/common.hpp>


TEST(Schema, Tokenizer) {
    using namespace datapack;

    std::vector<Token> tokens;
    {
        Entity dummy;
        datapack::Tokenizer(tokens).value(dummy);
    }

    std::vector<Token> expected = {
        token::ObjectBegin(),
            token::ObjectNext("index"), IntType::I32,
            token::ObjectNext("name"), std::string(),
            token::ObjectNext("enabled"), bool(),
            token::ObjectNext("pose"),
                token::ObjectBegin(sizeof(Pose)),
                    token::ObjectNext("x"), FloatType::F64,
                    token::ObjectNext("y"), FloatType::F64,
                    token::ObjectNext("angle"), FloatType::F64,
                token::ObjectEnd(sizeof(Pose)),
            token::ObjectNext("physics"),
                token::Enumerate(
                    std::vector<std::string>{"dynamic", "kinematic", "static"}
                ),
            token::ObjectNext("hitbox"), token::Optional(),
                token::VariantBegin(std::vector<std::string>{"circle", "rect"}),
                    token::VariantNext(0),
                        token::ObjectBegin(sizeof(Circle)),
                            token::ObjectNext("radius"), FloatType::F64,
                        token::ObjectEnd(sizeof(Circle)),
                    token::VariantNext(1),
                        token::ObjectBegin(sizeof(Rect)),
                            token::ObjectNext("width"), FloatType::F64,
                            token::ObjectNext("height"), FloatType::F64,
                        token::ObjectEnd(sizeof(Rect)),
                token::VariantEnd(),
            token::ObjectNext("sprite"),
                token::ObjectBegin(),
                    token::ObjectNext("width"), IntType::U64,
                    token::ObjectNext("height"), IntType::U64,
                    token::ObjectNext("data"),
                        token::List(true),
                            token::ObjectBegin(sizeof(Sprite::Pixel)),
                                token::ObjectNext("r"), FloatType::F64,
                                token::ObjectNext("g"), FloatType::F64,
                                token::ObjectNext("b"), FloatType::F64,
                            token::ObjectEnd(sizeof(Sprite::Pixel)),
                token::ObjectEnd(),
            token::ObjectNext("items"),
                token::List(),
                    token::ObjectBegin(),
                        token::ObjectNext("count"), IntType::U64,
                        token::ObjectNext("name"), std::string(),
                    token::ObjectEnd(),
            token::ObjectNext("assigned_items"),
                token::TupleBegin(sizeof(Entity::assigned_items)),
                    token::TupleNext(), IntType::I32,
                    token::TupleNext(), IntType::I32,
                    token::TupleNext(), IntType::I32,
                token::TupleEnd(sizeof(Entity::assigned_items)),
        token::ObjectEnd()
    };

    ASSERT_EQ(tokens.size(), expected.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] != expected[i]) {
            std::cerr << "Token["  << i << "]:\n" << datapack::debug(tokens[i]) << std::endl;
            std::cerr << "Expected:\n" << datapack::debug(expected[i]) << std::endl;
        }
        EXPECT_TRUE(tokens[i] == expected[i]);
    }
}
