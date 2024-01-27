#pragma once

#include "datapack/token.h"

namespace datapack {

class Parser {
public:
    virtual std::optional<Token> next() = 0;
};

} // namespace datapack
