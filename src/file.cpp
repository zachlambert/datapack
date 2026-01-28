#include "datapack/file.hpp"
#include <cstring>

namespace datapack {

static const char* SPECIAL = "DATAPACK";

FileWriter::FileWriter(const std::string& path) : os(path, std::ios_base::binary) {
  os << SPECIAL;
}

void FileWriter::close() {
  if (os) {
    os.close();
  }
}

void FileWriter::write_chunk(
    const std::string& label,
    std::uint64_t hash,
    const std::vector<std::uint8_t>& data) {
  // Write label
  std::uint32_t label_size = label.size();
  os.write((const char*)&label_size, sizeof(label_size));
  os.write(label.data(), label.size());

  // Write hash
  os.write((const char*)&hash, sizeof(hash));

  // Write data
  std::uint64_t data_size = data.size();
  os.write((const char*)&data_size, sizeof(data_size));
  os.write((const char*)data.data(), data.size());
}

FileReader::FileReader(const std::string& path) : is(path, std::ios_base::binary) {
  std::string special_buff = SPECIAL;
  is.read(special_buff.data(), 8);
  if (special_buff != SPECIAL) {
    throw FileError();
  }
}

void FileReader::close() {
  if (is) {
    is.close();
  }
}

std::optional<std::string> FileReader::next() {
  is.peek(); // Required to check for eof
  if (is.eof()) {
    return std::nullopt;
  }
  std::uint32_t label_size;
  if (!is.read((char*)&label_size, sizeof(label_size))) {
    throw FileError();
  }
  current_label.resize(label_size);
  if (!is.read(current_label.data(), current_label.size())) {
    throw FileError();
  }
  return current_label;
}

std::tuple<std::uint64_t, std::vector<std::uint8_t>> FileReader::read_chunk_remainder() {
  std::uint64_t hash;
  if (!is.read((char*)&hash, sizeof(hash))) {
    throw FileError();
  }
  std::uint64_t data_size;
  if (!is.read((char*)&data_size, sizeof(data_size))) {
    throw FileError();
  }
  std::vector<std::uint8_t> data(data_size);
  if (!is.read((char*)data.data(), data.size())) {
    throw FileError();
  }
  return std::make_tuple(hash, data);
}

void FileReader::skip() {
  if (!is.seekg(sizeof(std::uint64_t), std::ios::cur)) {
    throw FileError();
  }
  std::uint64_t data_size;
  if (!is.read((char*)&data_size, sizeof(data_size))) {
    throw FileError();
  }
  if (!is.seekg(data_size, std::ios::cur)) {
    throw FileError();
  }
}

} // namespace datapack
