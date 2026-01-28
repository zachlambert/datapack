#pragma once

#include "datapack/datapack.hpp"
#include "datapack/schema/token.hpp"
#include "datapack/schema/tokenizer.hpp"
#include <stdexcept>

namespace datapack {

class SchemaError : public std::runtime_error {
public:
  SchemaError(const std::string& message) : std::runtime_error(message) {}
};

class Schema {
public:
  void apply(Reader& reader, Writer& writer);
  void apply(Reader&& reader, Writer&& writer) {
    apply(reader, writer);
  }

  class Iterator {
  public:
    Iterator() : schema(nullptr), index(0) {}

    const token::Number* number() const {
      return std::get_if<token::Number>(token());
    }
    bool boolean() const {
      return std::get_if<token::Boolean>(token());
    }
    bool string() const {
      return std::get_if<token::String>(token());
    }
    const token::Enumerate* enumerate() const {
      return std::get_if<token::Enumerate>(token());
    }
    bool binary() const {
      return std::get_if<token::Binary>(token());
    }

    bool optional() const {
      return std::get_if<token::Optional>(token());
    }
    const token::VariantBegin* variant_begin() const {
      return std::get_if<token::VariantBegin>(token());
    }
    const token::VariantNext* variant_next() const {
      return std::get_if<token::VariantNext>(token());
    }
    bool variant_end() const {
      return std::get_if<token::VariantEnd>(token());
    }

    const token::ObjectBegin* object_begin() const {
      return std::get_if<token::ObjectBegin>(token());
    }
    const token::ObjectNext* object_next() const {
      return std::get_if<token::ObjectNext>(token());
    }
    bool object_end() const {
      return std::get_if<token::ObjectEnd>(token());
    }

    bool tuple_begin() const {
      return std::get_if<token::TupleBegin>(token());
    }
    bool tuple_next() const {
      return std::get_if<token::TupleNext>(token());
    }
    bool tuple_end() const {
      return std::get_if<token::TupleEnd>(token());
    }

    bool list() const {
      return std::get_if<token::List>(token());
    }

    // Next immediate token
    Iterator next() const;

    // Skips to next token at the same depth
    Iterator skip() const;

    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
      return lhs.index == rhs.index;
    }

  private:
    Iterator(const Schema* schema, std::size_t index) : schema(schema), index(index) {}

    const Token* token() const {
      return &schema->tokens[index];
    }

    const Schema* schema;
    std::size_t index;
    friend class Schema;
  };

  Iterator iter() const {
    return Iterator(this, 0);
  }

  template <typename T>
  static Schema make() {
    T dummy;
    Schema result;
    Tokenizer(result.tokens).value(dummy);
    return result;
  }

  static Schema from_tokens(const std::vector<Token>& tokens) {
    Schema result;
    result.tokens = tokens;
    return result;
  }

  Iterator begin() const {
    return Iterator(this, 0);
  }
  Iterator end() const {
    return Iterator(this, tokens.size());
  }

  std::uint64_t hash() const {
    std::uint64_t hash = 0;
    for (const auto& token : tokens) {
      hash = hash ^ std::hash<size_t>{}(token.index());
      if (auto number = std::get_if<token::Number>(&token)) {
        hash = hash ^ std::hash<int>{}((int)number->type);
        if (number->constraint) {
          hash = hash ^ std::hash<size_t>{}(number->constraint->index());
          if (auto constraint = std::get_if<ConstraintNumberRange>(&*number->constraint)) {
            hash = hash ^ std::hash<double>{}(constraint->lower);
            hash = hash ^ std::hash<double>{}(constraint->upper);
          }
        }
      } else if (auto enumerate = std::get_if<token::Enumerate>(&token)) {
        for (const auto& label: enumerate->labels) {
          hash = hash ^ std::hash<std::string>{}(label);
        }
      } else if (auto variant_begin = std::get_if<token::VariantBegin>(&token)) {
        for (const auto& label: variant_begin->labels) {
          hash = hash ^ std::hash<std::string>{}(label);
        }
      } else if (auto variant_next = std::get_if<token::VariantNext>(&token)) {
        hash = hash ^ std::hash<int>{}(variant_next->index);
      } else if (auto object_begin = std::get_if<token::ObjectBegin>(&token)) {
        if (object_begin->constraint) {
          hash = hash ^ std::hash<size_t>{}(object_begin->constraint->index());
        }
      } else if (auto object_next = std::get_if<token::ObjectNext>(&token)) {
        hash = hash ^ std::hash<std::string>{}(object_next->key);
      }
    }
    return hash;
  }

  DATAPACK_CLASS_DECL();
private:
  std::vector<Token> tokens;

  friend bool operator==(const Schema& lhs, const Schema& rhs);
};

template <typename T>
std::uint64_t get_hash() {
  static std::uint64_t hash = 0;
  static bool calculated = false;
  if (!calculated) {
    calculated = true;
    hash = Schema::make<T>().hash();
  }
  return hash;
}

} // namespace datapack
