#include "datapack/examples/templated.hpp"

namespace dpack {

DPACK_TEMPLATED_DEF((FooBar<Foo, Bar>), (typename Foo, typename Bar))
DPACK_TEMPLATED_INSTANTIATE(FooBar, int, std::string);

} // namespace dpack
