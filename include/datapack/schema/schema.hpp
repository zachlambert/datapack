#pragma once

#include "datapack/datapack.hpp"
#include "datapack/schema/token.hpp"
#include "datapack/schema/tokenizer.hpp"

namespace datapack {

class Schema {
public:
  void apply(Reader& reader, Writer& writer);
  void apply(Reader&& reader, Writer&& writer) {
    apply(reader, writer);
  }

  class Iterator {
  public:
    Iterator() : schema(nullptr), index(0) {}

    const Token& operator*() const {
      return schema->tokens[index];
    }

    // Next immediate token
    Iterator next() const;

    // Skips to next token at the same depth
    Iterator skip() const;

    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
      return lhs.index == rhs.index;
    }

  private:
    Iterator(const Schema* schema, std::size_t index) : schema(schema) {}
    const Schema* schema;
    std::size_t index;
    friend class Schema;
  };

  Iterator iter() const {
    return Iterator(this, 0);
  }

  template <typename T>
  static Schema Make() {
    T dummy;
    Schema result;
    Tokenizer(result.tokens).value(dummy);
    result.init_depth();
    return result;
  }

  Iterator begin() const {
    return Iterator(this, 0);
  }
  Iterator end() const {
    return Iterator(this, tokens.size());
  }

private:
  void init_depth();

  std::vector<Token> tokens;
  std::vector<int> depth;

  DATAPACK_FRIEND(Schema);
  friend bool operator==(const Schema& lhs, const Schema& rhs);
};

DATAPACK(Schema);

} // namespace datapack
