#include <datapack/debug.hpp>
#include <datapack/examples/entity.hpp>

int main() {
  std::cout << datapack::debug(Entity::example()) << std::endl;
  return 0;
}
