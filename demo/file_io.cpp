#include <datapack/examples/entity.hpp>
#include <datapack/file.hpp>
#include <datapack/std/vector.hpp>
#include <datapack/debug.hpp>
#include <filesystem>

int main() {
  datapack::FileWriter writer("entity.dtp");
  writer.write("list", std::vector<int>{1, 2, 3});
  writer.write<std::string>("string", "hello");
  writer.write("entity", Entity::example());
  writer.close();

  datapack::FileReader reader("entity.dtp");
  while (auto label = reader.next()) {
    std::cerr << "========= " << *label << std::endl;
    if (*label == "list") {
      std::cout << datapack::debug(reader.read<std::vector<int>>());
    } else if (*label == "string") {
      std::cout << datapack::debug(reader.read<std::string>());
    } else if (*label == "entity") {
      std::cout << datapack::debug(reader.read<Entity>());
    }
  }
  reader.close();

  std::filesystem::remove("entity.dtp");
}
