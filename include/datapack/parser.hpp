#pragma once

#include "datapack/token.hpp"

namespace datapack {

class Parser {
public:
    virtual Token next();
};

} // namespace datapack
