#include <iostream>
#include <type_traits>

#include "gtest/gtest.h"
#include "hessian2/basic_codec/object_codec.hpp"
#include "hessian2/basic_codec/list_codec.hpp"
#include "hessian2/basic_codec/map_codec.hpp"
#include "hessian2/test_framework/decoder_test_framework.h"
#include "hessian2/test_framework/encoder_test_framework.h"

namespace Hessian2 {

TEST_F(TestDecoderFramework, DecoderJavaTestCaseForMap) {
  {
    Object::UntypedMap o;
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<UntypedMapObject>("replyUntypedMap_0", obj));
  }

  {
    Object::UntypedMap o;
    o.emplace(std::make_unique<StringObject>(absl::string_view("a")),
              std::make_unique<IntegerObject>(0));
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<UntypedMapObject>("replyUntypedMap_1", obj));
  }

  {
    Object::UntypedMap o;
    o.emplace(std::make_unique<IntegerObject>(0),
              std::make_unique<StringObject>(absl::string_view("a")));
    o.emplace(std::make_unique<IntegerObject>(1),
              std::make_unique<StringObject>(absl::string_view("b")));
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<UntypedMapObject>("replyUntypedMap_2", obj));
  }

  {
    Object::UntypedMap o;
    Object::UntypedList o_list;
    o_list.emplace_back(std::make_unique<StringObject>(absl::string_view("a")));
    o.emplace(std::make_unique<UntypedListObject>(std::move(o_list)),
              std::make_unique<IntegerObject>(0));
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<UntypedMapObject>("replyUntypedMap_3", obj));
  }

  {
    Object::TypedMap o;
    o.type_name_ = "java.util.Hashtable";
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<TypedMapObject>("replyTypedMap_0", obj));
  }

  {
    Object::TypedMap o;
    o.type_name_ = "java.util.Hashtable";
    o.field_name_and_value_.emplace(
        std::make_unique<StringObject>(absl::string_view("a")),
        std::make_unique<IntegerObject>(0));
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<TypedMapObject>("replyTypedMap_1", obj));
  }

  {
    Object::TypedMap o;
    o.type_name_ = "java.util.Hashtable";
    o.field_name_and_value_.emplace(
        std::make_unique<IntegerObject>(0),
        std::make_unique<StringObject>(absl::string_view("a")));
    o.field_name_and_value_.emplace(
        std::make_unique<IntegerObject>(1),
        std::make_unique<StringObject>(absl::string_view("b")));
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<TypedMapObject>("replyTypedMap_2", obj));
  }

  {
    Object::TypedMap o;
    Object::UntypedList o_list;
    o_list.emplace_back(std::make_unique<StringObject>(absl::string_view("a")));
    o.type_name_ = "java.util.Hashtable";
    o.field_name_and_value_.emplace(
        std::make_unique<UntypedListObject>(std::move(o_list)),
        std::make_unique<IntegerObject>(0));
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Decode<TypedMapObject>("replyTypedMap_3", obj));
  }
}

TEST_F(TestEncoderFramework, EncoderJavaTestCaseForMap) {
  {
    Object::UntypedMap o;
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<UntypedMapObject>("argUntypedMap_0", obj));
  }

  {
    Object::UntypedMap o;
    o.emplace(std::make_unique<StringObject>(absl::string_view("a")),
              std::make_unique<IntegerObject>(0));
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<UntypedMapObject>("argUntypedMap_1", obj));
  }

  {
    Object::UntypedMap o;
    o.emplace(std::make_unique<IntegerObject>(0),
              std::make_unique<StringObject>(absl::string_view("a")));
    o.emplace(std::make_unique<IntegerObject>(1),
              std::make_unique<StringObject>(absl::string_view("b")));
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<UntypedMapObject>("argUntypedMap_2", obj));
  }

  {
    Object::UntypedMap o;
    Object::UntypedList o_list;
    o_list.emplace_back(std::make_unique<StringObject>(absl::string_view("a")));
    o.emplace(std::make_unique<UntypedListObject>(std::move(o_list)),
              std::make_unique<IntegerObject>(0));
    UntypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<UntypedMapObject>("argUntypedMap_3", obj));
  }

  {
    Object::TypedMap o;
    o.type_name_ = "java.util.Hashtable";
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<TypedMapObject>("argTypedMap_0", obj));
  }

  {
    Object::TypedMap o;
    o.type_name_ = "java.util.Hashtable";
    o.field_name_and_value_.emplace(
        std::make_unique<StringObject>(absl::string_view("a")),
        std::make_unique<IntegerObject>(0));
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<TypedMapObject>("argTypedMap_1", obj));
  }

  {
    Object::TypedMap o;
    o.type_name_ = "java.util.Hashtable";
    o.field_name_and_value_.emplace(
        std::make_unique<IntegerObject>(0),
        std::make_unique<StringObject>(absl::string_view("a")));
    o.field_name_and_value_.emplace(
        std::make_unique<IntegerObject>(1),
        std::make_unique<StringObject>(absl::string_view("b")));
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<TypedMapObject>("argTypedMap_2", obj));
  }

  {
    Object::TypedMap o;
    Object::UntypedList o_list;
    o_list.emplace_back(std::make_unique<StringObject>(absl::string_view("a")));
    o.type_name_ = "java.util.Hashtable";
    o.field_name_and_value_.emplace(
        std::make_unique<UntypedListObject>(std::move(o_list)),
        std::make_unique<IntegerObject>(0));
    TypedMapObject obj(std::move(o));
    EXPECT_TRUE(Encode<TypedMapObject>("argTypedMap_3", obj));
  }
}

