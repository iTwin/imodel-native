/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

struct BeJsValueTest : public DgnDbTestFixture {
};

static BeJsConst getThing(BeJsConst json) {
  return json["thing"];
}

BeJsDocument testMoveCtor() {
  BeJsDocument val;
  val["testName"] = "test value";
  return val;
}

TEST_F(BeJsValueTest, RapidJson) {
  BeJsDocument test;
  auto thing = getThing(test);
  ASSERT_TRUE(thing.isNull());
  ASSERT_FALSE(test.hasMember("thing"));

  test["thing"] = "thing val";
  auto test2 = std::move(test);
  auto thing2 = getThing(test2);
  ASSERT_FALSE(thing2.isNull());
  ASSERT_TRUE(test2.hasMember("thing"));
  ASSERT_FALSE(test.hasMember("thing"));

  auto testMoved = testMoveCtor();
  auto val = testMoved["testName"];
  ASSERT_TRUE(val.asString() == "test value");
}

// A uint64_t must be assignable to a BeJsValue and round-trip exactly. Without an explicit uint64_t
// overload the assignment is ambiguous across the double/bool/int32/uint32/int64 overloads.
TEST_F(BeJsValueTest, AssignUInt64) {
  BeJsDocument doc;

  uint64_t const large = 0xfedcba9876543210ull; // > INT64_MAX
  doc["large"] = large;
  EXPECT_EQ(large, doc["large"].asUInt64());

  uint64_t const beyondDouble = (1ull << 53) + 1; // not representable as a double
  doc["beyondDouble"] = beyondDouble;
  EXPECT_EQ(beyondDouble, doc["beyondDouble"].asUInt64());

  // BeInt64Id must still select the BeInt64Id overload and be stored as a hex string.
  doc["id"] = BeInt64Id(0x1fULL);
  EXPECT_TRUE(doc["id"].isString());
  EXPECT_STREQ(BeInt64Id(0x1fULL).ToHexStr().c_str(), doc["id"].asCString());

  // Assignment through a plain BeJsValue (not just BeJsDocument) must work too.
  BeJsValue nested = doc["nested"];
  nested["v"] = large;
  EXPECT_EQ(large, doc["nested"]["v"].asUInt64());
}

// BeJsValue::From must preserve 64-bit integers exactly. Copying them through double silently
// rounds anything above 2^53 and also changes how the value is subsequently serialized.
TEST_F(BeJsValueTest, FromPreservesLargeIntegers) {
  BeJsDocument src;
  src["int64Max"] = std::numeric_limits<int64_t>::max();
  src["int64Min"] = std::numeric_limits<int64_t>::lowest();
  src["beyondDouble"] = (uint64_t)((1ull << 53) + 1);
  src["uint64Max"] = std::numeric_limits<uint64_t>::max();
  src["small"] = (int32_t)42;
  src["real"] = 3.5;

  BeJsDocument copy;
  copy.From(src);

  EXPECT_EQ(std::numeric_limits<int64_t>::max(), copy["int64Max"].asInt64());
  EXPECT_EQ(std::numeric_limits<int64_t>::lowest(), copy["int64Min"].asInt64());
  EXPECT_EQ((uint64_t)((1ull << 53) + 1), copy["beyondDouble"].asUInt64());
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), copy["uint64Max"].asUInt64());
  EXPECT_EQ(42, copy["small"].asInt());
  EXPECT_DOUBLE_EQ(3.5, copy["real"].asDouble());

  // The serialized form must round-trip the full precision, not 9.2233720368547758e18.
  EXPECT_STREQ("9223372036854775807", copy["int64Max"].Stringify().c_str());
  EXPECT_STREQ("9007199254740993", copy["beyondDouble"].Stringify().c_str());
  EXPECT_STREQ("18446744073709551615", copy["uint64Max"].Stringify().c_str());

  // A copy of a copy must stay stable.
  BeJsDocument again;
  again.From(copy);
  EXPECT_STREQ(copy.Stringify().c_str(), again.Stringify().c_str());
}

