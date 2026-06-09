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

struct HintPositive {
  bool allow_zero;

  HintPositive() : allow_zero(false) {}
  HintPositive(bool allow_zero) : allow_zero(allow_zero) {}
};

struct HintColor {};

using Hint = std::variant<HintChoices, HintRange, HintPositive, HintColor>;

} // namespace datapack
