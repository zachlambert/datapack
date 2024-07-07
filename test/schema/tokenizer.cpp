#include <gtest/gtest.h>
#include <datapack/schema/tokenizer.hpp>
#include <datapack/examples/entity.hpp>


TEST(Schema, Tokenizer) {
    using namespace datapack;

    std::vector<Token> tokens;
    {
        Entity dummy;
        datapack::Tokenizer(tokens).value(dummy);
    }

    std::vector<Token> expected = {
        token::ObjectBegin(),
            token::ObjectNext("index"), int(),
            token::ObjectNext("name"), std::string(),
            token::ObjectNext("enabled"), bool(),
            token::ObjectNext("pose"),
                token::ObjectBegin(),
                    token::ObjectNext("x"), double(),
                    token::ObjectNext("y"), double(),
                    token::ObjectNext("angle"), double(),
                token::ObjectEnd(),
            token::ObjectNext("physics"),
                token::Enumerate(
                    std::vector<std::string>{"dynamic", "kinematic", "static"}
                ),
            token::ObjectNext("hitbox"), token::Optional(),
                token::VariantBegin(std::vector<std::string>{"circle", "rect"}),
                    token::VariantNext("circle"),
                        token::ObjectBegin(),
                            token::ObjectNext("radius"), double(),
                        token::ObjectEnd(),
                    token::VariantNext("rect"),
                        token::ObjectBegin(),
                            token::ObjectNext("width"), double(),
                            token::ObjectNext("height"), double(),
                        token::ObjectEnd(),
                token::VariantEnd(),
            token::ObjectNext("sprite"),
                token::ObjectBegin(),
                    token::ObjectNext("width"), std::size_t(),
                    token::ObjectNext("height"), std::size_t(),
                    token::ObjectNext("data"),
                        token::List(true),
                            token::ObjectBegin(),
                                token::ObjectNext("r"), double(),
                                token::ObjectNext("g"), double(),
                                token::ObjectNext("b"), double(),
                            token::ObjectEnd(),
                token::ObjectEnd(),
            token::ObjectNext("items"),
                token::List(),
                    token::ObjectBegin(),
                        token::ObjectNext("count"), std::size_t(),
                        token::ObjectNext("name"), std::string(),
                    token::ObjectEnd(),
            token::ObjectNext("assigned_items"),
                token::List(true),
                    int(),
            token::ObjectNext("properties"),
                token::List(),
                    token::TupleBegin(),
                        token::TupleNext(), std::string(),
                        token::TupleNext(), double(),
                    token::TupleEnd(),
            token::ObjectNext("flags"),
                token::List(),
                    token::TupleBegin(),
                        token::TupleNext(), int(),
                        token::TupleNext(), bool(),
                    token::TupleEnd(),
        token::ObjectEnd()
    };

    ASSERT_EQ(tokens.size(), expected.size());
    for (std::size_t i = 0; i < tokens.size(); i++) {
        EXPECT_TRUE(tokens[i] == expected[i]);
    }
}
