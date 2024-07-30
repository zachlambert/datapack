#include <gtest/gtest.h>
#include <datapack/util/object_writer.hpp>
#include <datapack/examples/entity.hpp>
#include <cmath>

bool compare_map(datapack::Object::ConstReference a, datapack::Object::ConstReference b) {
    // Special case: Unordered map with key != string, uses list of lists
    // and does not know that the order doesn't matter
    auto iter1 = a[0].ptr();
    bool found_all = true;
    while (iter1) {
        bool found = false;
        auto iter2 = b[0].ptr();
        while (iter2) {
            if (*iter1 == *iter2) {
                found = true;
            }
            iter2 = iter2.next();
        }
        if (!found) {
            found_all = false;
        }
        iter1 = iter1.next();
    }
    return found_all;
}

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

        auto properties = expected["properties"];
        auto add_property = [&items](const std::string& name, double value) {
            auto property = items.append(Object::list_t());
            property.append(name);
            property.append(value);
        };
        add_property("strength", 10.5);
        add_property("agility", 5.0);

        auto objects = expected["objects"];
        auto add_flag = [&objects](int index, bool value) {
            auto flag = objects.append(Object::list_t());
            flag.append(index);
            flag.append(value);
        };
        add_flag(0, true);
        add_flag(1, false);
        add_flag(2, true);

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
    EXPECT_TRUE(compare_map(object.at("properties"), expected.at("properties")));
    EXPECT_TRUE(compare_map(object.at("flags"), expected.at("flags")));

    Object object_temp = object.clone();
    EXPECT_TRUE(object == object_temp);
    Object expected_temp = object.clone();
    EXPECT_TRUE(expected == expected_temp);

    object_temp.at("properties").erase();
    object_temp.at("flags").erase();
    expected_temp.at("properties").erase();
    expected_temp.at("flags").erase();

    EXPECT_TRUE(object == expected);
}
