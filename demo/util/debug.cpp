#include <datapack/examples/entity.hpp>
#include <datapack/util/debug.hpp>

int main() {
  std::cout << datapack::debug(Entity::example()) << std::endl;
  return 0;
}
