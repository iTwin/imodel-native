/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <BeRapidJson/BeJsValue.h>
#include <cstring>

USING_NAMESPACE_BENTLEY

// Existing iModels hold numbers written by JsonCpp's "%#.17g" formatting, and nothing rewrites
// them. RapidJson must therefore keep recovering the identical double indefinitely, long after
// JsonCpp itself is gone. These tests deliberately depend only on RapidJson.
//
// They are the regression test for the kParseFullPrecisionFlag on BeJsDocument::Parse. Several
// entries below are values that RapidJson's DEFAULT (fast-path) number parser gets 1 ULP wrong;
// drop the flag and this file fails.
struct LegacyJsonNumberReadTests : ::testing::Test {};

static uint64_t BitsOf(double d)
    {
    uint64_t bits;
    memcpy(&bits, &d, sizeof(bits));
    return bits;
    }

static double ParseSingleNumber(Utf8CP numberText)
    {
    Utf8String json("{\"v\":");
    json.append(numberText).append("}");

    BeJsDocument doc(json);
    EXPECT_FALSE(doc.hasParseError()) << numberText;
    return doc["v"].asDouble();
    }

struct LegacyNumberCase
    {
    Utf8CP m_jsonCppText;
    uint64_t m_expectedBits;
    };

// Spellings emitted by JsonCpp's valueToString(double), paired with the doubles strtod() recovers.
static LegacyNumberCase const s_legacyNumbers[] =
    {
        {"6356794.7191953063",      0x41583fceae074bc0ull},
        {"6378120.0",               0x415854a200000000ull},
        {"1234567.8901234567",      0x4132d687e3df2180ull},
        {"-36.144100000000002",     0xc0421271de69ad43ull},
        {"0.10000000000000001",     0x3fb999999999999aull},
        {"0.33333333333333331",     0x3fd5555555555555ull},
        {"12345.678901234567",      0x40c81cd6e63c53d7ull},
        {"987654321.12345684",      0x41cd6f34588fcd6full},
        {"-2.8586293668882208",     0xc006de7912d15c5aull},
        {"1000000000000.0",         0x426d1a94a2000000ull},
        {"4.9406564584124655e-15",  0x3cf640306766bac8ull},
        // Counterexamples that the DEFAULT (fast-path) number parser gets 1 ULP wrong. These are
        // the reason BeJsDocument::Parse passes kParseFullPrecisionFlag; they are the regression
        // test for that flag. Do not remove them.
        {"9.9999999999999995e-21",  0x3bc79ca10c924223ull}, // JsonCpp's spelling of 1e-20
        {"50.516504499999996",      0x4049421cd1c7de50ull}, // from a real project-extents property
        {"-1.8287999999999998",     0xbffd42c3c9eecbfaull},
        {"0.09950371902099893",     0x3fb9791363068b55ull}, // from a skew-axis sphere in GeomLibs
    };

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(LegacyJsonNumberReadTests, JsonCppSpellings_RecoverBitIdenticalDoubles)
    {
    for (auto const& legacyCase : s_legacyNumbers)
        EXPECT_EQ(legacyCase.m_expectedBits, BitsOf(ParseSingleNumber(legacyCase.m_jsonCppText))) << legacyCase.m_jsonCppText;
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(LegacyJsonNumberReadTests, JsonCppSpellings_SurviveNesting)
    {
    Utf8CP legacy = R"({"high":[6356794.7191953063,1234567.8901234567,-2.8586293668882208],)"
                    R"("low":[-36.144100000000002,0.10000000000000001,4.9406564584124655e-15]})";

    BeJsDocument doc(legacy);
    ASSERT_FALSE(doc.hasParseError());

    EXPECT_EQ(0x41583fceae074bc0ull, BitsOf(doc["high"][0u].asDouble()));
    EXPECT_EQ(0x4132d687e3df2180ull, BitsOf(doc["high"][1u].asDouble()));
    EXPECT_EQ(0xc006de7912d15c5aull, BitsOf(doc["high"][2u].asDouble()));
    EXPECT_EQ(0xc0421271de69ad43ull, BitsOf(doc["low"][0u].asDouble()));
    EXPECT_EQ(0x3fb999999999999aull, BitsOf(doc["low"][1u].asDouble()));
    EXPECT_EQ(0x3cf640306766bac8ull, BitsOf(doc["low"][2u].asDouble()));
    }

//---------------------------------------------------------------------------------------
// The subnormal boundary is the sharpest case: RapidJson's default fast-path parser lands 1 ULP
// above strtod (0x0010000000000000 instead of 0x000fffffffffffff), turning the largest subnormal
// into the smallest normal. BeJsDocument::Parse passes kParseFullPrecisionFlag, so it agrees with
// strtod - which is what JsonCpp used, and therefore what every persisted iModel was written
// against. If this ever starts returning 0x0010000000000000 again, the flag has been dropped.
// @betest
//---------------------------------------------------------------------------------------
TEST_F(LegacyJsonNumberReadTests, SubnormalBoundary_MatchesStrtod)
    {
    EXPECT_EQ(0x000fffffffffffffull, BitsOf(ParseSingleNumber("2.2250738585072011e-308")));
    }
