#pragma once

#include <ctime>

#include "hessian2/codec.hpp"

/////////////////////////////////////////
// Date
/////////////////////////////////////////

namespace Hessian2 {
// # time in UTC encoded as 64-bit long milliseconds since epoch
// ::= x4a b7 b6 b5 b4 b3 b2 b1 b0
// ::= x4b b3 b2 b1 b0       # minutes since epoch
template <>
std::unique_ptr<bool> Decoder::decode() {
  uint8_t code = reader_->read<uint8_t>().second;
  if (code == 0x46) {
    return std::make_unique<bool>(false);
  }
  if (code == 0x54) {
    return std::make_unique<bool>(true);
  }
  error_pos_ = offset();
  error_code_ = ErrorCode::UNEXPECTED_TYPE;
  return nullptr;
}

// # boolean true/false
// ::= 'T'
// ::= 'F'
template <>
bool Encoder::encode(const bool &value) {
  if (value) {
    writer_->writeByte(0x54);
  } else {
    writer_->writeByte(0x46);
  }

  return true;
}

}  // namespace Hessian2
