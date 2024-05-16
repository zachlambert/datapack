#include "datapack/schema.hpp"
#include <iostream>

namespace datapack {

} // namespace datapack

std::ostream& operator<<(std::ostream& os, const std::vector<datapack::DToken>& tokens) {
    using namespace datapack::dtoken;
    int depth = 0;
    auto indent = [&]() {
        for (int i = 0; i < depth; i++) {
            os << "    ";
        }
    };
    for (const auto& token: tokens) {
        if (std::get_if<std::int32_t>(&token)) {
            os << "i32\n";
        }
        else if (std::get_if<std::int64_t>(&token)) {
            os << "i64\n";
        }
        else if (std::get_if<std::uint32_t>(&token)) {
            os << "u32\n";
        }
        else if (std::get_if<std::uint64_t>(&token)) {
            os << "u64\n";
        }
        else if (std::get_if<float>(&token)) {
            os << "f32\n";
        }
        else if (std::get_if<double>(&token)) {
            os << "f64\n";
        }
        else if (std::get_if<std::string>(&token)) {
            os << "string\n";
        }
        else if (std::get_if<bool>(&token)) {
            os << "bool\n";
        }

        else if (std::get_if<Optional>(&token)) {
            os << "optional: ";
        }

        else if (auto x = std::get_if<Enumerate>(&token)) {
            os << "enum:\n";
            depth++;
            for (const auto& label: x->labels) {
                indent();
                os << label << "\n";
            }
            depth--;
        }

        else if (std::get_if<VariantBegin>(&token)) {
            os << "variant:\n";
            depth++;
        }
        else if (std::get_if<VariantEnd>(&token)) {
            depth--;
        }
        else if (auto x = std::get_if<VariantNext>(&token)) {
            indent();
            os << x->type << ": ";
        }

        else if (auto x = std::get_if<Binary>(&token)) {
            if (x->expected_size == 0) {
                os << "binary: ";
            } else {
                os << "binary (expected_size = " << x->expected_size << "): ";
            }
        }

        else if (auto x = std::get_if<ObjectBegin>(&token)) {
            os << "object:\n";
            depth++;
        }
        else if (auto x = std::get_if<ObjectEnd>(&token)) {
            depth--;
        }
        else if (auto x = std::get_if<ObjectNext>(&token)) {
            indent();
            os << x->key << ": ";
        }

        else if (auto x = std::get_if<TupleBegin>(&token)) {
            os << "tuple:\n";
            depth++;
        }
        else if (auto x = std::get_if<TupleEnd>(&token)) {
            depth--;
        }
        else if (auto x = std::get_if<TupleNext>(&token)) {
            indent();
        }

        else if (auto x = std::get_if<Map>(&token)) {
            os << "map: ";
        }
        else if (auto x = std::get_if<List>(&token)) {
            os << "list: ";
        }
    }
    return os;
}
