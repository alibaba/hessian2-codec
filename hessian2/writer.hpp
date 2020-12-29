#pragma once

#include "absl/strings/string_view.h"
#include "byte_order.h"

namespace hessian2 {

class Writer {
 public:
  Writer() = default;
  virtual ~Writer() = default;
  virtual void RawWrite(const void* data, uint64_t size) = 0;
  virtual void RawWrite(absl::string_view data) = 0;

  void WriteByte(uint8_t value) { RawWrite(std::addressof(value), 1); }

  template <typename T>
  void WriteByte(T value) {
    WriteByte(static_cast<uint8_t>(value));
  }

  template <ByteOrderType Endianness = ByteOrderType::Host, typename T>
  void Write(
      typename std::enable_if<std::is_integral<T>::value, T>::type value) {
    const auto data = toEndian<Endianness>(value);
    RawWrite(reinterpret_cast<const char*>(std::addressof(data)), sizeof(T));
  }

  template <typename T>
  void WriteLE(T value) {
    Write<ByteOrderType::LittleEndian, T>(value);
  }

  template <typename T>
  void WriteBE(T value) {
    Write<ByteOrderType::BigEndian, T>(value);
  }
};

using WriterPtr = std::unique_ptr<Writer>;

}  // namespace hessian2