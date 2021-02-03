#pragma once

#include "hessian2/codec.hpp"

/////////////////////////////////////////
// Bool
/////////////////////////////////////////

namespace Hessian2 {

namespace {

template <typename T>
std::unique_ptr<
    typename std::enable_if<std::chrono::__is_duration<T>::value, T>::type>
readDate(ReaderPtr &reader) {
  auto out = std::make_unique<T>();
  uint8_t code = reader->read<uint8_t>().second;
  switch (code) {
    case 0x4b:
      if (reader->byteAvailable() < 4) {
        return nullptr;
      }
      return std::make_unique<T>(std::chrono::duration_cast<T>(
          std::chrono::minutes(reader->readBE<int32_t>().second)));
    case 0x4a:
      if (reader->byteAvailable() < 8) {
        return nullptr;
      }
      return std::make_unique<T>(std::chrono::duration_cast<T>(
          std::chrono::milliseconds(reader->readBE<int64_t>().second)));
  }
  return nullptr;
}

template <typename T>
void writeDate(
    WriterPtr &writer,
    const typename std::enable_if<std::chrono::__is_duration<T>::value, T>::type
        &value) {
  int64_t value_in_min =
      std::chrono::duration_cast<std::chrono::minutes>(value).count();
  int64_t value_in_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(value).count();
  if (value_in_min * 60000 == value_in_ms) {
    writer->writeByte(0x4b);
    writer->writeBE<int32_t>(value_in_min);
  } else {
    writer->writeByte(0x4a);
    writer->writeBE<int64_t>(value_in_ms);
  }
}

}  // namespace

template <>
std::unique_ptr<std::chrono::milliseconds> Decoder::decode();

template <>
std::unique_ptr<std::chrono::minutes> Decoder::decode();

template <>
std::unique_ptr<std::chrono::seconds> Decoder::decode();

template <>
std::unique_ptr<std::chrono::hours> Decoder::decode();

#if _LIBCPP_STD_VER > 17
template <>
std::unique_ptr<std::chrono::days> Decoder::decode();

template <>
std::unique_ptr<std::chrono::weeks> Decoder::decode();

template <>
std::unique_ptr<std::chrono::years> Decoder::decode();

template <>
std::unique_ptr<std::chrono::months> Decoder::decode();
#endif

template <>
bool Encoder::encode(const std::chrono::minutes &value);

template <>
bool Encoder::encode(const std::chrono::milliseconds &value);

template <>
bool Encoder::encode(const std::chrono::seconds &value);

template <>
bool Encoder::encode(const std::chrono::hours &value);

#if _LIBCPP_STD_VER > 17
template <>
bool Encoder::encode(const std::chrono::days &value);

template <>
bool Encoder::encode(const std::chrono::weeks &value);

template <>
bool Encoder::encode(const std::chrono::years &value);

template <>
bool Encoder::encode(const std::chrono::months &value);
#endif

}  // namespace Hessian2
