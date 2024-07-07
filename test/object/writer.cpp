#include <gtest/gtest.h>
#include <datapack/util/object_writer.hpp>
#include <datapack/examples/entity.hpp>
#include <cmath>

bool compare_map(datapack::ConstObject object, datapack::ConstObject expected) {
    // Special case: Unordered map with key != string, uses list of lists
    // and does not know that the order doesn't matter
    auto iter1 = object["flags"][0];
    bool found_all = true;
    while (iter1) {
        bool found = false;
        auto iter2 = expected["flags"][0];
        while (iter2) {
            if (compare(iter1, iter2)) {
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
    Object object = write_object(entity);

    Object expected = Object(Object::map_t());

    expected.insert("index", 5);
    EXPECT_TRUE(compare(object["index"], expected["index"]));

    expected.insert("name", "player");
    EXPECT_TRUE(compare(object["name"], expected["name"]));

    expected.insert("enabled", true);
    EXPECT_TRUE(compare(object["enabled"], expected["enabled"]));

    [](Object object) {
        object.insert("x", 1.0);
        object.insert("y", 2.0);
        object.insert("z", M_PI/2);
    }(expected.insert("pose", Object::map_t()));
    EXPECT_TRUE(compare(object["pose"], expected["pose"]));

    expected.insert("physics", "kinematic");
    EXPECT_TRUE(compare(object["physics"], expected["physics"]));

    expected.insert("hitbox", Object::map_t());
    expected["hitbox"].insert("type", "circle");
    expected["hitbox"].insert("value", Object::map_t());
    expected["hitbox"]["value"].insert("radius", 1.0);
    EXPECT_TRUE(compare(object["hitbox"], expected["hitbox"]));

    [](Object object) {
        object.insert("width", 2);
        object.insert("height", 2);

        auto data = object.insert("data", Object::list_t());
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                auto pixel = data.append(Object::map_t());
                pixel.insert("r", ((double)i + 0.5) / 2);
                pixel.insert("g", ((double)j + 0.5) / 2);
                pixel.insert("b", 0.0);
            }
        }
    }(expected.insert("sprite", Object::map_t()));
    EXPECT_TRUE(compare(object["sprite"], expected["sprite"]));

    [](Object object) {
        auto add_item = [&object](int count, const std::string& name) {
            auto item = object.append(Object::map_t());
            item.insert("count", count);
            item.insert("name", name);
        };
        add_item(5, "hp_potion");
        add_item(1, "sword");
        add_item(1, "map");
        add_item(120, "gold");
    }(expected.insert("items", Object::list_t()));
    EXPECT_TRUE(compare(object["items"], expected["items"]));

    [](Object object) {
        object.insert("strength", 10.5);
        object.insert("agility", 5.0);
    }(expected.insert("properties", Object::map_t()));

    EXPECT_TRUE(compare_map(object["properties"], expected["properties"]));
    object["properties"].erase();
    expected["properties"].erase();

    [](Object object) {
        auto add_flag = [&object](int index, bool value) {
            auto flag = object.append(Object::list_t());
            flag.append(index);
            flag.append(value);
        };
        add_flag(0, true);
        add_flag(1, false);
        add_flag(2, true);
    }(expected.insert("flags", Object::list_t()));

    EXPECT_TRUE(compare_map(object["flags"], expected["properties"]));
    object["flags"].erase();
    expected["flags"].erase();

    EXPECT_TRUE(compare(object, expected));
}
