#include "datapack/schema/schema.hpp"
#include "datapack/std/string.hpp"
#include "datapack/std/variant.hpp"
#include "datapack/std/vector.hpp"
#include <stdexcept>

#include <stack>

namespace datapack {

static std::size_t get_tokens_end(const std::vector<Token>& tokens, std::size_t begin) {
  std::size_t pos = begin;
  std::size_t depth = 0;
  while (true) {
    if (depth == 0 && pos != begin) {
      break;
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("Invalid binary schema");
    }
    const auto& token = tokens[pos];
    pos++;

    // All these tokens are preceded by another value
    // so skip to the next token, such that the loop doesn't exit
    // if the depth is still zero
    if (std::get_if<token::List>(&token)) {
      continue;
    } else if (std::get_if<token::Optional>(&token)) {
      continue;
    }
    // Explicit container tokens, that increment or decrement depth
    // Where depth is decreased, fall through to the end of the loop
    // body to check if depth is zero
    else if (std::get_if<token::ObjectBegin>(&token)) {
      depth++;
      continue;
    } else if (std::get_if<token::ObjectEnd>(&token)) {
      depth--;
    } else if (std::get_if<token::TupleBegin>(&token)) {
      depth++;
      continue;
    } else if (std::get_if<token::TupleEnd>(&token)) {
      depth--;
    } else if (std::get_if<token::VariantBegin>(&token)) {
      depth++;
      continue;
    } else if (std::get_if<token::VariantEnd>(&token)) {
      depth--;
    }
    // Remaining tokens are values
    // Either these are in a container and depth remains unchanged
    // at a non-zero value, so continues, or they are the only value
    // and depth is still at zero, which stops the loop

    if (depth == 0) {
      break;
    }
  }
  return pos;
}

void use_schema(const Schema& schema, Reader& reader, Writer& writer) {
  enum class StateType { None, List, Array, Optional, Variant };
  struct State {
    const StateType type;
    const std::size_t value_tokens_begin;
    const std::size_t value_tokens_end;
    int remaining;
    State(
        StateType type,
        std::size_t value_tokens_begin,
        std::size_t value_tokens_end,
        int remaining) :
        type(type),
        value_tokens_begin(value_tokens_begin),
        value_tokens_end(value_tokens_end),
        remaining(remaining) {}
  };
  std::stack<State> states;
  states.push(State(StateType::None, 0, 0, 0));

  std::size_t token_pos = 0;
  while (true) {
    if (token_pos == schema.tokens.size()) {
      break;
    }

    auto& state = states.top();
    if (state.type == StateType::List) {
      if (!reader.list_next()) {
        reader.list_end();
        writer.list_end();
        token_pos = state.value_tokens_end;
        states.pop();
        continue;
      }
      writer.list_next();
      token_pos = state.value_tokens_begin;
    } else if (state.type == StateType::Optional) {
      bool has_value = false;
      if (state.remaining) {
        has_value = reader.optional_begin();
        writer.optional_begin(has_value);
        state.remaining = 0;
      }
      if (!has_value) {
        reader.optional_end();
        writer.optional_end();
        token_pos = state.value_tokens_end;
        states.pop();
        continue;
      }
      token_pos = state.value_tokens_begin;
    } else if (state.type == StateType::Variant) {
      if (!state.remaining) {
        reader.variant_end();
        writer.variant_end();
        token_pos = state.value_tokens_end;
        states.pop();
        continue;
      }
      state.remaining = 0;
      token_pos = state.value_tokens_begin;
      // Fall-through to processing value below
    } else if (state.type == StateType::Array) {
      if (state.remaining == 0) {
        token_pos = state.value_tokens_end;
        states.pop();
        reader.list_end();
        writer.list_end();
        token_pos++;
        continue;
      }
      writer.list_next();
      state.remaining--;
      token_pos = state.value_tokens_begin;
      // Fall-through to processing value below
    }

    const auto& token = schema.tokens[token_pos];
    token_pos++;

    if (std::get_if<token::ObjectBegin>(&token)) {
      states.push(State(StateType::None, 0, 0, 0));
      reader.object_begin();
      writer.object_begin();
      continue;
    }
    if (std::get_if<token::ObjectEnd>(&token)) {
      states.pop();
      reader.object_end();
      writer.object_end();
      continue;
    }
    if (auto value = std::get_if<token::ObjectNext>(&token)) {
      reader.object_next(value->key.c_str());
      writer.object_next(value->key.c_str());
      continue;
    }

    if (std::get_if<token::TupleBegin>(&token)) {
      states.push(State(StateType::None, 0, 0, 0));
      reader.tuple_begin();
      writer.tuple_begin();
      continue;
    }
    if (std::get_if<token::TupleEnd>(&token)) {
      states.pop();
      reader.tuple_end();
      writer.tuple_end();
      continue;
    }
    if (std::get_if<token::TupleNext>(&token)) {
      reader.tuple_next();
      writer.tuple_next();
      continue;
    }

    if (std::get_if<token::List>(&token)) {
      reader.list_begin();
      writer.list_begin();

      states.push(State(StateType::List, token_pos, get_tokens_end(schema.tokens, token_pos), 0));
      continue;
    }

    if (std::get_if<token::Optional>(&token)) {
      // Call writer.optional() elsewhere
      states.push(
          State(StateType::Optional, token_pos, get_tokens_end(schema.tokens, token_pos), 1));
      continue;
    }
    if (auto variant = std::get_if<token::VariantBegin>(&token)) {
      std::vector<const char*> labels_cstr;
      for (const auto& label : variant->labels) {
        labels_cstr.push_back(label.c_str());
      }

      int variant_index = reader.variant_begin(labels_cstr);

      bool found_match = false;
      std::size_t variant_start;
      // Move token_pos forward to the corresponding variant value
      while (true) {
        const auto& token = schema.tokens[token_pos];
        token_pos++;
        if (auto value = std::get_if<token::VariantNext>(&token)) {
          if (value->index == variant_index) {
            if (found_match) {
              throw std::runtime_error("Repeated variant labels");
            }
            found_match = true;
            variant_start = token_pos;
            writer.variant_begin(variant_index, labels_cstr);
          }
          token_pos = get_tokens_end(schema.tokens, token_pos);
          continue;
        } else if (auto value = std::get_if<token::VariantEnd>(&token)) {
          break;
        }
        throw std::runtime_error("Invalid binary schema");
      }
      if (!found_match) {
        throw std::runtime_error("No matching variant");
      }

      states.push(State(StateType::Variant, variant_start, token_pos, 1));
      continue;
    }

    if (auto x = std::get_if<NumberType>(&token)) {
      std::uint8_t dummy_buffer[8]; // Large enough for all number types
      reader.number(*x, dummy_buffer);
      writer.number(*x, dummy_buffer);
    } else if (std::get_if<bool>(&token)) {
      writer.boolean(reader.boolean());
    } else if (std::get_if<std::string>(&token)) {
      std::string value;
      const char* string = reader.string();
      writer.string(string);
    } else if (auto value = std::get_if<token::Enumerate>(&token)) {
      std::vector<const char*> labels_cstr;
      for (const auto& label : value->labels) {
        labels_cstr.push_back(label.c_str());
      }
      int enum_value = reader.enumerate(labels_cstr);
      writer.enumerate(enum_value, labels_cstr);
    } else if (auto value = std::get_if<token::Binary>(&token)) {
      auto data = reader.binary();
      writer.binary(data);
    } else {
      throw std::runtime_error("Shouldn't be here");
    }
  }
}

bool operator==(const Schema& lhs, const Schema& rhs) { return lhs == rhs; }

DATAPACK_IMPL(Schema, value, packer) {
  packer.object_begin();
  packer.value("tokens", value.tokens);
  packer.object_end();
}

} // namespace datapack
