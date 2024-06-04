#include <chrono>
#include <functional>
#include <datapack/format/binary.hpp>
#include <datapack/examples/entity.hpp>
#include <assert.h>
#include <cstring>


using Clock = std::chrono::high_resolution_clock;
void measure(const std::string& label, std::size_t N, const std::function<Clock::duration()>& func) {
    // A few dummy invocations at the start to avoid one-off initial costs
    for (std::size_t i = 0; i < 5; i++) {
        func();
    }

    Clock::duration::rep nanos = 0;
    for (std::size_t i = 0; i < N; i++) {
        nanos += std::chrono::duration_cast<std::chrono::nanoseconds>(func()).count();
    }
    nanos /= N;
    std::cout << label << ": " << nanos << std::endl;
}

void write_direct(const Entity& value, std::vector<std::uint8_t>& data) {
    std::size_t pos = 0;

    data.resize(pos + 4);
    *((int*)&data[pos]) = value.index;
    pos = data.size();

    data.resize(pos + value.name.size() + 1);
    strcpy((char*)&data[pos], value.name.c_str());
    pos = data.size();

    data.push_back(value.enabled);
    pos = data.size();

#if 1
    data.resize(pos + 8 * 3);
    *((double*)&data[pos]) = value.pose.x;
    *((double*)&data[pos] + 1) = value.pose.y;
    *((double*)&data[pos] + 2) = value.pose.angle;
    pos = data.size();
#else
    data.resize(pos + sizeof(Pose));
    *((Pose*)&data[pos]) = value.pose;
    pos = data.size();
#endif

    data.push_back((std::uint8_t)value.physics);
    pos = data.size();

    data.push_back(value.hitbox.has_value());
    if (value.hitbox.has_value()) {
        if (auto circle = std::get_if<Circle>(&value.hitbox.value())) {
            std::string label = "circle";
            data.resize(pos + label.size() + 1);
            strcpy((char*)&data[pos], label.c_str());
            pos = data.size();

#if 1
            data.resize(pos + 8);
            *((double*)&data[pos]) = circle->radius;
            pos = data.size();
#else
            data.resize(pos + sizeof(Circle));
            *((Circle*)&data[pos]) = *circle;
            pos = data.size();
#endif
        }
        else if (auto rect = std::get_if<Rect>(&value.hitbox.value())) {
            std::string label = "rect";
            data.resize(pos + label.size() + 1);
            strcpy((char*)&data[pos], label.c_str());
            pos = data.size();

#if 1
            data.resize(pos + 16);
            *((double*)&data[pos]) = rect->width;
            *((double*)&data[pos] + 1) = rect->height;
            pos = data.size();
#else
            data.resize(pos + sizeof(Rect));
            *((Rect*)&data[pos]) = *rect;
            pos = data.size();
#endif
        }
    }

    data.resize(pos + sizeof(std::size_t) * 2);
    *((std::size_t*)&data[pos]) = value.sprite.width;
    *((std::size_t*)&data[pos] + 1) = value.sprite.height;
    pos = data.size();
    {
        std::size_t size = value.sprite.data.size() * sizeof(Sprite::Pixel);
        data.resize(pos + size);
        std::memcpy(&data[pos], value.sprite.data.data(), size);
        pos = data.size();
    }

#if 0
    for (const auto& item: value.items) {
        data.push_back(true);
        pos = data.size();
        data.resize(pos + sizeof(Item));
        *((Item*)&data[pos]) = item;
    }
#else
    data.resize(pos + sizeof(std::size_t));
    *((std::size_t*)&data[pos]) = value.items.size() * sizeof(Item);
    pos = data.size();
    data.resize(pos + value.items.size() * sizeof(Item));
    std::memcpy(&data[pos], value.items.data(), value.items.size() * sizeof(Item));
    pos = data.size();
#endif

    data.resize(pos + sizeof(value.assigned_items));
    std::memcpy(&data[pos], &value.assigned_items, sizeof(value.assigned_items));
    pos = data.size();

    for (const auto& pair: value.properties) {
        data.push_back(true);
        pos = data.size();

        data.resize(pos + pair.first.size() + 1);
        strcpy((char*)&data[pos], pair.first.c_str());
        pos = data.size();

        data.resize(pos + 8);
        *((double*)&data[pos]) = pair.second;
        pos = data.size();
    }

    for (const auto& pair: value.flags) {
        data.push_back(true);
        pos = data.size();

        data.resize(pos + 4);
        *((int*)&data[pos]) = pair.first;
        pos = data.size();

        data.push_back(pair.second);
        pos = data.size();
    }
}

int main() {
    std::size_t N = 10000;
    Entity input = Entity::example();
    Entity output;
    std::vector<std::uint8_t> data;
    data.reserve(20000);
    {
        measure("binary verbose write", N, [&](){
            datapack::BinaryWriter writer(data, false);
            auto before = Clock::now();
            writer.value(input);
            auto after = Clock::now();
            return after - before;
        });
    }
    {
        measure("binary efficient write", N, [&](){
            datapack::BinaryWriter writer(data, true);
            auto before = Clock::now();
            writer.value(input);
            auto after = Clock::now();
            return after - before;
        });
    }
    {
        measure("binary direct write", N, [&](){
            auto before = Clock::now();
            write_direct(input, data);
            auto after = Clock::now();
            return after - before;
        });
    }

    {
        std::vector<std::uint8_t> data;
        datapack::BinaryWriter writer(data, false);
        writer.value(input);

        measure("binary verbose read", N, [&](){
            datapack::BinaryReader reader(data, false);
            auto before = Clock::now();
            reader.value(output);
            auto after = Clock::now();
            return after - before;
        });
    }

    {
        std::vector<std::uint8_t> data;
        datapack::BinaryWriter writer(data, true);
        writer.value(input);

        measure("binary efficient read", N, [&](){
            datapack::BinaryReader reader(data, true);
            auto before = Clock::now();
            reader.value(output);
            auto after = Clock::now();
            return after - before;
        });
    }
}
