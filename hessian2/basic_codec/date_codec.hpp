#pragma once

#include "hessian2/codec.hpp"

/////////////////////////////////////////
// Bool
/////////////////////////////////////////

namespace hessian2 {

namespace {

template <typename T>
std::unique_ptr<
    typename std::enable_if<std::chrono::__is_duration<T>::value, T>::type>
ReadDate(ReaderPtr &reader) {
  auto out = std::make_unique<T>();
  uint8_t code = reader->Read<uint8_t>().second;
  switch (code) {
    case 0x4b:
      if (reader->ByteAvailable() < 4) {
        return nullptr;
      }
      return std::make_unique<T>(std::chrono::duration_cast<T>(
          std::chrono::minutes(reader->ReadBE<int32_t>().second)));
    case 0x4a:
      if (reader->ByteAvailable() < 8) {
        return nullptr;
      }
      return std::make_unique<T>(std::chrono::duration_cast<T>(
          std::chrono::milliseconds(reader->ReadBE<int64_t>().second)));
  }
  return nullptr;
}

template <typename T>
void WriteDate(
    WriterPtr &writer,
    const typename std::enable_if<std::chrono::__is_duration<T>::value, T>::type
        &value) {
  int64_t value_in_min =
      std::chrono::duration_cast<std::chrono::minutes>(value).count();
  int64_t value_in_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(value).count();
  if (value_in_min * 60000 == value_in_ms) {
    writer->WriteByte(0x4b);
    writer->WriteBE<int32_t>(value_in_min);
  } else {
    writer->WriteByte(0x4a);
    writer->WriteBE<int64_t>(value_in_ms);
  }
}

}  // namespace

// # time in UTC encoded as 64-bit long milliseconds since epoch
// ::= x4a b7 b6 b5 b4 b3 b2 b1 b0
// ::= x4b b3 b2 b1 b0       # minutes since epoch
template <>
std::unique_ptr<std::chrono::milliseconds> Decoder::decode() {
  return ReadDate<std::chrono::milliseconds>(reader_);
}

template <>
std::unique_ptr<std::chrono::minutes> Decoder::decode() {
  return ReadDate<std::chrono::minutes>(reader_);
}

template <>
std::unique_ptr<std::chrono::seconds> Decoder::decode() {
  return ReadDate<std::chrono::seconds>(reader_);
}

template <>
std::unique_ptr<std::chrono::hours> Decoder::decode() {
  return ReadDate<std::chrono::hours>(reader_);
}

#if _LIBCPP_STD_VER > 17
template <>
std::unique_ptr<std::chrono::days> Decoder::decode() {
  return ReadDate<std::chrono::days>(reader_);
}
std::unique_ptr<std::chrono::weeks> Decoder::decode() {
  return ReadDate<std::chrono::weeks>(reader_);
}
std::unique_ptr<std::chrono::years> Decoder::decode() {
  return ReadDate<std::chrono::years>(reader_);
}
std::unique_ptr<std::chrono::months> Decoder::decode() {
  return ReadDate<std::chrono::months>(reader_);
}
#endif

// # time in UTC encoded as 64-bit long milliseconds since epoch
// ::= x4a b7 b6 b5 b4 b3 b2 b1 b0
// ::= x4b b3 b2 b1 b0       # minutes since epoch

template <>
bool Encoder::encode(const std::chrono::minutes &value) {
  writer_->WriteByte(0x4b);
  writer_->WriteBE<int32_t>(value.count());
  return true;
}

template <>
bool Encoder::encode(const std::chrono::milliseconds &value) {
  std::chrono::minutes value_min =
      std::chrono::duration_cast<std::chrono::minutes>(value);
  if (value_min.count() * 60000 == value.count()) {
    return encode<std::chrono::minutes>(value_min);
  }
  writer_->WriteByte(0x4a);
  writer_->WriteBE<int64_t>(value.count());
  return true;
}

template <>
bool Encoder::encode(const std::chrono::seconds &value) {
  WriteDate<std::chrono::seconds>(writer_, value);
  return true;
}

template <>
bool Encoder::encode(const std::chrono::hours &value) {
  WriteDate<std::chrono::hours>(writer_, value);
  return true;
}

#if _LIBCPP_STD_VER > 17
template <>
bool Encoder::encode(const std::chrono::days &value) {
  WriteDate<std::chrono::days>(writer_, value);
  return true;
}
template <>
bool Encoder::encode(const std::chrono::weeks &value) {
  WriteDate<std::chrono::weeks>(writer_, value);
  return true;
}
template <>
bool Encoder::encode(const std::chrono::years &value) {
  WriteDate<std::chrono::years>(writer_, value);
  return true;
}
template <>
bool Encoder::encode(const std::chrono::months &value) {
  WriteDate<std::chrono::months>(writer_, value);
  return true;
}
#endif

}  // namespace hessian2