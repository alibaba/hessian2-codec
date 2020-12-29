#pragma once

#include <ctime>

#include "hessian2/codec.hpp"

/////////////////////////////////////////
// Binary, []byte
/////////////////////////////////////////

namespace hessian2 {

namespace {
constexpr size_t CHUNK_SIZE = 1024;
}

bool DecodeBytesWithReader(std::vector<uint8_t> &output, ReaderPtr &reader);
bool ReadBytes(std::vector<uint8_t> &output, ReaderPtr &reader, size_t length,
               bool is_last_chunk);

// # 8-bit binary data split into 64k chunks
// ::= x41(A) b1 b0 <binary-data> binary # non-final chunk
// ::= x42(B) b1 b0 <binary-data>        # final chunk
// ::= [x20-x2f] <binary-data>           # binary data of length 0-15
// ::= [x34-x37] <binary-data>           # binary data of length 0-1023
template <>
std::unique_ptr<std::vector<uint8_t>> Decoder::decode() {
  auto out = std::make_unique<std::vector<uint8_t>>();
  if (!DecodeBytesWithReader(*out.get(), reader_)) {
    return nullptr;
  }
  return out;
}

bool DecodeBytesWithReader(std::vector<uint8_t> &output, ReaderPtr &reader) {
  uint8_t code = reader->Read<uint8_t>().second;
  switch (code) {
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2a:
    case 0x2b:
    case 0x2c:
    case 0x2d:
    case 0x2e:
    case 0x2f:
      if (!ReadBytes(output, reader, code - 0x20, true)) {
        return false;
      }
      return true;
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37: {
      auto res = reader->Read<uint8_t>();
      if (!res.first ||
          !ReadBytes(output, reader, ((code - 0x34) << 8) + res.second, true)) {
        return false;
      }
      return true;
    }
    case 0x42: {
      auto res = reader->ReadBE<uint16_t>();
      if (!res.first) {
        return false;
      }
      return ReadBytes(output, reader, res.second, true);
    }
    case 0x41: {
      auto res = reader->ReadBE<uint16_t>();
      if (!res.first) {
        return false;
      }
      return ReadBytes(output, reader, res.second, false);
    }
  }
  return false;
}

// # 8-bit binary data split into 64k chunks
// ::= x41('A') b1 b0 <binary-data> binary # non-final chunk
// ::= x42('B') b1 b0 <binary-data>        # final chunk
// ::= [x20-x2f] <binary-data>  # binary data of length 0-15
// ::= [x34-x37] <binary-data>  # binary data of length 0-1023
template <>
bool Encoder::encode(const std::vector<uint8_t> &data) {
  size_t size = data.size();
  if (size < 16) {
    writer_->WriteByte(0x20 + size);
    writer_->RawWrite(
        absl::string_view(std::move(std::string(data.begin(), data.end()))));
    return true;
  }
  if (size < 1024) {
    writer_->WriteByte(0x34 + (size >> 8));
    writer_->WriteByte(size);
    writer_->RawWrite(
        absl::string_view(std::move(std::string(data.begin(), data.end()))));
    return true;
  }

  uint32_t offset = 0;
  while (size > CHUNK_SIZE) {
    writer_->WriteByte(0x41);
    writer_->WriteBE<uint16_t>(CHUNK_SIZE);
    size -= CHUNK_SIZE;
    writer_->RawWrite(absl::string_view(std::move(std::string(
        data.begin() + offset, data.begin() + offset + CHUNK_SIZE))));
    offset += CHUNK_SIZE;
  }

  if (size > 0) {
    writer_->WriteByte(0x42);
    writer_->WriteBE<uint16_t>(size);
    writer_->RawWrite(absl::string_view(
        std::move(std::string(data.begin() + offset, data.end()))));
  }
  return true;
}

bool ReadBytes(std::vector<uint8_t> &output, ReaderPtr &reader, size_t length,
               bool is_last_chunk) {
  if (length == 0) {
    return true;
  }
  if (length > reader->ByteAvailable()) {
    return false;
  }
  auto offset = output.size();
  output.resize(offset + length);
  reader->ReadNbytes(&output[offset], length);
  if (is_last_chunk) {
    return true;
  }
  return DecodeBytesWithReader(output, reader);
}

}  // namespace hessian2