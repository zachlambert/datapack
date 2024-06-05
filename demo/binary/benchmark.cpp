#include <chrono>
#include <functional>
#include <datapack/format/binary.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/util/random.hpp>
#include <datapack/common/vector.hpp>
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

void write_direct(const std::vector<Entity>& values, std::vector<std::uint8_t>& data) {
    std::size_t pos = 0;

    for (const auto& value: values) {
        data.push_back(0x01);
        pos = data.size();

        data.resize(pos + 4);
        *((int*)&data[pos]) = value.index;
        pos = data.size();

        data.resize(pos + value.name.size() + 1);
        strcpy((char*)&data[pos], value.name.c_str());
        pos = data.size();

        data.push_back(value.enabled);
        pos = data.size();

        data.resize(pos + 8 * 3);
        *((double*)&data[pos]) = value.pose.x;
        *((double*)&data[pos] + 1) = value.pose.y;
        *((double*)&data[pos] + 2) = value.pose.angle;
        pos = data.size();

        data.push_back((int)value.physics);
        pos = data.size();

        data.push_back(value.hitbox.has_value());
        if (value.hitbox.has_value()) {
            if (auto circle = std::get_if<Circle>(&value.hitbox.value())) {
                std::string label = "circle";
                data.resize(pos + label.size() + 1);
                strcpy((char*)&data[pos], label.c_str());
                pos = data.size();

                data.resize(pos + 8);
                *((double*)&data[pos]) = circle->radius;
                pos = data.size();
            }
            else if (auto rect = std::get_if<Rect>(&value.hitbox.value())) {
                std::string label = "rect";
                data.resize(pos + label.size() + 1);
                strcpy((char*)&data[pos], label.c_str());
                pos = data.size();

                data.resize(pos + 16);
                *((double*)&data[pos]) = rect->width;
                *((double*)&data[pos] + 1) = rect->height;
                pos = data.size();
            }
        }

        data.resize(pos + sizeof(std::size_t) * 2);
        *((std::size_t*)&data[pos]) = value.sprite.width;
        *((std::size_t*)&data[pos] + 1) = value.sprite.height;
        pos = data.size();
        {
            std::size_t size = value.sprite.data.size() * sizeof(Sprite::Pixel);
            data.resize(pos + sizeof(std::size_t));
            *((std::size_t*)&data[pos]) = size;
            pos = data.size();

            data.resize(pos + size);
            std::memcpy(&data[pos], value.sprite.data.data(), size);
            pos = data.size();
        }

        for (const auto& item: value.items) {
            data.push_back(0x01);
            pos = data.size();

            data.resize(pos + sizeof(std::size_t));
            *((std::size_t*)&data[pos]) = item.count;
            pos = data.size();

            data.resize(pos + item.name.size() + 1);
            strcpy((char*)&data[pos], item.name.c_str());
            pos = data.size();
        }
        data.push_back(0x00);
        pos = data.size();

        // Ignore size of assigned items, is fixed
        data.resize(pos + sizeof(std::size_t));
        *((std::size_t*)&data[pos]) = sizeof(value.assigned_items);
        pos = data.size();

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
    data.push_back(0x00);
    pos = data.size();
}

void read_direct(std::vector<Entity>& values, const std::vector<std::uint8_t>& data) {
    std::size_t pos = 0;

    while (true) {
        bool has_next = data[pos];
        pos++;
        if (!has_next) break;

        values.emplace_back();
        auto& value = values.back();

        value.index = *((int*)&data[pos]);
        pos += sizeof(int);

        value.name = (char*)&data[pos];
        pos += value.name.size() + 1;

        value.enabled = data[pos];
        pos++;

        value.pose.x = *((double*)&data[pos]);
        value.pose.y = *((double*)&data[pos] + 1);
        value.pose.angle = *((double*)&data[pos] + 2);
        pos += 3 * sizeof(double);

        value.physics = (Physics)(*((int*)&data[pos]));
        pos += sizeof(int);

        bool hitbox_has_value = data[pos];
        pos++;
        if (hitbox_has_value) {
            std::string label = (char*)&data[pos];
            pos += label.size() + 1;

            if (label == "circle") {
                Circle circle;
                circle.radius = *((double*)&data[pos]);
                pos += sizeof(double);
                value.hitbox = circle;

            } else if (label == "rect") {
                Rect rect;
                rect.width = *((double*)&data[pos]);
                rect.height = *((double*)&data[pos] + 1);
                pos += 2 * sizeof(double);
                value.hitbox = rect;

            } else {
                throw std::runtime_error("invalid label '" + label + "'");
            }
        } else {
            value.hitbox = std::nullopt;
        }

        value.sprite.width = *((std::size_t*)&data[pos]);
        value.sprite.height = *((std::size_t*)&data[pos] + 1);
        pos += 2 * sizeof(std::size_t);
        {
            std::size_t size = *((std::size_t*)&data[pos]);
            pos += sizeof(std::size_t);
            if (size % sizeof(Sprite::Pixel) != 0) {
                throw std::runtime_error("Invalid size " + std::to_string(size));
            }
            value.sprite.data.resize(size / sizeof(Sprite::Pixel));

            std::memcpy((std::uint8_t*)value.sprite.data.data(), &data[pos], size);
            pos += size;
        }

        value.items.clear();
        while (true) {
            bool has_next = data[pos];
            pos++;
            if (!has_next) break;

            Item item;
            item.count = *((std::size_t*)&data[pos]);
            pos += sizeof(std::size_t);
            item.name = (char*)&data[pos];
            pos += item.name.size() + 1;
        }

        {
            std::size_t size = *((std::size_t*)&data[pos]);
            if (size != value.assigned_items.size() * sizeof(int)) {
                throw std::runtime_error("Invalid size " + std::to_string(size));
            }
            pos += sizeof(std::size_t);

            std::memcpy((std::uint8_t*)value.assigned_items.data(), &data[pos], size);
            pos += size;
        }

        value.properties.clear();
        while (true) {
            bool has_next = data[pos];
            pos++;
            if (!has_next) break;

            std::string first;
            first = (char*)&data[pos];
            pos += first.size() + 1;

            double second = *((double*)&data[pos]);
            pos += sizeof(double);

            value.properties.emplace(first, second);
        }

        value.flags.clear();
        while (true) {
            bool has_next = data[pos];
            pos++;
            if (!has_next) break;

            int first;
            first = *((int*)&data[pos]);
            pos += sizeof(int);

            bool second = data[pos];
            pos++;

            value.flags.emplace(first, second);
        }
    }
}

int main() {
    std::size_t N = 10;
    std::vector<Entity> input;
    for (std::size_t n = 0; n < 10; n++) {
        input.push_back(datapack::random<Entity>());
    }
    std::vector<Entity> output;
    std::vector<std::uint8_t> data;
    data.reserve(20000);
    {
        measure("binary write without binary arrays", N, [&](){
            datapack::BinaryWriter writer(data, false);
            auto before = Clock::now();
            writer.value(input);
            auto after = Clock::now();
            return after - before;
        });
    }
    {
        measure("binary write with binary arrays", N, [&](){
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

    data = datapack::write_binary(input);
    {
        measure("binary read without binary arrays", N, [&](){
            datapack::BinaryReader reader(data, false);
            auto before = Clock::now();
            reader.value(output);
            auto after = Clock::now();
            return after - before;
        });
    }
    {
        measure("binary read with binary arrays", N, [&](){
            datapack::BinaryReader reader(data, true);
            auto before = Clock::now();
            reader.value(output);
            auto after = Clock::now();
            return after - before;
        });
    }
    {
        measure("binary direct read", N, [&](){
            auto before = Clock::now();
            read_direct(output, data);
            auto after = Clock::now();
            return after - before;
        });
    }
}
