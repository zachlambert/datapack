#pragma once

#include "datapack/binary.hpp"
#include "datapack/schema/schema.hpp"
#include <fstream>
#include <string>
#include <unordered_map>

namespace datapack {

class FileWriter {
public:
  class TypeError : std::exception {
  protected:
    const char* what() const noexcept override {
      return "Tried to write value with incorrect type";
    }
  };

  FileWriter(const std::string& path);
  void close();

  template <typename T>
  requires writeable<T>
  void write(const std::string& label, const T& value) {
    auto iter = label_hashes.find(label);
    if (iter == label_hashes.end()) {
      iter = label_hashes.emplace(label, get_hash<T>()).first;
    } else if (iter->second != get_hash<T>()) {
      throw TypeError();
    }
    write_chunk(label, iter->second, write_binary(value));
  }

private:
  void write_chunk(
      const std::string& label,
      std::uint64_t hash,
      const std::vector<std::uint8_t>& data);

  std::ofstream os;
  std::unordered_map<std::string, std::uint64_t> label_hashes;
};

class FileReader {
public:
  class TypeError : std::exception {
  protected:
    const char* what() const noexcept override {
      return "Tried to read value with incorrect type";
    }
  };
  class FileError : std::exception {
  protected:
    const char* what() const noexcept override {
      return "Unexpected file format";
    }
  };

  FileReader(const std::string& path);
  void close();

  std::optional<std::string> next();

  template <typename T>
  requires readable<T>
  T read() {
    auto [hash, bytes] = read_chunk_remainder();
    if (hash != get_hash<T>()) {
      throw TypeError();
    }
    auto iter = label_hashes.find(current_label);
    if (iter == label_hashes.end()) {
      label_hashes.emplace(current_label, hash);
    } else if (iter->second != hash) {
      throw FileError();
    }
    return read_binary<T>(bytes);
  }

  void skip();

private:
  std::tuple<std::uint64_t, std::vector<std::uint8_t>> read_chunk_remainder();

  std::ifstream is;
  std::string current_label;
  std::unordered_map<std::string, std::uint64_t> label_hashes;
};

} // namespace datapack
