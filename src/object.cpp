#include "datapack/object.h"

namespace datapack {

void Value::write(Writer& writer) const {
    std::visit([&writer](const auto& x) {
        writer.value(x);
    }, value);
}

} // namespace datapack
