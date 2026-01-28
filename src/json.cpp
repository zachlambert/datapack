#include "datapack/json.hpp"
#include "datapack/encode/base64.hpp"
#include "datapack/encode/floating_string.hpp"
#include <assert.h>

namespace datapack {

Object load_json(const std::string& json) {
  static constexpr int EXPECT_ELEMENT = 1 << 0;
  static constexpr int EXPECT_VALUE = 1 << 1;
  static constexpr int EXPECT_END = 1 << 2;
  static constexpr int EXPECT_NEXT = 1 << 3;
  static constexpr int IS_OBJECT = 1 << 4;
  static constexpr int IS_ARRAY = 1 << 5;

  std::size_t pos = 0;
  std::stack<int> states;
  states.push(EXPECT_VALUE);
  Object object;
  Object::Ptr ptr = object.ptr();

  while (true) {
    int& state = states.top();

    if (pos == json.size()) {
      if (!(state & EXPECT_END) || (state & IS_OBJECT) || (state & IS_ARRAY)) {
        throw JsonLoadError("Not expecting the document end");
      }
      assert(!ptr);
      assert(states.size() == 1);
      break;
    }

    const char c = json[pos];
    if (std::isspace(c)) {
      pos++;
      continue;
    }
    assert(ptr);

    if (c == '"' && (state & IS_OBJECT) && (state & EXPECT_ELEMENT)) {
      pos++;
      std::size_t begin = pos;
      while (true) {
        if (pos == json.size()) {
          throw JsonLoadError("Key missing terminating '\"'");
        }
        const char c = json[pos];
        if (c == '"') {
          break;
        }
        pos++;
      }
      std::size_t end = pos;
      pos++;

      while (true) {
        if (pos == json.size()) {
          throw JsonLoadError("Expected ':' following key");
        }
        const char c = json[pos];
        pos++;
        if (std::isspace(c)) {
          continue;
        }
        if (c == ':') {
          break;
        }
        throw JsonLoadError("Expected ':' following key");
      }
      state &= ~EXPECT_ELEMENT;
      state |= EXPECT_VALUE;
      std::string key = json.substr(begin, end - begin);
      ptr = ptr->emplace(key).ptr();
      continue;
    }

    if (c != ']' && (state & IS_ARRAY) && (state & EXPECT_ELEMENT)) {
      state &= ~EXPECT_ELEMENT;
      state |= EXPECT_VALUE;
      ptr = ptr->emplace_back().ptr();
      continue;
    }

    if (c == '{') {
      if (!(state | EXPECT_VALUE)) {
        throw JsonLoadError("Unexpected character '{'");
      }
      pos++;
      states.push(IS_OBJECT | EXPECT_ELEMENT | EXPECT_END);
      ptr->to_map();
      continue;
    }
    if (c == '[') {
      if (!(state & EXPECT_VALUE)) {
        throw JsonLoadError("Unexpected character '['");
      }
      pos++;
      states.push(IS_ARRAY | EXPECT_ELEMENT | EXPECT_END);
      ptr->to_list();
      continue;
    }
    if (c == '}') {
      if (!(state & IS_OBJECT) || !(state & EXPECT_END)) {
        throw JsonLoadError("Unexpected character '}'");
      }
      pos++;
      states.pop();

      int& new_state = states.top();
      new_state &= ~EXPECT_VALUE;
      new_state |= EXPECT_NEXT | EXPECT_END;
      ptr = ptr.parent();
      continue;
    }
    if (c == ']') {
      if (!(state & IS_ARRAY) | !(state & EXPECT_END)) {
        throw JsonLoadError("Unexpected character ']'");
      }
      pos++;
      states.pop();

      int& new_state = states.top();
      new_state &= ~EXPECT_VALUE;
      new_state |= EXPECT_NEXT | EXPECT_END;
      ptr = ptr.parent();
      continue;
    }
    if (c == ',') {
      pos++;

      if (!(state & EXPECT_NEXT)) {
        throw JsonLoadError("Unexpected character ','");
      }
      state &= ~EXPECT_NEXT;
      state |= EXPECT_ELEMENT;
      continue;
    }

    if (!(state & EXPECT_VALUE)) {
      throw JsonLoadError("Not expecting a value");
    }

    state &= ~EXPECT_VALUE;
    state |= EXPECT_END | EXPECT_NEXT;

    if (c == '"') {
      pos++;
      std::size_t begin = pos;
      while (true) {
        if (pos == json.size()) {
          throw JsonLoadError("String missing terminating '\"'");
        }
        if (json[pos] == '"') {
          break;
        }
        pos++;
      }
      std::size_t end = pos;
      pos++;
      *ptr = json.substr(begin, end - begin);
      ptr = ptr.parent();
      continue;
    }

    std::size_t begin = pos;
    while (true) {
      if (pos == json.size()) {
        break;
      }
      const char c = json[pos];
      if (std::isspace(c) || c == ',' || c == '}' || c == ']') {
        break;
      }
      pos++;
    }
    std::size_t end = pos;

    std::string value = json.substr(begin, end - begin);
    if (value == "true") {
      *ptr = true;
      ptr = ptr.parent();
      continue;
    }
    if (value == "false") {
      *ptr = false;
      ptr = ptr.parent();
      continue;
    }
    if (value == "null") {
      ptr->to_null();
      ptr = ptr.parent();
      continue;
    }
    try {
      object::number_t result = std::strtod(value.c_str(), nullptr);
      *ptr = result;
      ptr = ptr.parent();
      continue;
    } catch (std::invalid_argument) {
    } catch (std::out_of_range) {
    }

    throw JsonLoadError("Invalid value '" + std::string(value) + "'");
  }

  return object;
}

std::string dump_json(ConstObject object) {
  std::string json = "";

  std::stack<ConstObject::Ptr> stack;
  stack.push(object.ptr());
  bool start = true;
  bool end = false;

  while (!stack.empty()) {
    auto ptr = stack.top();
    if (!ptr) {
      stack.pop();
      end = true;
      continue;
    }

    if (end) {
      end = false;
      if (!start) {
        json += "\n";
        for (int i = 0; i < (stack.size() - 1); i++) {
          json += "    ";
        }
      }
      start = false;
      if (ptr->is_map()) {
        json += "}";
        stack.pop();
        stack.push(ptr.next());
        continue;
      }
      if (ptr->is_list()) {
        json += "]";
        stack.pop();
        stack.push(ptr.next());
        continue;
      }
      stack.pop();
      assert(stack.empty());
      continue;
    }

    if (!start) {
      json += ",\n";
    } else if (!json.empty()) {
      json += "\n";
    }
    start = false;
    for (int i = 0; i < (stack.size() - 1); i++) {
      json += "    ";
    }

    if (!ptr.key().empty()) {
      json += "\"" + ptr.key() + "\": ";
    }

    if (ptr->is_map()) {
      json += "{";
      stack.push(ptr.child());
      start = true;
      continue;
    }
    if (ptr->is_list()) {
      json += "[";
      stack.push(ptr.child());
      start = true;
      continue;
    }

    if (auto value = ptr->number_if()) {
      json += floating_to_string(*value);
    } else if (auto value = ptr->string_if()) {
      json += "\"" + *value + "\"";
    } else if (ptr->is_null()) {
      json += "null";
    } else if (auto value = ptr->boolean_if()) {
      json += (*value ? "true" : "false");
    } else if (auto value = ptr->binary_if()) {
      json += "\"" + base64_encode(*value) + "\"";
    }

    stack.pop();
    stack.push(ptr.next());
  }

  return json;
}

} // namespace datapack
