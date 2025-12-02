#pragma once

#include <optional>
#include <variant>

namespace datapack {

struct ConstraintNumberRange {
  std::optional<double> lower;
  std::optional<double> upper;
};

struct ConstraintObjectColor {};

using ConstraintNumber = std::variant<ConstraintNumberRange>;
using ConstraintObject = std::variant<ConstraintObjectColor>;
using Constraint = std::variant<ConstraintNumber, ConstraintObject>;

} // namespace datapack
