#pragma once

#include <string>
#include <variant>
#include <vector>

namespace datapack {

struct HintChoices {
  std::vector<std::string> choices;
};

struct HintRange {
  double lower;
  double upper;
};

struct HintColor {};

using Hint = std::variant<HintChoices, HintRange, HintColor>;

} // namespace datapack
