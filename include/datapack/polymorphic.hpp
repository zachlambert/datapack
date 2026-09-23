#pragma once

#include "datapack/datapack.hpp"
#include "datapack/names/type_names.hpp"
#include <memory>
#include <stdexcept>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

/* Polymorphic types are serialized like a variant, with the registered implementations taking the
 * place of the alternatives, so their labels are calculated the same way: the label of an
 * implementation is its type label, customised with DPACK_TYPE_NAMES on the type itself.
 *
 * Unlike a variant, the implementations are registered at runtime rather than being known from the
 * type, so the labels cannot be collected into an array up front and uniqueness cannot be checked
 * with a static_assert. Both happen on registration instead.
 */

namespace dpack {

class PolyError : public std::runtime_error {
public:
  PolyError(const std::string& message) : std::runtime_error(message) {}
};

// Specialise this to register implementations that are always available for Base, most easily with
// DPACK_POLY_DEFAULTS. Called once, the first time Base is serialized
template <typename T>
void register_polymorphic_defaults() {}

namespace detail {

template <typename Base>
class PolyTypeInterface {
public:
  PolyTypeInterface(std::string_view label, std::type_index type, int index) :
      label_(label), type_(type), index_(index) {}

  // NOTE: Safe to store a view, since type labels have static storage duration
  std::string_view label() const {
    return label_;
  }
  std::type_index type() const {
    return type_;
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
  std::string_view label_;
  std::type_index type_;
  int index_;
};

template <typename Base, typename Child>
class PolyTypeImpl : public PolyTypeInterface<Base> {
public:
  PolyTypeImpl(int index) :
      PolyTypeInterface<Base>(type_label<Child>(), std::type_index(typeid(Child)), index) {}

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
  void add() {
    const std::string_view label = type_label<Child>();
    const std::type_index type(typeid(Child));
    for (const auto& interface : interfaces_) {
      if (interface->type() == type) {
        return; // Already registered
      }
      if (interface->label() == label) {
        throw PolyError(
            "Polymorphic base has duplicate labels, '" + std::string(label) +
            "' is already registered by another type");
      }
    }
    interfaces_.push_back(std::make_unique<PolyTypeImpl<Base, Child>>(interfaces_.size()));
    labels_.push_back(label);
  }

  void write(Writer& writer, const std::unique_ptr<Base>& value) {
    auto interface = get(value.get());
    writer.variant_begin(interface->index(), labels_);
    interface->write(writer, value);
    writer.variant_end();
  }

  void write(Writer& writer, const std::shared_ptr<Base>& value) {
    auto interface = get(value.get());
    writer.variant_begin(interface->index(), labels_);
    interface->write(writer, value);
    writer.variant_end();
  }

  void read(Reader& reader, std::unique_ptr<Base>& value) {
    if (reader.is_tokenizer()) {
      tokenize(reader);
      return;
    }
    const int index = reader.variant_begin(labels_);
    auto interface = get(index);
    interface->read(reader, value);
    reader.variant_end();
  }

  void read(Reader& reader, std::shared_ptr<Base>& value) {
    if (reader.is_tokenizer()) {
      tokenize(reader);
      return;
    }
    const int index = reader.variant_begin(labels_);
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
    throw PolyError("Missing polymorphic registration");
    return nullptr;
  }

  const PolyTypeInterface<Base>* get(int index) {
    if (index < 0 || (size_t)index >= interfaces_.size()) {
      throw PolyError("Missing polymorphic registration");
    }
    return interfaces_[index].get();
  }

  void tokenize(Reader& reader) {
    reader.variant_begin(labels_);
    for (int i = 0; i < interfaces_.size(); i++) {
      reader.variant_tokenize(i);
      interfaces_[i]->read_dummy(reader);
    }
    reader.variant_end();
  }

  std::vector<std::unique_ptr<PolyTypeInterface<Base>>> interfaces_;

  // Used for variant_begin()
  std::vector<std::string_view> labels_;
};

extern std::unordered_map<std::type_index, std::unique_ptr<PolyInterfacesBase>> poly_interfaces_;

template <typename Base>
inline PolyInterfaces<Base>* get_poly_interfaces() {
  auto iter = poly_interfaces_.find(std::type_index(typeid(Base)));
  if (iter == poly_interfaces_.end()) {
    iter = poly_interfaces_
               .emplace(std::type_index(typeid(Base)), std::make_unique<PolyInterfaces<Base>>())
               .first;
    register_polymorphic_defaults<Base>();
  }
  return dynamic_cast<PolyInterfaces<Base>*>(iter->second.get());
}

} // namespace detail

template <typename Base, serializable Impl>
void register_polymorphic() {
  detail::get_poly_interfaces<Base>()->template add<Impl>();
}

// Defines the register_polymorphic_defaults specialisation for Base, registering each of the
// listed implementations. Must be used at global scope
#define _DPACK_POLY_REGISTER(X) ::dpack::register_polymorphic<_DpackPolyBase, X>();

#define DPACK_POLY_DEFAULTS(Base, ...)                                                             \
  template <>                                                                                      \
  inline void dpack::register_polymorphic_defaults<Base>() {                                       \
    using _DpackPolyBase = Base;                                                                   \
    _DPACK_FOR_EACH(_DPACK_POLY_REGISTER, __VA_ARGS__)                                             \
  }

template <typename Base>
void write(Writer& writer, const std::unique_ptr<Base>& value) {
  detail::get_poly_interfaces<Base>()->write(writer, value);
}

template <typename Base>
void read(Reader& reader, std::unique_ptr<Base>& value) {
  detail::get_poly_interfaces<Base>()->read(reader, value);
}

template <typename Base>
void write(Writer& writer, const std::shared_ptr<Base>& value) {
  detail::get_poly_interfaces<Base>()->write(writer, value);
}

template <typename Base>
void read(Reader& reader, std::shared_ptr<Base>& value) {
  detail::get_poly_interfaces<Base>()->read(reader, value);
}

} // namespace dpack
