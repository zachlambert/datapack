#include <datapack/debug.hpp>
#include <datapack/examples/entity.hpp>
#include <datapack/file.hpp>
#include <datapack/std/vector.hpp>
#include <filesystem>

int main() {
  dpack::FileWriter writer("entity.dpack");
  writer.write("list", std::vector<int>{1, 2, 3});
  writer.write<std::string>("string", "hello");
  writer.write("entity", Entity::example());
  writer.close();

  dpack::FileReader reader("entity.dpack");
  while (auto label = reader.next()) {
    std::cerr << "========= " << *label << std::endl;
    if (*label == "list") {
      std::cout << dpack::debug(reader.read<std::vector<int>>());
    } else if (*label == "string") {
      std::cout << dpack::debug(reader.read<std::string>());
    } else if (*label == "entity") {
      std::cout << dpack::debug(reader.read<Entity>());
    }
  }
  reader.close();

  std::filesystem::remove("entity.dpack");
}
