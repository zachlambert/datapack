#pragma once

#include "datapack/packers.hpp"
#include "datapack/schema/token.hpp"


namespace datapack {

class Tokenizer: public Reader {
public:
    Tokenizer(std::vector<Token>& tokens);

    void integer(IntType type, void* value) override;
    void floating(FloatType type, void* value) override;
    bool boolean() override;
    const char* string() override;
    int enumerate(const std::span<const char*>& labels) override;
    std::tuple<const std::uint8_t*, std::size_t> binary(std::size_t length, std::size_t stride) override;

    bool optional_begin() override;
    void optional_end() override;

    int variant_begin(const std::span<const char*>& labels) override;
    void variant_tokenize(int index) override;
    void variant_end() override;

    void object_begin(std::size_t size) override;
    void object_next(const char* key) override;
    void object_end(std::size_t size) override;

    void tuple_begin(std::size_t size) override;
    void tuple_next() override;
    void tuple_end(std::size_t size) override;

    void list_begin(bool is_trivial) override;
    bool list_next() override;
    void list_end() override;

private:
    std::vector<Token>& tokens;
    bool first_element;
};

} // namespace datapack
