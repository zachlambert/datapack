#pragma once

#include <memory>
#include <variant>
#include <optional>
#include <vector>

#include "datapack/token.hpp"
#include "datapack/writer.hpp"
#include "datapack/reader.hpp"

namespace datapack {

struct Object {
    std::vector<Token> tokens;
};

void write(Writer& writer, const Object& object);

class ObjectWriter: public Writer {
public:
    void i32(int value) override;
    void i64(long value) override;
    void f32(float value) override;
    void f64(double value) override;
    void string(const std::string& value) override;
    void boolean(bool value) override;
    void null() override;
    void binary(const binary_t& value) override;

    void object_begin() override;
    void object_end() override;
    void object_element(const std::string& key) override;

    void array_begin() override;
    void array_end() override;
    void array_element() override;

    const Object& result() {
        return object;
    }

private:
    Object object;
};

class ObjectReader: public Reader {
public:
    ObjectReader(const Object& object);

    void i32(int& value) override;
    void i64(long& value) override;
    void f32(float& value) override;
    void f64(double& value) override;
    void string(std::string& value) override;
    void boolean(bool& value) override;
    bool null() override;
    void binary(binary_t& value) override;

    void object_begin() override;
    void object_end() override;
    void object_element(const std::string& key) override;

    void array_begin() override;
    void array_end() override;
    bool array_element() override;

private:
    const Object& object;
    std::size_t pos;
};

} // namespace datapack
