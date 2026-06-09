#include "datapack/smart_ptr.hpp"

namespace datapack {

std::unordered_map<std::type_index, std::unique_ptr<PolyInterfacesBase>> poly_interfaces_;

}
