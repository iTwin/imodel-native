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
