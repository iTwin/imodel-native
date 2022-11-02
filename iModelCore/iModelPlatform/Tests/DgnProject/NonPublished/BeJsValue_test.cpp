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

TEST_F(BeJsValueTest, JsonCPP) {
  Json::Value test(Json::objectValue);
  auto thing = getThing(test);
  ASSERT_TRUE(thing.isNull());
  ASSERT_FALSE(test.hasMember("thing"));
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
