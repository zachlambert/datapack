#pragma once

#include <concepts>
#include <type_traits>

namespace datapack {

class ConstraintBase {
public:
  virtual ~ConstraintBase() {}
};

template <typename Constraint>
concept is_constraint = requires() { std::is_base_of_v<ConstraintBase, Constraint>; };

template <typename T, typename Constraint>
concept is_constrained = requires(const T& value, const Constraint& constraint) {
  is_constraint<Constraint>;
  { validate(value, constraint) } -> std::convertible_to<bool>;
};

struct RangeConstraint : public ConstraintBase {
  const double lower;
  const double upper;
  RangeConstraint(double lower, double upper) : lower(lower), upper(upper) {}
  bool validate(double value) const { return lower <= value && value <= upper; }
};

struct LengthConstraint : public ConstraintBase {
  const std::size_t length;
  LengthConstraint(std::size_t length) : length(length) {}
};

struct OrderedConstraint : public ConstraintBase {};

} // namespace datapack
