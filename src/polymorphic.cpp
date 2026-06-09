#include "datapack/polymorphic.hpp"

namespace dpack {

std::unordered_map<std::type_index, std::unique_ptr<PolyInterfacesBase>> poly_interfaces_;

}
