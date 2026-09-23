#include "datapack/schema/token.hpp"

namespace dpack {

bool operator==(const Token& lhs, const Token& rhs) {
  if (lhs.index() != rhs.index())
    return false;

  // ObjectBegin::debug_name, token::Hint and token::Description are metadata: they
  // describe the data rather than define its structure, so they take part in neither
  // equality nor the schema hash
  if (auto lhs_value = std::get_if<token::Number>(&lhs)) {
    auto rhs_value = std::get_if<token::Number>(&rhs);
    return lhs_value->type == rhs_value->type;
  }
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
  return true;
}

} // namespace dpack
