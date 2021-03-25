#include "hessian2/basic_codec/string_codec.hpp"

namespace Hessian2 {

namespace {
constexpr size_t STRING_CHUNK_SIZE = 32768;

// The legal UTF-8 encoding uses 1 to 4 bytes to represent a character. Their
// format is shown below.

// length byte[0]  byte[1]  byte[2]  byte[3]
// 1      0xxxxxxx
// 2      110xxxxx 10xxxxxx
// 3      1110xxxx 10xxxxxx 10xxxxxx
// 4      11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

// According to the above format, only the first five bits of the first byte are
// needed to determine the number of bytes occupied by a character. There are a
// total of 32 possibilities for 5 bits. Use 32 possible values as indexes and
// the corresponding number of bytes as values to form the following array to
// speed up the parsing of UTF-8 characters.
// Ref: https://nullprogram.com/blog/2017/10/06/
static const size_t UTF_8_CHAR_LENGTHS[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                            0, 0, 2, 2, 2, 2, 3, 3, 4, 0};

template <bool WITH_PER_CHUNK>
int64_t getUtf8StringLength(const absl::string_view &out,
                            std::vector<uint64_t> &per_chunk_raw_size) {
  size_t utf_len = 0;
  size_t i = 0;
  size_t next_chunk_size = 0;

  for (; i < out.size();) {
    auto code = static_cast<uint8_t>(out[i]);
    auto size = UTF_8_CHAR_LENGTHS[code >> 3];

    if (size == 0) {
      return -1;
    }

    utf_len++;
    i += size;

    if constexpr (WITH_PER_CHUNK) {
      if ((utf_len - next_chunk_size) >= STRING_CHUNK_SIZE) {
        next_chunk_size += STRING_CHUNK_SIZE;
        per_chunk_raw_size.push_back(i);
      }
    }
  }

  if (per_chunk_raw_size.empty() || per_chunk_raw_size.back() != i) {
    per_chunk_raw_size.push_back(i);
  }

  return utf_len;
}

// TODO(tianqian.zyf): Do I need to check the UTF-8 validity?
// Ref: https://www.cl.cam.ac.uk/~mgk25/ucs/utf8_check.c
bool finalReadUtf8String(std::string &output, Reader &reader, size_t length) {
  // The length length refers to the length of utF8 characters,
  // and utF8 can be represented by up to 4 bytes, so it is length * 4
  output.reserve(length * 4);
  while (length > 0) {
    if (reader.byteAvailable() < length) {
      return false;
    }
    uint64_t current_pos = output.size();
    std::vector<uint64_t> raw_bytes_size;

    output.resize(current_pos + length);
    // Read the 'length' bytes from the reader buffer to the output.
    reader.readNBytes(&output[current_pos], length);

    auto output_view = absl::string_view(output).substr(current_pos);
    int64_t utf8_len = getUtf8StringLength<false>(output_view, raw_bytes_size);

    if (utf8_len == -1) {
      return false;
    }

    size_t byte_len = raw_bytes_size.back();
    if (byte_len > length) {
      auto padding_size = byte_len - length;
      if (reader.byteAvailable() < padding_size) {
        return false;
      }
      output.resize(current_pos + byte_len);
      // Read the 'padding_size' bytes from the reader buffer to the output.
      reader.readNBytes(&output[0] + current_pos + length, padding_size);
    }

    length -= utf8_len;
  }
  return true;
}

bool readChunkString(std::string &output, Reader &reader, size_t length,
                     bool is_last_chunk);

bool decodeStringWithReader(std::string &out, Reader &reader) {
  size_t delta_length = 0;
  auto ret = reader.read<uint8_t>();
  if (!ret.first) {
    return false;
  }
  uint8_t code = ret.second;
  switch (code) {
    // ::= [x00-x1f] <utf8-data>         # string of length
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
    case 0x0c:
    case 0x0d:
    case 0x0e:
    case 0x0f:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f: {
      return readChunkString(out, reader, code - 0x00, true);
    }

    // ::= [x30-x33] <utf8-data> # string of length
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33: {
      auto res = reader.read<uint8_t>();
      if (!res.first) {
        return false;
      }
      delta_length = (code - 0x30) * 256 + res.second;
      return readChunkString(out, reader, delta_length, true);
    }

    case 0x53:  // 0x53 is 'S', 'S' b1 b0 <utf8-data>
    {
      auto res = reader.readBE<uint16_t>();
      if (!res.first) {
        return false;
      }
      return readChunkString(out, reader, res.second, true);
    }
    case 0x52:  // 0x52 b1 b0 <utf8-data>
    {
      auto res = reader.readBE<uint16_t>();
      if (!res.first) {
        return false;
      }
      return readChunkString(out, reader, res.second, false);
    }
  }
  return false;
}

bool readChunkString(std::string &output, Reader &reader, size_t length,
                     bool is_last_chunk) {
  auto ret = finalReadUtf8String(output, reader, length);
  if (!ret) {
    return false;
  }

  if (is_last_chunk) {
    return true;
  }

  return decodeStringWithReader(output, reader);
}

}  // namespace

template <>
std::unique_ptr<std::string> Decoder::decode() {
  auto out = std::make_unique<std::string>();
  if (!decodeStringWithReader(*out.get(), *reader_.get())) {
    return nullptr;
  }
  return out;
}

// # UTF-8 encoded character string split into 32k chunks
// ::= x52 b1 b0 <utf8-data> string  # non-final chunk
// ::= 'S' b1 b0 <utf8-data>         # string of length 0-32768
// ::= [x00-x1f] <utf8-data>         # string of length 0-31
// ::= [x30-x34] <utf8-data>         # string of length 0-1023
template <>
bool Encoder::encode(const absl::string_view &data) {
  std::vector<uint64_t> raw_chunk_size;
  int64_t length = getUtf8StringLength<true>(data, raw_chunk_size);
  if (length == -1) {
    return false;
  }
  // Java's 16-bit integers are signed, so the maximum value is 32768
  uint32_t str_offset = 0;
  uint16_t step_length = STRING_CHUNK_SIZE;
  int pos = 0;
  while (static_cast<uint64_t>(length) > STRING_CHUNK_SIZE) {
    writer_->writeByte(0x52);
    writer_->writeBE<uint16_t>(step_length);
    length -= step_length;
    auto raw_offset = raw_chunk_size[pos++];
    writer_->rawWrite(data.substr(str_offset, raw_offset - str_offset));
    str_offset = raw_offset;
  }

  if (length == 0) {
    // x00  # "", empty string
    writer_->writeByte(0x00);
    return true;
  }

  if (length <= 31) {
    // [x00-x1f] <utf8-data>
    // Compact: short strings
    writer_->writeByte(length);
    writer_->rawWrite(data.substr(str_offset, data.size() - str_offset));
    return true;
  }

  // [x30-x34] <utf8-data>
  if (length <= 1023) {
    uint8_t code = length / 256;
    uint8_t remain = length % 256;
    writer_->writeByte(0x30 + code);
    writer_->writeByte(remain);
    writer_->rawWrite(data.substr(str_offset, data.size() - str_offset));
    return true;
  }

  writer_->writeByte(0x53);
  writer_->writeBE<uint16_t>(length);
  writer_->rawWrite(data.substr(str_offset, data.size() - str_offset));
  return true;
}

template <>
bool Encoder::encode(const std::string &data) {
  return encode<absl::string_view>(absl::string_view(data));
}

}  // namespace Hessian2
