
#include <vector>
#include <iostream>

#include "hessian2/codec.hpp"
#include "hessian2/basic_codec/object_codec.hpp"
#include "hessian2/reader.hpp"
#include "hessian2/writer.hpp"

#include "absl/strings/string_view.h"

struct Slice {
  const uint8_t* data_;
  size_t size_;
};

class SliceReader : public ::hessian2::Reader {
 public:
  SliceReader(Slice buffer) : buffer_(buffer){};
  virtual ~SliceReader() = default;

  virtual void RawReadNbytes(void* out, size_t len,
                             size_t peek_offset) override {
    ABSL_ASSERT(ByteAvailable() + peek_offset >= len);
    uint8_t* dest = static_cast<uint8_t*>(out);
    // Offset() Returns the current position that has been read.
    memcpy(dest, buffer_.data_ + Offset() + peek_offset, len);
  }
  virtual uint64_t Length() const override { return buffer_.size_; }

 private:
  Slice buffer_;
};

class VectorWriter : public ::hessian2::Writer {
 public:
  VectorWriter(std::vector<uint8_t>& data) : data_(data) {}
  ~VectorWriter() = default;
  virtual void RawWrite(const void* data, uint64_t size) {
    const char* src = static_cast<const char*>(data);
    for (size_t i = 0; i < size; i++) {
      data_.push_back(src[i]);
    }
  }
  virtual void RawWrite(absl::string_view data) {
    for (auto& ch : data) {
      data_.push_back(ch);
    }
  }

 private:
  std::vector<uint8_t>& data_;
};

int main() {
  std::vector<uint8_t> data;
  auto writer = std::make_unique<VectorWriter>(data);

  ::hessian2::Encoder encode(std::move(writer));
  encode.encode<std::string>("test string");
  Slice s{static_cast<const uint8_t*>(data.data()), data.size()};
  auto reader = std::make_unique<SliceReader>(s);
  ::hessian2::Decoder decode(std::move(reader));
  auto ret = decode.decode<std::string>();
  if (ret) {
    std::cout << *ret << std::endl;
  } else {
    std::cerr << "decode failed: " << decode.getErrorMessage() << std::endl;
  }
}