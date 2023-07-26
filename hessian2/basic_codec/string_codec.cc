#include "hessian2/basic_codec/string_codec.hpp"

#include "absl/container/inlined_vector.h"

namespace Hessian2 {

namespace {
constexpr size_t STRING_CHUNK_SIZE = 32768;

using Uint64Vector = absl::InlinedVector<uint64_t, 8>;

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
static const uint8_t UTF_8_CHAR_LENGTHS[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                             1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                             0, 0, 2, 2, 2, 2, 3, 3, 4, 0};

/**
 * Get number of UTF-8 characters in string. Per chunk raw bytes offset and
 * four bytes char offsets are also calculated. This is only used for
 * 'encode' function.
 */
int64_t getUtf8StringLength(absl::string_view in,
                            Uint64Vector &per_chunk_bytes_offsets,
                            Uint64Vector &four_bytes_char_offsets) {
  int64_t utf8_length = 0;
  size_t raw_bytes_length = 0;

  size_t current_chunk = 0;

  const size_t in_size = in.size();

  for (; raw_bytes_length < in_size;) {
    const uint8_t code = static_cast<uint8_t>(in[raw_bytes_length]);
    const uint8_t char_length = UTF_8_CHAR_LENGTHS[code >> 3];

    // Check the validity of UTF-8 string.
    if (char_length == 0 || raw_bytes_length + char_length > in_size) {
      return -1;
    }

    // Record the offset of the four bytes UTF-8 character.
    if (char_length == 4) {
      four_bytes_char_offsets.push_back(raw_bytes_length);
    }

    utf8_length++;
    raw_bytes_length += char_length;

    current_chunk++;

    // Check whether the current chunk is full and record the bytes offset of
    // the current chunk.
    if (current_chunk == STRING_CHUNK_SIZE) {
      per_chunk_bytes_offsets.push_back(raw_bytes_length);
      current_chunk = 0;
    }
  }

  // Record the bytes offset of the last chunk.
  if (current_chunk > 0) {
    per_chunk_bytes_offsets.push_back(raw_bytes_length);
    current_chunk = 0;
  }

  return utf8_length;
}

/**
 * Rewrite 4 bytes UTF-8 characters of UTF-8 string.
 */
std::string rewriteUtf8String(absl::string_view in,
                              const Uint64Vector &four_bytes_char_offsets) {
  std::string out;
  out.reserve(in.size() + four_bytes_char_offsets.size() * 3);

  size_t last_pos = 0;
  for (const size_t pos : four_bytes_char_offsets) {
    const absl::string_view sub_segment = in.substr(last_pos, pos - last_pos);
    out.append(sub_segment.data(), sub_segment.size());

    // Get code point of 4-byte character.
    uint32_t code_point = (static_cast<uint32_t>(in[pos] & 0x07) << 18) |
                          (static_cast<uint32_t>(in[pos + 1] & 0x3F) << 12) |
                          (static_cast<uint32_t>(in[pos + 2] & 0x3F) << 6) |
                          (static_cast<uint32_t>(in[pos + 3] & 0x3F));

    // Check the range of code point of 4-byte character.
    if (code_point < 0x10000 || code_point > 0x10FFFF) {
      return "";
    }

    // Covert the code point to UTF-16 surrogate pair.
    code_point -= 0x10000;
    static const uint16_t surrogate_pair[2] = {
        static_cast<uint16_t>(0xD800 + (code_point >> 10)),
        static_cast<uint16_t>(0xDC00 + (code_point & 0x3FF))};

    // Covert high and low surrogate to UTF-8.
    // The Java hessian2 library will encode one surrogate pair
    // (U+10000-U+10FFFF) to two UTF-8 characters. This is wrong, because one
    // surrogate pair (U+10000-U+10FFFF) should be encoded to one 4 bytes
    // UTF-8 characters. However, we still need to be compatible with the
    // Java hessian2 library, so we need to do the same thing even it is
    // wrong. Ref:
    // https://github.com/apache/dubbo-hessian-lite/blob/ca001b4658227d5122f85bcb45032a0dac4faf0d/src/main/java/com/alibaba/com/caucho/hessian/io/Hessian2Output.java#L1360
    for (auto utf16_char : surrogate_pair) {
      // Needn't to check the range of 'utf16_char', because it must larger
      // than 0x800 and less than 0xFFFF， so it must be 3 bytes UTF-8.
      out.push_back(static_cast<char>(0xE0 | ((utf16_char >> 12))));
      out.push_back(static_cast<char>(0x80 | ((utf16_char >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | ((utf16_char & 0x3F))));
    }

    last_pos = pos + 4;
  }

  const absl::string_view last_segment = in.substr(last_pos);
  out.append(last_segment.data(), last_segment.size());

  return out;
}

/**
 * Get number of UTF-8 characters in string. This is only used for
 * 'finalReadUtf8String' function.
 *
 * @param in The input string.
 * @return pair of number of UTF-8 characters and number of raw bytes. If the
 *         input string is not a valid UTF-8 string, the number of UTF-8
 *         characters is -1.
 */
std::pair<int64_t, size_t> getUtf8StringLength(absl::string_view in) {
  int64_t utf8_length = 0;
  size_t raw_bytes_length = 0;

  const size_t in_size = in.size();

  for (; raw_bytes_length < in_size;) {
    const uint8_t code = static_cast<uint8_t>(in[raw_bytes_length]);
    const uint8_t char_length = UTF_8_CHAR_LENGTHS[code >> 3];

    if (char_length == 0) {
      return {-1, 0};
    }

    utf8_length++;
    raw_bytes_length += char_length;
  }

  return {utf8_length, raw_bytes_length};
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
    const uint64_t current_pos = output.size();

    output.resize(current_pos + length);
    // Read the 'length' bytes from the reader buffer to the output.
    reader.readNBytes(&output[current_pos], length);

    const auto output_view = absl::string_view(output).substr(current_pos);

    const auto result = getUtf8StringLength(output_view);
    const int64_t utf8_length = result.first;
    const size_t raw_bytes_length = result.second;

    if (utf8_length == -1) {
      return false;
    }

    if (raw_bytes_length > length) {
      const size_t padding_size = raw_bytes_length - length;
      if (reader.byteAvailable() < padding_size) {
        return false;
      }
      output.resize(current_pos + raw_bytes_length);
      // Read the 'padding_size' bytes from the reader buffer to the output.
      reader.readNBytes(&output[0] + current_pos + length, padding_size);
    }

    length -= utf8_length;
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
  Uint64Vector per_chunk_bytes_offsets;
  Uint64Vector four_bytes_char_offsets;

  int64_t length = getUtf8StringLength(data, per_chunk_bytes_offsets,
                                       four_bytes_char_offsets);
  if (length == -1) {
    return false;
  }

  absl::string_view data_view = data;

  std::string rewrite_data;
  if (!four_bytes_char_offsets.empty()) {
    rewrite_data = rewriteUtf8String(data, four_bytes_char_offsets);
    if (rewrite_data.empty()) {
      return false;
    }

    per_chunk_bytes_offsets.clear();
    four_bytes_char_offsets.clear();

    length = getUtf8StringLength(rewrite_data, per_chunk_bytes_offsets,
                                 four_bytes_char_offsets);
    data_view = rewrite_data;
  }

  // Check length again.
  if (length == -1) {
    return false;
  }

  // Java's 16-bit integers are signed, so the maximum value is 32768
  uint32_t str_offset = 0;
  const uint16_t step_length = STRING_CHUNK_SIZE;
  int pos = 0;
  while (static_cast<uint64_t>(length) > STRING_CHUNK_SIZE) {
    writer_->writeByte(0x52);
    writer_->writeBE<uint16_t>(step_length);
    length -= step_length;
    auto raw_offset = per_chunk_bytes_offsets[pos++];
    writer_->rawWrite(data_view.substr(str_offset, raw_offset - str_offset));
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
    writer_->rawWrite(data_view.substr(str_offset, data.size() - str_offset));
    return true;
  }

  // [x30-x34] <utf8-data>
  if (length <= 1023) {
    uint8_t code = length / 256;
    uint8_t remain = length % 256;
    writer_->writeByte(0x30 + code);
    writer_->writeByte(remain);
    writer_->rawWrite(data_view.substr(str_offset, data.size() - str_offset));
    return true;
  }

  writer_->writeByte(0x53);
  writer_->writeBE<uint16_t>(length);
  writer_->rawWrite(data_view.substr(str_offset, data.size() - str_offset));
  return true;
}

template <>
bool Encoder::encode(const std::string &data) {
  return encode<absl::string_view>(absl::string_view(data));
}

}  // namespace Hessian2
