#pragma once

#include "absl/strings/string_view.h"
#include "hessian2/reader.hpp"

namespace hessian2 {

class StringReader : public Reader {
 public:
  StringReader(absl::string_view data) : buffer_(data){};
  virtual ~StringReader() = default;

  virtual void RawReadNbytes(void* out, size_t len,
                             size_t peek_offset) override {
    ABSL_ASSERT(ByteAvailable() + peek_offset >= len);
    absl::string_view data = buffer_.substr(Offset() + peek_offset, len);
    uint8_t* dest = static_cast<uint8_t*>(out);
    memcpy(dest, data.data(), len);
  }
  virtual uint64_t Length() const override { return buffer_.size(); }

 private:
  absl::string_view buffer_;
};

}  // namespace hessian2
