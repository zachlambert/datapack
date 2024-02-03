#pragma once

#include "datapack/token.h"
#include "datapack/object.h"

namespace datapack {

class Parser {
public:
    virtual std::optional<Token> next() = 0;
};

Object parse(Parser& parser);
Object parse(Parser&& parser);

} // namespace datapack
