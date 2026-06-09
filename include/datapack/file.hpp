#pragma once

#include "datapack/binary.hpp"
#include "datapack/object.hpp"
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
    check_hash(label, get_hash<T>());
    write_chunk(label, get_hash<T>(), to_binary(value));
  }

  void write_object(const std::string& label, const Object& object, const Schema& schema) {
    check_hash(label, schema.hash());

    ObjectReader reader(object);
    BinarySizeWriter size_writer;
    schema.apply(reader, size_writer);

    std::vector<std::uint8_t> buffer(size_writer.size());
    BinaryWriter binary_writer(buffer);
    schema.apply(reader, binary_writer);

    write_chunk(label, schema.hash(), buffer);
  }

private:
  void check_hash(const std::string& label, std::uint64_t hash);
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
    check_hash(hash, current_label, get_hash<T>());
    return from_binary<T>(bytes);
  }

  Object read_object(const std::string& label, const Schema& schema) {
    auto [hash, bytes] = read_chunk_remainder();
    check_hash(hash, current_label, schema.hash());

    BinaryReader reader(bytes);
    Object object;
    ObjectWriter writer(object);
    schema.apply(reader, writer);

    return object;
  }

  void skip();

private:
  void check_hash(std::uint64_t stored_hash, const std::string& label, std::uint64_t expected_hash);
  std::tuple<std::uint64_t, std::vector<std::uint8_t>> read_chunk_remainder();

  std::ifstream is;
  std::string current_label;
  std::unordered_map<std::string, std::uint64_t> label_hashes;
};

template <typename T>
void to_file(const std::string& path, const T& value) {
  FileWriter writer(path);
  writer.write("", value);
}

template <typename T>
T from_file(const std::string& path) {
  FileReader reader(path);
  auto label = reader.next();
  if (!label || *label != "") {
    throw FileReader::FileError();
  }
  return reader.read<T>();
}

} // namespace datapack
