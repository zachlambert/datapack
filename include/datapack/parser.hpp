#pragma once

#include "datapack/object.hpp"

namespace datapack {

class Parser {
public:
    virtual Token next();
};

} // namespace datapack
