#include "datapack/examples/templated.hpp"

namespace dpack {

DATAPACK_TEMPLATED_DEF((FooBar<Foo, Bar>), (typename Foo, typename Bar))
DATAPACK_TEMPLATED_INSTANTIATE(FooBar, int, std::string);

} // namespace dpack
