#pragma once

#include <string>
#include <variant>
#include <vector>

namespace datapack {

struct HintNumberRange {
  double lower;
  double upper;
};

struct HintObjectColor {};

struct HintStringChoices {
  std::vector<std::string> choices;
};

using HintNumber = std::variant<HintNumberRange>;
using HintObject = std::variant<HintObjectColor>;
using HintString = std::variant<HintStringChoices>;
using Hint = std::variant<HintNumber, HintObject, HintString>;

} // namespace datapack
