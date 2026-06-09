#pragma once

#include "datapack/datapack.hpp"
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace datapack {

class PolyError : public std::runtime_error {
public:
  PolyError(const std::string& message) : std::runtime_error(message) {}
};

template <typename Base>
class PolyTypeInterface {
public:
  PolyTypeInterface(const std::string& label, int index) : label_(label), index_(index) {}

  const std::string& label() const {
    return label_;
  }
  const int index() const {
    return index_;
  }

  virtual bool matches(const Base* ptr) const = 0;

  virtual void write(Writer& writer, const std::unique_ptr<Base>& ptr) const = 0;
  virtual void read(Reader& reader, std::unique_ptr<Base>& ptr) const = 0;

  virtual void write(Writer& writer, const std::shared_ptr<Base>& ptr) const = 0;
  virtual void read(Reader& reader, std::shared_ptr<Base>& ptr) const = 0;

  virtual void read_dummy(Reader& reader) const = 0;

private:
  std::string label_;
  int index_;
};

template <typename Base, typename Child>
class PolyTypeImpl : public PolyTypeInterface<Base> {
public:
  PolyTypeImpl(const std::string& label, int index) : PolyTypeInterface<Base>(label, index) {}

  bool matches(const Base* ptr) const override {
    return dynamic_cast<const Child*>(ptr);
  }

  void write(Writer& writer, const std::unique_ptr<Base>& ptr) const override {
    const Child* child = dynamic_cast<const Child*>(ptr.get());
    writer.value(*child);
  }
  void read(Reader& reader, std::unique_ptr<Base>& ptr) const override {
    auto child_ptr = std::make_unique<Child>();
    reader.value(*child_ptr);
    ptr = std::move(child_ptr);
  }

  void write(Writer& writer, const std::shared_ptr<Base>& ptr) const override {
    const Child* child = dynamic_cast<const Child*>(ptr.get());
    writer.value(*child);
  }
  void read(Reader& reader, std::shared_ptr<Base>& ptr) const override {
    auto child_ptr = std::make_shared<Child>();
    reader.value(*child_ptr);
    ptr = std::move(child_ptr);
  }

  void read_dummy(Reader& reader) const override {
    Child dummy;
    reader.value(dummy);
  }
};

class PolyInterfacesBase {
public:
  virtual ~PolyInterfacesBase() {}
};

template <typename Base>
class PolyInterfaces : public PolyInterfacesBase {
public:
  template <typename Child>
  void add(const std::string& label) {
    for (const auto& interface : interfaces_) {
      if (interface->label() == label) {
        return;
      }
    }
    interfaces_.push_back(std::make_unique<PolyTypeImpl<Base, Child>>(label, interfaces_.size()));
    labels_.push_back(label);
    rebuild_labels_cstr();
  }

  void write(Writer& writer, const std::unique_ptr<Base>& value) {
    auto interface = get(value.get());
    writer.variant_begin(interface->index(), labels_cstr_);
    interface->write(writer, value);
    writer.variant_end();
  }

  void write(Writer& writer, const std::shared_ptr<Base>& value) {
    auto interface = get(value.get());
    writer.variant_begin(interface->index(), labels_cstr_);
    interface->write(writer, value);
    writer.variant_end();
  }

  void read(Reader& reader, std::unique_ptr<Base>& value) {
    if (reader.is_tokenizer()) {
      tokenize(reader);
      return;
    }
    const int index = reader.variant_begin(labels_cstr_);
    auto interface = get(index);
    interface->read(reader, value);
    reader.variant_end();
  }

  void read(Reader& reader, std::shared_ptr<Base>& value) {
    if (reader.is_tokenizer()) {
      tokenize(reader);
      return;
    }
    const int index = reader.variant_begin(labels_cstr_);
    auto interface = get(index);
    interface->read(reader, value);
    reader.variant_end();
  }

private:
  const PolyTypeInterface<Base>* get(const Base* ptr) {
    for (const auto& interface : interfaces_) {
      if (interface->matches(ptr)) {
        return interface.get();
      }
    }
    throw PolyError("Missing smart ptr registration");
    return nullptr;
  }

  const PolyTypeInterface<Base>* get(int index) {
    if (index >= interfaces_.size()) {
      throw PolyError("Missing smart ptr registration");
    }
    return interfaces_[index].get();
  }

  void rebuild_labels_cstr() {
    labels_cstr_.clear();
    for (const auto& label : labels_) {
      labels_cstr_.push_back(label.c_str());
    }
  }

  void tokenize(Reader& reader) {
    reader.variant_begin(labels_cstr_);
    for (int i = 0; i < interfaces_.size(); i++) {
      reader.variant_tokenize(i);
      interfaces_[i]->read_dummy(reader);
    }
    reader.variant_end();
  }

  std::vector<std::unique_ptr<PolyTypeInterface<Base>>> interfaces_;

  // Used for variant_begin()
  std::vector<std::string> labels_;
  std::vector<const char*> labels_cstr_;
};

extern std::unordered_map<std::type_index, std::unique_ptr<PolyInterfacesBase>> poly_interfaces_;

template <typename Base>
inline PolyInterfaces<Base>* get_poly_interfaces() {
  auto iter = poly_interfaces_.find(std::type_index(typeid(Base)));
  if (iter == poly_interfaces_.end()) {
    iter = poly_interfaces_
               .emplace(std::type_index(typeid(Base)), std::make_unique<PolyInterfaces<Base>>())
               .first;
  }
  return dynamic_cast<PolyInterfaces<Base>*>(iter->second.get());
}

template <typename Base, supported Impl>
void register_smart_ptr(const std::string& label) {
  get_poly_interfaces<Base>()->template add<Impl>(label);
}

template <typename Base>
void write(Writer& writer, const std::unique_ptr<Base>& value) {
  get_poly_interfaces<Base>()->write(writer, value);
}

template <typename Base>
void read(Reader& reader, std::unique_ptr<Base>& value) {
  get_poly_interfaces<Base>()->read(reader, value);
}

template <typename Base>
void write(Writer& writer, const std::shared_ptr<Base>& value) {
  get_poly_interfaces<Base>()->write(writer, value);
}

template <typename Base>
void read(Reader& reader, std::shared_ptr<Base>& value) {
  get_poly_interfaces<Base>()->read(reader, value);
}

} // namespace datapack
