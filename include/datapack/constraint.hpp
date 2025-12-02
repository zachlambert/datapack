#pragma once

#include <variant>

namespace datapack {

struct ConstraintNumberRange {
  double lower;
  double upper;
};

struct ConstraintObjectColor {};

using ConstraintNumber = std::variant<ConstraintNumberRange>;
using ConstraintObject = std::variant<ConstraintObjectColor>;
using Constraint = std::variant<ConstraintNumber, ConstraintObject>;

} // namespace datapack
