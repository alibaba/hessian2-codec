#pragma once

#include "hessian2/codec.hpp"

namespace hessian2 {
/////////////////////////////////////////
// Double
/////////////////////////////////////////
namespace {
template <typename T>
typename std::enable_if<
    std::is_signed<T>::value && (sizeof(T) > sizeof(int8_t)), T>::type
LeftShift(int8_t left, uint16_t bit_number) {
  if (left < 0) {
    left = left * -1;
    return -1 * (left << bit_number);
  }
  return left << bit_number;
}

template <typename T, size_t Size>
typename std::enable_if<std::is_floating_point<T>::value, T>::type ReadBE(
    ReaderPtr &reader) {
  static_assert(sizeof(T) == 8, "Only support double type");
  static_assert(Size == 8 || Size == 4, "Only support 4 or 8 size");
  double out;
  if (Size == 8) {
    auto in = reader->ReadBE<uint64_t>();
    ABSL_ASSERT(in.first);
    std::memcpy(&out, &in.second, 8);
    return out;
  }

  // Frankly, I don't know why I'm doing this, I'm just referring to the Java
  // implementation.
  ABSL_ASSERT(Size == 4);
  auto in = reader->ReadBE<int32_t>();
  ABSL_ASSERT(in.first);
  return 0.001 * in.second;
}

void WriteBEDouble(WriterPtr &writer, const double &value) {
  uint64_t out;
  std::memcpy(&out, &value, 8);
  writer->WriteBE<uint64_t>(out);
}
}  // namespace

// # 64-bit IEEE double
// ::= 'D' b7 b6 b5 b4 b3 b2 b1 b0
// ::= x5b                   # 0.0
// ::= x5c                   # 1.0
// ::= x5d b0                # byte cast to double (-128.0 to 127.0)
// ::= x5e b1 b0             # short cast to double
// ::= x5f b3 b2 b1 b0       # 32-bit float cast to double
template <>
std::unique_ptr<double> Decoder::decode() {
  auto out = std::make_unique<double>();
  uint8_t code = reader_->ReadBE<uint8_t>().second;
  switch (code) {
    // ::= x5b                   # 0.0
    case 0x5b:
      *out.get() = 0.0;
      return out;
    // ::= x5c                   # 1.0
    case 0x5c:
      *out.get() = 1.0;
      return out;
    // ::= x5d b0                # byte cast to double (-128.0 to 127.0)
    case 0x5d:
      if (reader_->ByteAvailable() < 1) {
        return nullptr;
      }
      *out.get() = static_cast<double>(reader_->ReadBE<int8_t>().second);
      return out;
    // ::= x5e b1 b0             # short cast to double
    case 0x5e:
      if (reader_->ByteAvailable() < 2) {
        return nullptr;
      }
      *out.get() = static_cast<double>(reader_->ReadBE<int16_t>().second);
      return out;
    // ::= x5f b3 b2 b1 b0       # 32-bit float cast to double
    case 0x5f:
      if (reader_->ByteAvailable() < 4) {
        return nullptr;
      }
      *out.get() = ReadBE<double, 4>(reader_);
      return out;
    // ::= 'D' b7 b6 b5 b4 b3 b2 b1 b0
    case 'D':
      if (reader_->ByteAvailable() < 8) {
        return nullptr;
      }
      *out.get() = ReadBE<double, 8>(reader_);
      return out;
  }
  return nullptr;
}

// # 64-bit IEEE double
// ::= 'D' b7 b6 b5 b4 b3 b2 b1 b0
// ::= x5b                   # 0.0
// ::= x5c                   # 1.0
// ::= x5d b0                # byte cast to double (-128.0 to 127.0)
// ::= x5e b1 b0             # short cast to double
// ::= x5f b3 b2 b1 b0       # 32-bit float cast to double
template <>
bool Encoder::encode(const double &value) {
  int32_t int_value = static_cast<int32_t>(value);
  if (int_value == value) {
    if (int_value == 0) {
      writer_->WriteByte(0x5b);
      return true;
    }

    if (int_value == 1) {
      writer_->WriteByte(0x5c);
      return true;
    }

    if (int_value >= -0x80 && int_value < 0x80) {
      writer_->WriteByte(0x5d);
      writer_->WriteBE<int8_t>(int_value);
      return true;
    }

    if (int_value >= -0x8000 && int_value < 0x8000) {
      writer_->WriteByte(0x5e);
      writer_->WriteBE<int8_t>(int_value >> 8);
      writer_->WriteBE<uint8_t>(int_value);
      return true;
    }
  }

  writer_->WriteByte(0x44);
  WriteBEDouble(writer_, value);
  return true;
}

/////////////////////////////////////////
// Int32
/////////////////////////////////////////

// # 32-bit signed integer
// ::= 'I' b3 b2 b1 b0
// ::= [x80-xbf]             # -x10 to x3f
// ::= [xc0-xcf] b0          # -x800 to x7ff
// ::= [xd0-xd7] b1 b0       # -x40000 to x3ffff
template <>
std::unique_ptr<int32_t> Decoder::decode() {
  auto out = std::make_unique<int32_t>();
  uint8_t code = reader_->ReadBE<uint8_t>().second;

  // ::= [x80-xbf]             # -x10 to x3f
  if (code >= 0x80 && code <= 0xbf) {
    *out.get() = (code - 0x90);
    return out;
  }
  switch (code) {
    // ::= [xc0-xcf] b0          # -x800 to x7ff
    case 0xc0:
    case 0xc1:
    case 0xc2:
    case 0xc3:
    case 0xc4:
    case 0xc5:
    case 0xc6:
    case 0xc7:
    case 0xc8:
    case 0xc9:
    case 0xca:
    case 0xcb:
    case 0xcc:
    case 0xcd:
    case 0xce:
    case 0xcf:
      if (reader_->ByteAvailable() < 1) {
        return nullptr;
      }
      *out.get() = LeftShift<int16_t>(code - 0xc8, 8) +
                   reader_->ReadBE<uint8_t>().second;
      return out;
    // ::= [xd0-xd7] b1 b0       # -x40000 to x3ffff
    case 0xd0:
    case 0xd1:
    case 0xd2:
    case 0xd3:
    case 0xd4:
    case 0xd5:
    case 0xd6:
    case 0xd7:
      if (reader_->ByteAvailable() < 2) {
        return nullptr;
      }
      *out.get() = LeftShift<int32_t>(code - 0xd4, 16) +
                   reader_->ReadBE<uint16_t>().second;
      return out;
    // ::= 'I' b3 b2 b1 b0
    case 0x49:
      if (reader_->ByteAvailable() < 4) {
        return nullptr;
      }
      *out.get() = reader_->ReadBE<int32_t>().second;
      return out;
  }

  return nullptr;
}

// # 32-bit signed integer
// ::= 'I' b3 b2 b1 b0
// ::= [x80-xbf]             # -x10 to x3f
// ::= [xc0-xcf] b0          # -x800 to x7ff
// ::= [xd0-xd7] b1 b0       # -x40000 to x3ffff
template <>
bool Encoder::encode(const int32_t &data) {
  if (data >= -0x10 && data <= 0x2f) {
    writer_->WriteByte(data + 0x90);
    return true;
  }

  if (data >= -0x800 && data <= 0x7ff) {
    writer_->WriteByte(0xc8 + (data >> 8));
    writer_->WriteByte(data);
    return true;
  }

  if (data >= -0x40000 && data <= 0x3ffff) {
    writer_->WriteByte(0xd4 + (data >> 16));
    writer_->WriteByte(data >> 8);
    writer_->WriteByte(data);
    return true;
  }
  writer_->WriteByte(0x49);
  writer_->WriteBE<uint32_t>(data);
  return true;
}

/////////////////////////////////////////
// Int64
/////////////////////////////////////////

// # 64-bit signed long integer
// ::= 'L' b7 b6 b5 b4 b3 b2 b1 b0
// ::= [xd8-xef]             # -x08 to x0f
// ::= [xf0-xff] b0          # -x800 to x7ff
// ::= [x38-x3f] b1 b0       # -x40000 to x3ffff
// ::= x59 b3 b2 b1 b0       # 32-bit integer cast to long
template <>
std::unique_ptr<int64_t> Decoder::decode() {
  auto out = std::make_unique<int64_t>();
  uint8_t code = reader_->ReadBE<uint8_t>().second;
  switch (code) {
    // ::= [xd8-xef]             # -x08 to x0f
    case 0xd8:
    case 0xd9:
    case 0xda:
    case 0xdb:
    case 0xdc:
    case 0xdd:
    case 0xde:
    case 0xdf:
    case 0xe0:
    case 0xe1:
    case 0xe2:
    case 0xe3:
    case 0xe4:
    case 0xe5:
    case 0xe6:
    case 0xe7:
    case 0xe8:
    case 0xe9:
    case 0xea:
    case 0xeb:
    case 0xec:
    case 0xed:
    case 0xee:
    case 0xef:
      *out.get() = (code - 0xe0);
      return out;
    // ::= [xf0-xff] b0          # -x800 to x7ff
    case 0xf0:
    case 0xf1:
    case 0xf2:
    case 0xf3:
    case 0xf4:
    case 0xf5:
    case 0xf6:
    case 0xf7:
    case 0xf8:
    case 0xf9:
    case 0xfa:
    case 0xfb:
    case 0xfc:
    case 0xfd:
    case 0xfe:
    case 0xff:
      if (reader_->ByteAvailable() < 1) {
        return nullptr;
      }
      *out.get() = LeftShift<int16_t>(code - 0xf8, 8) +
                   reader_->ReadBE<uint8_t>().second;
      return out;
    // ::= [x38-x3f] b1 b0       # -x40000 to x3ffff
    case 0x38:
    case 0x39:
    case 0x3a:
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f:
      if (reader_->ByteAvailable() < 2) {
        return nullptr;
      }
      *out.get() = LeftShift<int32_t>(code - 0x3c, 16) +
                   reader_->ReadBE<uint16_t>().second;
      return out;
    // ::= x59 b3 b2 b1 b0       # 32-bit integer cast to long
    case 0x59:
      if (reader_->ByteAvailable() < 4) {
        return nullptr;
      }
      *out.get() = reader_->ReadBE<int32_t>().second;
      return out;
    // ::= 'L' b7 b6 b5 b4 b3 b2 b1 b0
    case 0x4c:
      if (reader_->ByteAvailable() < 8) {
        return nullptr;
      }
      *out.get() = reader_->ReadBE<int64_t>().second;
      return out;
  }
  return nullptr;
}

// # 64-bit signed long integer
// ::= 'L' b7 b6 b5 b4 b3 b2 b1 b0
// ::= [xd8-xef]             # -x08 to x0f
// ::= [xf0-xff] b0          # -x800 to x7ff
// ::= [x38-x3f] b1 b0       # -x40000 to x3ffff
// ::= x59 b3 b2 b1 b0       # 32-bit integer cast to long
template <>
bool Encoder::encode(const int64_t &data) {
  if (data >= -0x08 && data <= 0x0f) {
    writer_->WriteByte(data + 0xe0);
    return true;
  }

  if (data >= -0x800 && data <= 0x7ff) {
    writer_->WriteByte(0xf8 + (data >> 8));
    writer_->WriteByte(data);
    return true;
  }

  if (data >= -0x40000 && data <= 0x3ffff) {
    writer_->WriteByte(0x3c + (data >> 16));
    writer_->WriteByte(data >> 8);
    writer_->WriteByte(data);
    return true;
  }

  if (data >= -0x80000000L && data <= 0x7fffffffL) {
    writer_->WriteByte(0x59);
    writer_->WriteBE<int32_t>(data);
    return true;
  }

  writer_->WriteByte(0x4c);
  writer_->WriteBE<int64_t>(data);
  return true;
}

template <>
bool Encoder::encode(const int8_t &data) {
  return encode<int32_t>(data);
}

template <>
bool Encoder::encode(const int16_t &data) {
  return encode<int32_t>(data);
}

template <>
bool Encoder::encode(const uint8_t &data) {
  return encode<int32_t>(data);
}

template <>
bool Encoder::encode(const uint16_t &data) {
  return encode<int32_t>(data);
}

template <>
bool Encoder::encode(const uint32_t &data) {
  return encode<int64_t>(data);
}

// Encoding and decoding of uint64_t is not supported because Java 64-bit
// integers are signed.

}  // namespace hessian2