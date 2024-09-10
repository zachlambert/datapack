#pragma once

#include "datapack/packers.hpp"
#include "datapack/schema/token.hpp"


namespace datapack {

class Tokenizer: public Editor {
public:
    Tokenizer(std::vector<Token>& tokens);

    void primitive(Primitive primtive, void* value) override;
    const char* string(const char*) override;
    int enumerate(int value, const std::span<const char*>& labels) override;

    bool optional_begin(bool has_value) override;
    void optional_end() override;

    int variant_begin(int value, const std::span<const char*>& labels) override;
    void variant_tokenize(int index) override;
    void variant_end() override;

    void binary_data(std::uint8_t* data, std::size_t length, std::size_t stride, bool fixed_length) override;

    void object_begin(std::size_t size) override;
    void object_end(std::size_t size) override;
    void object_next(const char* key) override;

    void tuple_begin(std::size_t size) override;
    void tuple_end(std::size_t size) override;
    void tuple_next() override;

    void list_begin(bool is_trivial) override;
    void list_next() override;
    ContainerAction list_end() override;

private:
    std::vector<Token>& tokens;
    bool first_element;
};

} // namespace datapack