// A map keeps the first of two equal keys and destroys the second one, which
// already has a reference number. A reference to it fails the decode.
TEST(MapCodecTest, ReferenceToDroppedDuplicateKeyInUntypedMap) {
  // H { [] : 0, [] : 0, ref(2) : 0 } Z. Reference 0 is the map, 1 the first
  // key and 2 the duplicate key.
  const std::string in("\x48\x78\x90\x78\x90\x51\x92\x90\x5a", 9);
  Decoder decoder(in);
  EXPECT_EQ(nullptr, decoder.decode<Object>());
}

TEST(MapCodecTest, ReferenceToDroppedDuplicateKeyInTypedMap) {
  // M "T" { [] : 0, [] : 0, ref(2) : 0 } Z.
  const std::string in("\x4d\x01\x54\x78\x90\x78\x90\x51\x92\x90\x5a", 11);
  Decoder decoder(in);
  EXPECT_EQ(nullptr, decoder.decode<Object>());
}

TEST(MapCodecTest, ReferenceToDroppedDuplicateValue) {
  // H { 1 : [], 1 : [], 2 : ref(2) } Z. The value of the dropped entry is
  // reference 2.
  const std::string in("\x48\x91\x78\x91\x78\x92\x51\x92\x5a", 9);
  Decoder decoder(in);
  EXPECT_EQ(nullptr, decoder.decode<Object>());
}

TEST(MapCodecTest, ReferenceToEarlierValueStillWorks) {
  // H { 1 : [], 2 : ref(1) } Z.
  const std::string in("\x48\x91\x78\x92\x51\x91\x5a", 7);
  Decoder decoder(in);
  auto output = decoder.decode<Object>();
  ASSERT_NE(nullptr, output);
  const auto& map = output->toUntypedMap().value().get();
  auto it = map.find(ObjectPtr(std::make_unique<IntegerObject>(2)));
  ASSERT_NE(map.end(), it);
  ASSERT_EQ(Object::Type::Ref, it->second->type());
  EXPECT_EQ(Object::Type::UntypedList, it->second->toRefDest().value()->type());
}

TEST(MapCodecTest, ReferenceToMapBeingDecoded) {
  // H { 1 : ref(0) } Z.
  const std::string in("\x48\x91\x51\x90\x5a", 5);
  Decoder decoder(in);
  auto output = decoder.decode<Object>();
  ASSERT_NE(nullptr, output);
  const auto& map = output->toUntypedMap().value().get();
  auto it = map.find(ObjectPtr(std::make_unique<IntegerObject>(1)));
  ASSERT_NE(map.end(), it);
  EXPECT_EQ(output.get(), it->second->toRefDest().value());
}

// The caller owns each decoded object and may destroy it before decoding the
// next one with the same decoder.
TEST(MapCodecTest, ReferenceToObjectDestroyedByCaller) {
  // [] then H { ref(0) : 0 } Z.
  const std::string in("\x78\x48\x51\x90\x90\x5a", 6);
  Decoder decoder(in);
  ASSERT_NE(nullptr, decoder.decode<Object>());
  EXPECT_EQ(nullptr, decoder.decode<Object>());
}

TEST(MapCodecTest, RefObjectOutlivesItsTarget) {
  // [] then ref(0).
  const std::string in("\x78\x51\x90", 3);
  Decoder decoder(in);
  auto list = decoder.decode<Object>();
  ASSERT_NE(nullptr, list);
  auto ref = decoder.decode<Object>();
  ASSERT_NE(nullptr, ref);
  ASSERT_EQ(list.get(), ref->toRefDest().value());
  list.reset();
  EXPECT_FALSE(ref->toRefDest().has_value());
  EXPECT_EQ(0, ref->hash());
  EXPECT_EQ("Type: Ref, target destroyed", ref->toDebugString());
}

TEST(MapCodecTest, EncodeRefObjectWhoseTargetIsDestroyed) {
  // [] then ref(0).
  const std::string in("\x78\x51\x90", 3);
  Decoder decoder(in);
  auto list = decoder.decode<Object>();
  ASSERT_NE(nullptr, list);
  auto ref = decoder.decode<Object>();
  ASSERT_NE(nullptr, ref);
  list.reset();
  std::string out;
  Encoder encoder(out);
  EXPECT_FALSE(encoder.encode<Object>(*ref));
}

}  // namespace Hessian2
