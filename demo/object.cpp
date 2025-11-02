#include <datapack/object.hpp>
#include <iostream>

void demo_mutation() {
  datapack::Object a;
  a = 1.0;
  std::cout << "a: " << a << std::endl;

  datapack::ConstObject b;
  // b = "Hello";
  // std::cout << "b: " << b << std::endl;

  datapack::Object c;
  c.push_back(1);
  c.push_back(2.0);
}

int main() {
  using namespace datapack;

  Object foo;
  foo["a"] = 1.0;
  foo["b"] = "hello";

  for (auto [key, value] : foo.items()) {
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

  Object object_b = object["b"];
  object_b = 3.12;

  std::cout << object << std::endl;
}
