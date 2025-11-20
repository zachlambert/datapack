#include "datapack/schema/schema.hpp"
#include "datapack/std/string.hpp"
#include "datapack/std/variant.hpp"
#include "datapack/std/vector.hpp"
#include <assert.h>

#include <stack>

namespace datapack {

Schema::Iterator Schema::Iterator::next() const {
  return Iterator(schema, index + 1);
}

Schema::Iterator Schema::Iterator::skip() const {
  int depth = 0;
  std::size_t skip_index = index;
  while (skip_index < schema->tokens.size()) {
    const auto& token = schema->tokens[skip_index];
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
    }
    skip_index++;
    if (depth == 0) {
      break;
    }
  }
  return Iterator(schema, skip_index);
}

void Schema::apply(Reader& reader, Writer& writer) {
  std::stack<Iterator> stack;

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
        if (!reader.list_next()) {
          reader.list_end();
          writer.list_end();
          stack.pop();
          continue;
        }
        writer.list_next();
        iter = parent.next();
        break;
      }

      if (parent.optional()) {
        if (iter != parent.next()) {
          assert(iter == parent.next().skip());
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

    if (iter.object_begin()) {
      reader.object_begin();
      writer.object_begin();
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
      reader.list_begin();
      writer.list_begin();
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
      std::vector<const char*> labels_c_str;
      for (const auto& label : variant_begin->labels) {
        labels_c_str.push_back(label.c_str());
      }
      int choice = reader.variant_begin(labels_c_str);
      writer.variant_begin(choice, labels_c_str);

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
      std::vector<const char*> labels_c_str;
      for (const auto& label : enumerate->labels) {
        labels_c_str.push_back(label.c_str());
      }
      writer.enumerate(reader.enumerate(labels_c_str), labels_c_str);
      continue;
    }
    if (iter.binary()) {
      writer.binary(reader.binary());
      continue;
    }

    throw SchemaError("Unexpected token");
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

DATAPACK_IMPL(Schema, value, packer) {
  packer.object_begin();
  packer.value("tokens", value.tokens);
  packer.object_end();
}

} // namespace datapack
