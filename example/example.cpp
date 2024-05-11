#include "example.hpp"
#include <cstring>
#include <cmath>


void read(datapack::Reader& reader, Circle& value) {
    reader.object_begin();
    reader.value("radius", value.radius);
    reader.object_end();
}

void write(datapack::Writer& writer, const Circle& value) {
    writer.object_begin();
    writer.value("radius", value.radius);
    writer.object_end();
}

void read(datapack::Reader& reader, Rect& value) {
    reader.object_begin();
    reader.value("width", value.width);
    reader.value("height", value.height);
    reader.object_end();
}

void write(datapack::Writer& writer, const Rect& value) {
    writer.object_begin();
    writer.value("width", value.width);
    writer.value("height", value.height);
    writer.object_end();
}

std::vector<const char*> datapack::enum_details<Physics>::labels = {
    "dynamic", "kinematic", "static"
};

const char* datapack::enum_details<Physics>::to_label(const Physics& value) {
    switch (value){
        case Physics::Dynamic:
            return "dynamic";
        case Physics::Kinematic:
            return "kinematic";
        case Physics::Static:
            return "static";
    }
    return "";
}

Physics datapack::enum_details<Physics>::from_label(const char* label) {
    if (std::strcmp(label, "dynamic") == 0) {
        return Physics::Dynamic;
    }
    if (std::strcmp(label, "kinematic") == 0) {
        return Physics::Kinematic;
    }
    if (std::strcmp(label, "static") == 0) {
        return Physics::Static;
    }
    throw std::runtime_error("Invalid label");
    return Physics::Dynamic;
}

std::vector<const char*> datapack::variant_details<Shape>::labels = {
    "circle", "rect"
};

const char* datapack::variant_details<Shape>::to_label(const Shape& value) {
    if (std::get_if<Circle>(&value)) {
        return "circle";
    }
    if (std::get_if<Rect>(&value)) {
        return "rect";
    }
    return "";
}

Shape datapack::variant_details<Shape>::from_label(const char* label) {
    if (std::strcmp(label, "circle") == 0) {
        return Circle();
    }
    if (std::strcmp(label, "rect") == 0) {
        return Rect();
    }
    throw std::runtime_error("Invalid label");
    return Circle();
}

void read(datapack::Reader& reader, Pose& value) {
    reader.object_begin();
    reader.value("x", value.x);
    reader.value("y", value.y);
    reader.value("angle", value.angle);
    reader.object_end();
}

void write(datapack::Writer& writer, const Pose& value) {
    writer.object_begin();
    writer.value("x", value.x);
    writer.value("y", value.y);
    writer.value("angle", value.angle);
    writer.object_end();
}

void read(datapack::Reader& reader, Item& value) {
    reader.object_begin();
    reader.value("count", value.count);
    reader.value("name", value.name);
    reader.object_end();
}

void write(datapack::Writer& writer, const Item& value) {
    writer.object_begin();
    writer.value("count", value.count);
    writer.value("name", value.name);
    writer.object_end();
}

void read(datapack::Reader& reader, Sprite& value) {
    reader.object_begin();
    reader.value("width", value.width);
    reader.value("height", value.height);
    reader.object_next("data");
    reader.value_binary(value.data, value.width * value.height);
    reader.object_end();
}

void write(datapack::Writer& writer, const Sprite& value) {
    writer.object_begin();
    writer.value("width", value.width);
    writer.value("height", value.height);
    writer.object_next("data");
    writer.value_binary(value.data);
    writer.object_end();
}

void Entity::read(datapack::Reader& reader) {
    reader.object_begin();
    reader.value("index", index);
    reader.value("name", name);
    reader.value("enabled", enabled);
    reader.value("pose", pose);
    reader.value("physics", physics);
    reader.value("hitbox", hitbox);
    reader.value("sprite", sprite);
    // reader.value("items", items);
    reader.value("assigned_items", assigned_items);
    reader.value("properties", properties);
    reader.value("flags", flags);
    reader.object_end();
}

void Entity::write(datapack::Writer& writer) const {
    static_assert(datapack::writeable<Item>);
    writer.object_begin();
    writer.value("index", index);
    writer.value("name", name);
    writer.value("enabled", enabled);
    writer.value("pose", pose);
    writer.value("physics", physics);
    writer.value("hitbox", hitbox);
    writer.value("sprite", sprite);
    // writer.value("items", items);
    writer.value("assigned_items", assigned_items);
    writer.value("properties", properties);
    writer.value("flags", flags);
    writer.object_end();
}

Entity Entity::example() {
    Entity result;
    result.index = 5;
    result.name = "player";
    result.enabled = true;
    result.pose = { 1.0, 2.0, M_PI/2 };
    result.physics = Physics::Kinematic;
    result.hitbox = Circle { 1.0 };
    result.sprite = [](){
        Sprite sprite;
        sprite.width = 20;
        sprite.height = 20;
        sprite.data.resize(sprite.height * sprite.width);
        for (int i = 0; i < sprite.height; i++) {
            for (int j = 0; j < sprite.width; j++) {
                auto& pixel = sprite.data[i*sprite.width + j];
                pixel.r = ((double)i + 0.5) / sprite.height;
                pixel.g = ((double)j + 0.5) / sprite.width;
                pixel.b = 0;
            }
        }
        return sprite;
    }();
    result.items = []() {
        std::vector<Item> items;
        items.push_back(Item { 5, "hp_potion" });
        items.push_back(Item { 1, "sword" });
        items.push_back(Item { 1, "map" });
        items.push_back(Item { 120, "gold" });
        return items;
    }();
    result.assigned_items = { 1, 2, -1 };
    result.properties = []() {
        std::unordered_map<std::string, double> properties;
        properties["strength"] = 10.5;
        properties["agility"] = 5.0;
        return properties;
    }();
    result.flags = []() {
        std::unordered_map<int, bool> flags;
        flags[0] = true;
        flags[1] = false;
        flags[2] = true;
        return flags;
    }();
    return result;
}
