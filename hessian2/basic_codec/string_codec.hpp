#pragma once

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "hessian2/codec.hpp"

namespace Hessian2 {

bool decodeStringWithReader(std::string &out, Reader &reader);
bool finalReadUtf8String(std::string &output, Reader &reader, size_t length);
bool readChunkString(std::string &output, Reader &reader, size_t length,
                     bool is_last_chunk);

int64_t getUtf8StringLength(const absl::string_view &out,
                            std::vector<uint64_t> &per_chunk_raw_size);

template <>
std::unique_ptr<std::string> Decoder::decode();

template <>
bool Encoder::encode(const absl::string_view &data);

template <>
bool Encoder::encode(const std::string &data);

}  // namespace Hessian2
