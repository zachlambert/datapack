#include <datapack/object.hpp>
#include <iostream>

int main() {
  using namespace datapack;

  Object foo;
  foo["a"] = 1.0;
  foo["b"] = "hello";

  for (auto [key, value] : foo) {
    std::cout << key << ": " << value << std::endl;
  }

  Object bar;
  bar.push_back(0.0);
  bar.push_back(1.0);
  std::cout << bar << std::endl;

  Object object;
  object["a"] = 1.0;
  object["a"] = 2.0;
  object["b"] = "hello";
  object["c"]["first"] = "first";
  object["c"]["second"] = "second";
  object["d"].push_back(100);

  std::cout << object << std::endl;
}
