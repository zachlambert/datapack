#include "datapack/schema/token.hpp"

namespace dpack {

DPACK_LABELLED_ENUM_DEF(NumberType) = {"i32", "i64", "u32", "u64", "u8", "f32", "f64"};

DPACK_LABELLED_VARIANT_DEF(Hint) = {"choices", "range", "color"};

DPACK_LABELLED_VARIANT_DEF(Token) = {
    "number",
    "boolean",
    "string",
    "enumerate",
    "binary",
    "optional",
    "variant_begin",
    "variant_next",
    "variant_end",
    "object_begin",
    "object_next",
    "object_end",
    "tuple_begin",
    "tuple_next",
    "tuple_end",
    "list",
    "hint",
    "description"};

namespace {
bool operator==(const Hint& lhs, const Hint& rhs) {
  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (auto lchoices = std::get_if<HintChoices>(&lhs)) {
    auto& rchoices = std::get<HintChoices>(rhs);
    if (lchoices->choices.size() != rchoices.choices.size()) {
      return false;
    }
    for (size_t i = 0; i < lchoices->choices.size(); i++) {
      if (lchoices->choices[i] != rchoices.choices[i]) {
        return false;
      }
    }
  } else if (auto lrange = std::get_if<HintRange>(&lhs)) {
    auto& rrange = std::get<HintRange>(rhs);
    // Must match exactly
    if (lrange->lower != rrange.lower) {
      return false;
    }
    if (lrange->upper != rrange.upper) {
      return false;
    }
  } else if (auto lpositive = std::get_if<HintPositive>(&lhs)) {
    auto& rpositive = std::get<HintPositive>(rhs);
    if (lpositive->allow_zero != rpositive.allow_zero) {
      return false;
    }
  }
  return true;
}
} // namespace

bool operator==(const Token& lhs, const Token& rhs) {
  if (lhs.index() != rhs.index())
    return false;

  if (auto lhs_value = std::get_if<token::Enumerate>(&lhs)) {
    auto rhs_value = std::get_if<token::Enumerate>(&rhs);
    return lhs_value->labels == rhs_value->labels;
  }
  if (auto lhs_value = std::get_if<token::VariantBegin>(&lhs)) {
    auto rhs_value = std::get_if<token::VariantBegin>(&rhs);
    return lhs_value->labels == rhs_value->labels;
  }
  if (auto lhs_value = std::get_if<token::VariantNext>(&lhs)) {
    auto rhs_value = std::get_if<token::VariantNext>(&rhs);
    return lhs_value->index == rhs_value->index;
  }
  if (auto lhs_value = std::get_if<token::ObjectNext>(&lhs)) {
    auto rhs_value = std::get_if<token::ObjectNext>(&rhs);
    return lhs_value->key == rhs_value->key;
  }
  if (auto lhs_hint = std::get_if<token::Hint>(&lhs)) {
    auto rhs_hint = std::get_if<token::Hint>(&rhs);
    if (!(lhs_hint->hint == rhs_hint->hint)) {
      return false;
    }
  }
  if (auto lhs_description = std::get_if<token::Description>(&lhs)) {
    auto rhs_description = std::get_if<token::Description>(&rhs);
    if (lhs_description->description != rhs_description->description) {
      return false;
    }
  }
  return true;
}

} // namespace dpack
