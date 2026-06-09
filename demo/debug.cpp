#include <datapack/debug.hpp>
#include <datapack/examples/entity.hpp>

int main() {
  std::cout << dpack::debug(Entity::example()) << std::endl;
  return 0;
}
