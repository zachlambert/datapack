#include <datapack/object.hpp>
#include <iostream>

void demo_build_primitive() {
  datapack::Object a;
  a = 1.0;
  std::cout << "a: " << a << std::endl;

  datapack::Object b;
  b = "Hello";
  std::cout << "b: " << b << std::endl;

  datapack::Object c;
  c = 12;
  std::cout << "c: " << c << std::endl;

  datapack::Object d;
  d = true;
  std::cout << "d: " << d << std::endl;

  datapack::Object e;
  e = std::vector<uint8_t>{0xAA, 0xBB, 0xCC, 0xDD};
  std::cout << "e: " << e << std::endl;
}

datapack::Object demo_build_map() {
  datapack::Object object;

  object["a"] = "hello";
  object["b"] = 100;
  object["c"] = false;

  object["list"].push_back(0);
  object["list"].push_back(1);
  object["list"].push_back(2);

  object["map"]["foo"] = true;
  object["map"]["bar"] = false;

  std::cout << object << std::endl;
  return object;
}

void demo_clone_copy() {
  datapack::Object a;
  a["foo"] = "foo";
  a["bar"] = "bar";

  auto b = a.clone();
  auto c = a; // Copy

  a["bar"] = "BAR";

  std::cout << "a: " << a << std::endl;
  std::cout << "b: " << b << std::endl;
  std::cout << "c: " << c << std::endl;
}

void demo_iterators() {
  datapack::ConstObject object = demo_build_map();

  std::cout << "Iterate object.items()" << std::endl;
  for (auto [key, value] : object.items()) {
    std::cout << key << ": " << value << std::endl;
  }

  std::cout << "Iterate object.values()" << std::endl;
  for (auto value : object.values()) {
    std::cout << "- " << value << std::endl;
  }

  std::cout << "Iterate object.[\"list\"].values()" << std::endl;
  for (auto value : object["list"].values()) {
    std::cout << "- " << value << std::endl;
  }

  std::cout << "Iterate object.[\"map\"].items()" << std::endl;
  for (auto [key, value] : object["map"].items()) {
    std::cout << key << ": " << value << std::endl;
  }
}

void demo_initializer_list() {
  using namespace datapack;
  Object a = {1, 2, 3};
  std::cout << "a: " << a << std::endl;

  Object b = {{"a", 1}, {"b", 2}, {"c", 3}};
  std::cout << "b: " << b << std::endl;

  Object b2 = Object::make_list({{"a", 1}, {"b", 2}, {"c", 3}});
  std::cout << "b2: " << b2 << std::endl;

  Object c;
  c = {{"foo", {1, 2, 3}}, {"bar", {{"alpha", true}, {"beta", false}}}};
  std::cout << "c: " << c << std::endl;
}

int main() {
  demo_build_primitive();
  demo_build_map();
  demo_clone_copy();
  demo_iterators();
  demo_initializer_list();
}
