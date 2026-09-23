#include "datapack/schema/schema.hpp"
#include <assert.h>

#include <cstdint>
#include <stack>
#include <string_view>

namespace dpack {

Schema::Iterator Schema::Iterator::next() const {
  return Iterator(schema, index + 1);
}

Schema::Iterator Schema::Iterator::skip() const {
  int depth = 0;
  std::size_t skip_index = index;
  bool at_start = false;
  while (skip_index < schema->tokens.size()) {
    const auto& token = schema->tokens[skip_index];
    bool is_wrapper = false;
    if (std::get_if<token::ObjectBegin>(&token)) {
      depth++;
    } else if (std::get_if<token::ObjectEnd>(&token)) {
      depth--;
    } else if (std::get_if<token::TupleBegin>(&token)) {
      depth++;
    } else if (std::get_if<token::TupleEnd>(&token)) {
      depth--;
    } else if (std::get_if<token::VariantBegin>(&token)) {
      depth++;
    } else if (std::get_if<token::VariantEnd>(&token)) {
      depth--;
    } else if (std::get_if<token::List>(&token)) {
      is_wrapper = true;
    } else if (std::get_if<token::Optional>(&token)) {
      is_wrapper = true;
    }
    skip_index++;

    if (depth == 0 && is_wrapper) {
      continue;
    }
    if (depth == 0 && !(at_start && is_wrapper)) {
      break;
    }
    if (depth > 0) {
      at_start = false;
    }
  }
  return Iterator(schema, skip_index);
}

void Schema::apply(Reader& reader, Writer& writer) const {
  std::stack<Iterator> stack;
  std::stack<size_t> list_remaining;

  for (auto iter = begin(); iter != end(); iter = iter.next()) {
    while (!stack.empty()) {
      auto parent = stack.top();
      if (parent.object_begin()) {
        if (iter.object_end()) {
          reader.object_end();
          writer.object_end();
          stack.pop();
          iter = iter.next();
          continue;
        }
        auto object_next = iter.object_next();
        if (!object_next) {
          throw SchemaError("Expected ObjectNext token");
        }
        reader.object_next(object_next->key.c_str());
        writer.object_next(object_next->key.c_str());

        iter = iter.next();
        if (iter == end()) {
          throw SchemaError("Expected a valid token after ObjectNext");
        }
        break;
      }

      if (parent.tuple_begin()) {
        if (iter.tuple_end()) {
          reader.tuple_end();
          writer.tuple_end();
          stack.pop();
          iter = iter.next();
          continue;
        }
        if (!iter.tuple_next()) {
          throw SchemaError("Expected TupleNext token");
        }
        reader.tuple_next();
        writer.tuple_next();

        iter = iter.next();
        if (iter == end()) {
          throw SchemaError("Expected a valid token after TupleNext");
        }
        break;
      }

      if (parent.list()) {
        if (list_remaining.empty()) {
          throw std::runtime_error("Shouldn't happen");
        }
        if (list_remaining.top() == 0) {
          reader.list_end();
          writer.list_end();
          list_remaining.pop();
          stack.pop();
          continue;
        }
        list_remaining.top()--;
        reader.list_next();
        writer.list_next();
        iter = parent.next();
        break;
      }

      if (parent.optional()) {
        if (iter != parent.next()) {
          assert(iter == parent.skip());
          reader.optional_end();
          writer.optional_end();
          stack.pop();
          continue;
        }
        break;
      }

      if (parent.variant_next()) {
        if (iter != parent.next()) {
          while (iter != end()) {
            if (iter.variant_end()) {
              break;
            }
            if (!iter.variant_next()) {
              throw SchemaError("Expected VariantNext");
            }
            iter = iter.next().skip();
          }
          reader.variant_end();
          writer.variant_end();
          stack.pop();
          iter = iter.next();
          continue;
        }
        break;
      }
      assert(false);
    }
    if (iter == end()) {
      break;
    }

    if (auto object_begin = iter.object_begin()) {
      reader.object_begin(object_begin->debug_name);
      writer.object_begin(object_begin->debug_name);
      stack.push(iter);
      continue;
    }
    if (iter.tuple_begin()) {
      reader.tuple_begin();
      writer.tuple_begin();
      stack.push(iter);
      continue;
    }
    if (iter.list()) {
      const size_t size = reader.list_begin();
      list_remaining.push(size);
      writer.list_begin(size);
      stack.push(iter);
      continue;
    }
    if (iter.optional()) {
      bool has_value = reader.optional_begin();
      writer.optional_begin(has_value);
      if (has_value) {
        stack.push(iter);
      }
      continue;
    }
    if (auto variant_begin = iter.variant_begin()) {
      std::vector<std::string_view> labels;
      for (const auto& label : variant_begin->labels) {
        labels.push_back(label);
      }
      int choice = reader.variant_begin(labels);
      if (choice < 0 || (size_t)choice >= labels.size()) {
        throw SchemaError("Variant index is out of range");
      }
      writer.variant_begin(choice, labels);

      // Don't push VariantBegin

      iter = iter.next();
      while (iter != end()) {
        auto variant_next = iter.variant_next();
        if (!variant_next) {
          throw SchemaError("Expected VariantNext");
        }
        if (variant_next->index == choice) {
          break;
        }
        iter = iter.next().skip();
      }

      if (iter == end()) {
        throw SchemaError("Failed to find a valid VariantNext token");
      }
      // Push VariantNext
      stack.push(iter);

      continue;
    }

    if (auto number = iter.number()) {
      char buffer[8];
      reader.number(number->type, (void*)buffer);
      writer.number(number->type, (void*)buffer);
      continue;
    }
    if (iter.boolean()) {
      writer.boolean(reader.boolean());
      continue;
    }
    if (iter.string()) {
      writer.string(reader.string());
      continue;
    }
    if (auto enumerate = iter.enumerate()) {
      std::vector<std::string_view> labels;
      for (const auto& label : enumerate->labels) {
        labels.push_back(label);
      }
      const int value = reader.enumerate(labels);
      if (value < 0 || (size_t)value >= labels.size()) {
        throw SchemaError("Enum index is out of range");
      }
      writer.enumerate(value, labels);
      continue;
    }
    if (iter.binary()) {
      writer.binary(reader.binary());
      continue;
    }

    throw SchemaError("Unexpected token");
  }
}

namespace {

// FNV-1a 64. Chosen over std::hash because the hash is persisted in .dpack files:
// it must be fully specified by us, stable across toolchains, order sensitive and
// unable to cancel out repeated contributions.
constexpr std::uint64_t FNV_OFFSET_BASIS = 14695981039346656037ull;
constexpr std::uint64_t FNV_PRIME = 1099511628211ull;

void hash_byte(std::uint64_t& hash, std::uint8_t byte) {
  hash ^= byte;
  hash *= FNV_PRIME;
}

void hash_integer(std::uint64_t& hash, std::uint64_t value) {
  // Little-endian, so the hash doesn't depend on the host's byte order
  for (int i = 0; i < 8; i++) {
    hash_byte(hash, (std::uint8_t)(value >> (i * 8)));
  }
}

void hash_string(std::uint64_t& hash, const std::string& value) {
  // Length-prefixed, so {"ab", "c"} and {"a", "bc"} hash differently
  hash_integer(hash, value.size());
  for (char c : value) {
    hash_byte(hash, (std::uint8_t)c);
  }
}

} // namespace

void Schema::set_hash() {
  // The hash identifies the structure of the data: token kinds, number types, enum and
  // variant labels, object keys and variant indexes. ObjectBegin::debug_name,
  // token::Hint and token::Description are metadata, so they are excluded here (and
  // from operator==), meaning an edit to a hint or a description doesn't invalidate
  // already stored files.
  //
  // Starting from FNV_OFFSET_BASIS rather than 0 means an empty schema still has a
  // non-zero hash, so it isn't confused with the pre-set_hash state of hash_.
  hash_ = FNV_OFFSET_BASIS;
  for (const auto& token : tokens) {
    hash_integer(hash_, token.index());

    if (auto number = std::get_if<token::Number>(&token)) {
      hash_integer(hash_, (std::uint64_t)number->type);

    } else if (auto enumerate = std::get_if<token::Enumerate>(&token)) {
      // Hash the count first, so an empty label list differs from an absent one
      hash_integer(hash_, enumerate->labels.size());
      for (const auto& label : enumerate->labels) {
        hash_string(hash_, label);
      }

    } else if (auto variant_begin = std::get_if<token::VariantBegin>(&token)) {
      hash_integer(hash_, variant_begin->labels.size());
      for (const auto& label : variant_begin->labels) {
        hash_string(hash_, label);
      }

    } else if (auto variant_next = std::get_if<token::VariantNext>(&token)) {
      hash_integer(hash_, (std::uint64_t)(std::int64_t)variant_next->index);

    } else if (auto object_next = std::get_if<token::ObjectNext>(&token)) {
      hash_string(hash_, object_next->key);
    }
  }
}

bool operator==(const Schema& lhs, const Schema& rhs) {
  if (lhs.tokens.size() != rhs.tokens.size()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.tokens.size(); i++) {
    if (lhs.tokens[i] != rhs.tokens[i]) {
      return false;
    }
  }
  return true;
}

void Schema::read(::dpack::Reader& packer) {
  packer.object_begin(::dpack::type_name<Schema>());
  packer.value("tokens", tokens);
  packer.object_end();
  set_hash();
}

void Schema::write(::dpack::Writer& packer) const {
  packer.object_begin(::dpack::type_name<Schema>());
  packer.value("tokens", tokens);
  packer.object_end();
}

} // namespace dpack
