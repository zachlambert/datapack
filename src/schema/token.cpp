#include "datapack/schema/token.hpp"
#include "datapack/common.hpp"

namespace datapack {

DATAPACK_IMPL(token::Enumerate) {
    visitor.object_begin();
    visitor.value("labels", value.labels);
    visitor.object_end();
}

DATAPACK_IMPL(token::VariantBegin) {
    visitor.object_begin();
    visitor.value("labels", value.labels);
    visitor.object_end();
}

DATAPACK_IMPL(token::VariantNext) {
    visitor.object_begin();
    visitor.value("type", value.type);
    visitor.object_end();
}

DATAPACK_IMPL(token::BinaryData) {
    visitor.object_begin();
    visitor.value("length", value.stride);
    visitor.value("stride", value.length);
    visitor.object_end();
}

DATAPACK_IMPL(token::ObjectBegin) {
    visitor.object_begin();
    visitor.value("size", value.size);
    visitor.object_end();
}

DATAPACK_IMPL(token::ObjectEnd) {
    visitor.object_begin();
    visitor.value("size", value.size);
    visitor.object_end();
}

DATAPACK_IMPL(token::ObjectNext) {
    visitor.object_begin();
    visitor.value("key", value.key);
    visitor.object_end();
}

DATAPACK_IMPL(token::TupleBegin) {
    visitor.object_begin();
    visitor.value("size", value.size);
    visitor.object_end();
}

DATAPACK_IMPL(token::TupleEnd) {
    visitor.object_begin();
    visitor.value("size", value.size);
    visitor.object_end();
}

DATAPACK_IMPL(token::List) {
    visitor.object_begin();
    visitor.value("is_trivial", value.is_trivial);
    visitor.object_end();
}

DATAPACK_LABELLED_VARIANT_DEF(Token) = {
    "i32", "i64", "u32", "u64", "f32", "f64",
    "string", "boolean",
    "optional", "enumerate",
    "variant_begin", "variant_end", "variant_next",
    "binary_data",
    "object_begin", "object_end", "object_next",
    "tuple_begin", "tuple_end", "tuple_next",
    "list"
};

bool operator==(const Token& lhs, const Token& rhs) {
    if (lhs.index() != rhs.index()) return false;

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
        return lhs_value->type == rhs_value->type;
    }
    if (auto lhs_value = std::get_if<token::ObjectBegin>(&lhs)) {
        auto rhs_value = std::get_if<token::ObjectBegin>(&rhs);
        return lhs_value->size == rhs_value->size;
    }
    if (auto lhs_value = std::get_if<token::ObjectEnd>(&lhs)) {
        auto rhs_value = std::get_if<token::ObjectEnd>(&rhs);
        return lhs_value->size == rhs_value->size;
    }
    if (auto lhs_value = std::get_if<token::ObjectNext>(&lhs)) {
        auto rhs_value = std::get_if<token::ObjectNext>(&rhs);
        return lhs_value->key == rhs_value->key;
    }
    if (auto lhs_value = std::get_if<token::TupleBegin>(&lhs)) {
        auto rhs_value = std::get_if<token::TupleBegin>(&rhs);
        return lhs_value->size == rhs_value->size;
    }
    if (auto lhs_value = std::get_if<token::TupleEnd>(&lhs)) {
        auto rhs_value = std::get_if<token::TupleEnd>(&rhs);
        return lhs_value->size == rhs_value->size;
    }
    if (auto lhs_value = std::get_if<token::List>(&lhs)) {
        auto rhs_value = std::get_if<token::List>(&rhs);
        return lhs_value->is_trivial == rhs_value->is_trivial;
    }

    return true;
}

} // namespace datapack
