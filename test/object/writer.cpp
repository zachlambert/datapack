#include <gtest/gtest.h>
#include <datapack/util/object_writer.hpp>
#include <datapack/examples/entity.hpp>
#include <cmath>

TEST(Object, Writer) {
    using namespace datapack;

    Entity entity = Entity::example();
    const Object object = write_object(entity);

    const Object expected = []() -> Object {
        Object expected;

        expected["index"] = 5;
        expected["name"] = "player";
        expected["enabled"] = true;

        auto pose = expected["pose"];
        pose["x"] = 1.0;
        pose["y"] = 1.0;
        pose["angle"] = M_PI/2;

        expected["physics"] = "kinematic";

        auto hitbox = expected["hitbox"];
        hitbox["type"] = "circle";
        hitbox["value"]["radius"] = 1.0;

        auto sprite = expected["sprite"];
        sprite["width"] = 2;
        sprite["height"] = 2;
        auto sprite_data = sprite["data"];
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                auto pixel = sprite_data.append(Object::map_t());
                pixel["r"] = (i + 0.5) / 2;
                pixel["g"] = (j + 0.5) / 2;
                pixel["b"] = 0.0;
            }
        }

        auto items = expected["items"];
        auto add_item = [&items](int count, const std::string& name) {
            auto item = items.append(Object::map_t());
            item["count"] = count;
            item["name"] = name;
        };
        add_item(5, "hp_potion");
        add_item(1, "sword");
        add_item(1, "map");
        add_item(120, "gold");

        return expected;
    }();

    EXPECT_TRUE(object.at("index") == expected.at("index"));
    EXPECT_TRUE(object.at("name") == expected.at("name"));
    EXPECT_TRUE(object.at("enabled") == expected.at("enabled"));
    EXPECT_TRUE(object.at("pose") == expected.at("pose"));
    EXPECT_TRUE(object.at("physics") == expected.at("physics"));
    EXPECT_TRUE(object.at("hitbox") == expected.at("hitbox"));
    EXPECT_TRUE(object.at("sprite") == expected.at("sprite"));
    EXPECT_TRUE(object.at("items") == expected.at("items"));
    EXPECT_TRUE(object == expected);
}
