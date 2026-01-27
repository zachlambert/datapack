#include "datapack/examples/templated.hpp"

namespace datapack {

DATAPACK_TEMPLATED_DEF((typename Foo, typename Bar), (FooBar<Foo, Bar>))
DATAPACK_TEMPLATED_INSTANTIATE(FooBar, (int, std::string));

} // namespace datapack
