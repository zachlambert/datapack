#include "example.hpp"
#include <cstring>

void read(datapack::Reader& reader, Circle& value) {

}

void write(datapack::Writer& writer, const Circle& value) {

}

void read(datapack::Reader& reader, Rect& value) {

}

void write(datapack::Writer& writer, const Rect& value) {

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
    reader.object_end();
}

void Entity::write(datapack::Writer& writer) const {
    writer.object_begin();
    writer.value("index", index);
    writer.value("name", name);
    writer.value("enabled", enabled);
    writer.value("pose", pose);
    writer.value("physics", physics);
    writer.object_end();
}
