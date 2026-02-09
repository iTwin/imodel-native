/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct PurgeNullTestFixture : public ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PurgeNullTestFixture, PurgeNulls) {
    Utf8String testInst = R"json({
      "id": "0x1234",
      "p2d": {
          "x": 341.34,
          "y": -4.322,
          "z": undefined
      },
      "bi": "encoding=base64;cd2DfQvyUAEK4Q==",
      "l": "",
  })json";

    Utf8String expectedInst = R"json({
      "id": "0x1234",
      "p2d": {
          "x": 341.34,
          "y": -4.322,
      },
      "bi": "encoding=base64;cd2DfQvyUAEK4Q==",
  })json";

    BeJsDocument testDoc;
    testDoc.Parse(testInst);
    BeJsDocument expectedDoc;
    expectedDoc.Parse(expectedInst);
    testDoc.PurgeNulls();
    ASSERT_STREQ(expectedDoc.Stringify(StringifyFormat::Indented).c_str(), testDoc.Stringify(StringifyFormat::Indented).c_str());
}

END_ECDBUNITTESTS_NAMESPACE