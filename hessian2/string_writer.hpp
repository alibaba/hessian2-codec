#pragma once

#include "absl/strings/string_view.h"
#include "hessian2/writer.hpp"

namespace hessian2 {

class StringWriter : public Writer {
 public:
  StringWriter(std::string& data) : data_(data) {}
  ~StringWriter() = default;
  virtual void RawWrite(const void* data, uint64_t size) {
    const char* src = static_cast<const char*>(data);
    data_.append(src, size);
  }
  virtual void RawWrite(absl::string_view data) {
    data_.append(data.data(), data.size());
  }

 private:
  std::string& data_;
};

}  // namespace hessian2