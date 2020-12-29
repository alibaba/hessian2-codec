#include "hessian2/codec.hpp"

#include "hessian2/basic_codec/type_ref_codec.hpp"
#include "hessian2/basic_codec/def_ref_codec.hpp"

namespace hessian2 {

void Encoder::encodeVarListBegin(const std::string&) {
  writer_->WriteByte(static_cast<uint8_t>(0x57));
}

void Encoder::encodeVarListEnd() { writer_->WriteByte('Z'); }

void Encoder::encodeFixedListBegin(const std::string& type, uint32_t len) {
  if (len <= 7) {
    writer_->WriteByte(static_cast<uint8_t>(0x70 + len));
  } else {
    writer_->WriteByte('V');
  }

  if (!type.empty()) {
    Object::TypeRef type_ref(type);
    encode<Object::TypeRef>(type_ref);
  }

  if (len > 7) {
    encode<int32_t>(len);
  }
}

void Encoder::encodeFixedListEnd() {
  // Do nothing
}

void Encoder::encodeMapBegin(const std::string& type) {
  if (type.empty()) {
    writer_->WriteByte('H');
  } else {
    writer_->WriteByte('M');
    Object::TypeRef ref(type);
    encode<Object::TypeRef>(ref);
  }
}

void Encoder::encodeMapEnd() { writer_->WriteByte('Z'); }

void Encoder::encodeClassInstanceBegin(const Object::RawDefinition& value) {
  encode<Object::RawDefinition>(value);
}

void Encoder::encodeClassInstanceEnd() {
  // Do nothing
}

}  // namespace hessian2