#include "datapack/parser.h"

namespace datapack {

Object parse(Parser& parser) {
    Object object;
    while (true) {
        auto next = parser.next();
        if (!next.has_value()) {
            break;
        }
        object.tokens.push_back(next.value());
    }
    return object;
}

Object parse(Parser&& parser) {
    return parse(parser);
}


} // namespace datapack