// The public BeJsConst::GetInt64 wrapper must not perform its own double->int64 conversion:
// NaN and out-of-range doubles would be an undefined conversion.
TEST_F(BeJsValueTest, GetInt64ClampsAndRejectsNonFinite) {
  BeJsDocument doc;
  doc["nan"] = std::numeric_limits<double>::quiet_NaN();
  doc["inf"] = std::numeric_limits<double>::infinity();
  doc["negInf"] = -std::numeric_limits<double>::infinity();
  doc["huge"] = 1e300;
  doc["negHuge"] = -1e300;
  doc["truncates"] = 2.75;

  EXPECT_EQ(0, doc["nan"].asInt64(-1)) << "NaN converts to 0, as JsonCpp did";
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), doc["inf"].asInt64());
  EXPECT_EQ(std::numeric_limits<int64_t>::lowest(), doc["negInf"].asInt64());
  EXPECT_EQ(std::numeric_limits<int64_t>::max(), doc["huge"].asInt64());
  EXPECT_EQ(std::numeric_limits<int64_t>::lowest(), doc["negHuge"].asInt64());
  EXPECT_EQ(2, doc["truncates"].asInt64()) << "JsonCpp truncated toward zero";

  // The string special case must survive.
  doc["decimalString"] = "123";
  doc["hexString"] = "0x1f";
  EXPECT_EQ(123, doc["decimalString"].asInt64());
  EXPECT_EQ(31, doc["hexString"].asInt64());
}

// static_cast<double>(UINT64_MAX) rounds up to exactly 2^64, so the uint64 range check needs an
// exclusive 2^64 bound; otherwise a double equal to 2^64 is cast out of range.
TEST_F(BeJsValueTest, GetUInt64RejectsTwoToThe64) {
  BeJsDocument doc;
  doc["twoTo64"] = 18446744073709551616.0; // exactly 2^64
  doc["justBelow"] = 18446744073709549568.0; // largest double < 2^64
  doc["negative"] = -1.0;
  doc["nan"] = std::numeric_limits<double>::quiet_NaN();
  doc["fractional"] = 2.5;

  EXPECT_EQ(7u, doc["twoTo64"].asUInt64(7)) << "2^64 is out of range for uint64_t";
  EXPECT_EQ(18446744073709549568ull, doc["justBelow"].asUInt64(7));
  EXPECT_EQ(7u, doc["negative"].asUInt64(7));
  EXPECT_EQ(7u, doc["nan"].asUInt64(7));
  EXPECT_EQ(7u, doc["fractional"].asUInt64(7)) << "non-integral doubles yield the default";
}

// rapidjson's writer emits nothing for NaN/Infinity, which would splice invalid JSON such as
// {"x":} into StringifyLegacy's output. JsonCpp wrote `null`.
TEST_F(BeJsValueTest, StringifyLegacyWritesNullForNonFinite) {
  BeJsDocument doc;
  doc["a"] = std::numeric_limits<double>::quiet_NaN();
  doc["b"] = std::numeric_limits<double>::infinity();
  doc["c"] = -std::numeric_limits<double>::infinity();
  doc["d"] = 1.5;
  // BeJsLegacyDoubleToString deliberately keeps one trailing zero, matching JsonCpp.
  EXPECT_STREQ(R"({"a":null,"b":null,"c":null,"d":1.50})", doc.StringifyLegacy().c_str());

  BeJsDocument nested;
  nested["obj"]["inner"] = std::numeric_limits<double>::quiet_NaN();
  BeJsValue arr = nested["arr"];
  arr.toArray();
  arr[0] = std::numeric_limits<double>::infinity();
  arr[1] = 2.0;
  EXPECT_STREQ(R"({"arr":[null,2.0],"obj":{"inner":null}})", nested.StringifyLegacy().c_str());
}
