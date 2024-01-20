#include "datapack/object.h"

namespace datapack {

#if 0
void Value::write(Writer& writer) const {
    std::visit([&writer](const auto& x) {
        writer.value(x);
    }, value);
}
#endif

} // namespace datapack
