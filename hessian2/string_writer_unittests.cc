#include <vector>

#include "gtest/gtest.h"
#include "common/common.h"
#include "hessian2/string_writer.hpp"
#include "hessian2/writer.hpp"

namespace hessian2 {

TEST(StringWriterTest, WriteI8) {
  std::string out;
  StringWriter buffer(out);
  buffer.WriteByte(-128);
  buffer.WriteByte(-1);
  buffer.WriteByte(0);
  buffer.WriteByte(1);
  buffer.WriteByte(127);

  EXPECT_EQ(std::string("\x80\xFF\0\x1\x7F", 5), out);
}

TEST(StringWriterTest, WriteLEI16) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int16_t>(std::numeric_limits<int16_t>::min());
    EXPECT_EQ(std::string("\0\x80", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int16_t>(0);
    EXPECT_EQ(std::string("\0\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int16_t>(1);
    EXPECT_EQ(std::string("\x1\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int16_t>(std::numeric_limits<int16_t>::max());
    EXPECT_EQ("\xFF\x7F", out);
  }
}

TEST(StringWriterTest, WriteLEU16) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint16_t>(0);
    EXPECT_EQ(std::string("\0\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint16_t>(1);
    EXPECT_EQ(std::string("\x1\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint16_t>(
        static_cast<uint16_t>(std::numeric_limits<int16_t>::max()) + 1);
    EXPECT_EQ(std::string("\0\x80", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint16_t>(std::numeric_limits<uint16_t>::max());
    EXPECT_EQ("\xFF\xFF", out);
  }
}

TEST(StringWriterTest, WriteLEI32) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int32_t>(std::numeric_limits<int32_t>::min());
    EXPECT_EQ(std::string("\0\0\0\x80", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int32_t>(0);
    EXPECT_EQ(std::string("\0\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int32_t>(1);
    EXPECT_EQ(std::string("\x1\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int32_t>(std::numeric_limits<int32_t>::max());
    EXPECT_EQ("\xFF\xFF\xFF\x7F", out);
  }
}

TEST(StringWriterTest, WriteLEU32) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint32_t>(0);
    EXPECT_EQ(std::string("\0\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint32_t>(1);
    EXPECT_EQ(std::string("\x1\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint32_t>(
        static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) + 1);
    EXPECT_EQ(std::string("\0\0\0\x80", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<uint32_t>(std::numeric_limits<uint32_t>::max());
    EXPECT_EQ("\xFF\xFF\xFF\xFF", out);
  }
}

TEST(StringWriterTest, WriteLEI64) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int64_t>(std::numeric_limits<int64_t>::min());
    EXPECT_EQ(std::string("\0\0\0\0\0\0\0\x80", 8), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int64_t>(1);
    EXPECT_EQ(std::string("\x1\0\0\0\0\0\0\0", 8), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int64_t>(0);
    EXPECT_EQ(std::string("\0\0\0\0\0\0\0\0", 8), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteLE<int64_t>(std::numeric_limits<int64_t>::max());
    EXPECT_EQ("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x7F", out);
  }
}

TEST(StringWriterTest, WriteBEI16) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int16_t>(std::numeric_limits<int16_t>::min());
    EXPECT_EQ(std::string("\x80\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int16_t>(0);
    EXPECT_EQ(std::string("\0\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int16_t>(1);
    EXPECT_EQ(std::string("\0\x1", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int16_t>(std::numeric_limits<int16_t>::max());
    EXPECT_EQ("\x7F\xFF", out);
  }
}

TEST(StringWriterTest, WriteBEU16) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint16_t>(0);
    EXPECT_EQ(std::string("\0\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint16_t>(1);
    EXPECT_EQ(std::string("\0\x1", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint16_t>(
        static_cast<uint16_t>(std::numeric_limits<int16_t>::max()) + 1);
    EXPECT_EQ(std::string("\x80\0", 2), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint16_t>(std::numeric_limits<uint16_t>::max());
    EXPECT_EQ("\xFF\xFF", out);
  }
}

TEST(StringWriterTest, WriteBEI32) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int32_t>(std::numeric_limits<int32_t>::min());
    EXPECT_EQ(std::string("\x80\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int32_t>(0);
    EXPECT_EQ(std::string("\0\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int32_t>(1);
    EXPECT_EQ(std::string("\0\0\0\x1", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int32_t>(std::numeric_limits<int32_t>::max());
    EXPECT_EQ("\x7F\xFF\xFF\xFF", out);
  }
}

TEST(StringWriterTest, WriteBEU32) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint32_t>(0);
    EXPECT_EQ(std::string("\0\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint32_t>(1);
    EXPECT_EQ(std::string("\0\0\0\x1", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint32_t>(
        static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) + 1);
    EXPECT_EQ(std::string("\x80\0\0\0", 4), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<uint32_t>(std::numeric_limits<uint32_t>::max());
    EXPECT_EQ("\xFF\xFF\xFF\xFF", out);
  }
}
TEST(StringWriterTest, WriteBEI64) {
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int64_t>(std::numeric_limits<int64_t>::min());
    EXPECT_EQ(std::string("\x80\0\0\0\0\0\0\0\0", 8), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int64_t>(1);
    EXPECT_EQ(std::string("\0\0\0\0\0\0\0\x1", 8), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int64_t>(0);
    EXPECT_EQ(std::string("\0\0\0\0\0\0\0\0", 8), out);
  }
  {
    std::string out;
    StringWriter buffer(out);
    buffer.WriteBE<int64_t>(std::numeric_limits<int64_t>::max());
    EXPECT_EQ("\x7F\xFF\xFF\xFF\xFF\xFF\xFF\xFF", out);
  }
}

}  // namespace hessian2
