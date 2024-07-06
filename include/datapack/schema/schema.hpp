#pragma once
#ifndef EMBEDDED

#include "datapack/schema/token.hpp"
#include "datapack/schema/tokenizer.hpp"


namespace datapack {

struct Schema {
    std::vector<Token> tokens;
};

template <readable T>
Schema create_schema() {
    T dummy;
    Schema schema;
    Tokenizer(schema.tokens).value(dummy);
    return schema;
}

void use_schema(const Schema& schema, Reader& reader, Writer& writer);
inline void use_schema(const Schema& schema, Reader&& reader, Writer&& writer) {
    use_schema(schema, reader, writer);
}
DATAPACK(Schema)

bool operator==(const Schema& lhs, const Schema& rhs);

} // namespace datapack
#endif
