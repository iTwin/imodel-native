/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"
#include <Formatting/AliasMappings.h>

USING_BENTLEY_FORMATTING
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct MappingTest : FormattingTestFixture {};

bvector<Utf8String> names = 
    { 
    "DefaultReal",
    "DefaultRealU",
    "Real2",
    "Real3",
    "Real4",
    "Real2U",
    "Real3U",
    "Real4U",
    "Real6U",
    "Real2UNS",
    "Real3UNS",
    "Real4UNS",
    "Real6UNS",
    "Stop100-2u",
    "Stop100-2-4u",
    "Stop1000-2-4u",
    "Stop1000-2u",
    "Stop100-2",
    "Stop100-2-4",
    "Stop1000-2-4",
    "Stop1000-2",
    "SignedReal",
    "ParenthsReal",
    "DefaultFractional",
    "DefaultFractionalU",
    "SignedFractional",
    "DefaultExp",
    "SignedExp",
    "NormalizedExp",
    "DefaultInt",
    "Fractional4",
    "Fractional8",
    "Fractional16",
    "Fractional32",
    "Fractional128",
    "Fractional4U",
    "Fractional8U",
    "Fractional16U",
    "Fractional32U",
    "Fractional128U",
    "AngleDMS",
    "AngleDMS8",
    "AngleDM8",
    "AmerMYFI4",
    "AmerFI8",
    "AmerFI16",
    "AmerFI32",
    "AmerYFI8",
    "Meters4u",
    "Feet4u",
    "Inches4u",
    "Inches18u",
    "DecimalDeg4",
    "StationFt2",
    "StationM4",
    "DefaultReal",
    "DefaultRealU",
    "Real2",
    "Real3",
    "Real4",
    "Real2U",
    "Real3U",
    "Real4U",
    "Real6U",
    "Stop100-2u",
    "Stop100-2uz",
    "Stop100-2-2z",
    "Stop1000-2-3z",
    "Stop100-2-4u",
    "Stop1000-2-4u",
    "Stop1000-2u",
    "Stop100-2",
    "Stop100-2-4",
    "Stop1000-2-4",
    "Stop1000-2",
    "SignedReal",
    "ParenthsReal",
    "DefaultFractional",
    "DefaultFractionalU",
    "SignedFractional",
    "DefaultExp",
    "SignedExp",
    "NormalizedExp",
    "DefaultInt",
    "Fractional4",
    "Fractional8",
    "Fractional16",
    "Fractional32",
    "Fractional64",
    "Fractional128",
    "Fractional4U",
    "Fractional8U",
    "Fractional16U",
    "Fractional64U",
    "Fractional32U",
    "Fractional128U",
    "CAngleDMS",
    "CAngleDMS8",
    "CAngleDM8",
    "AmerMYFI4",
    "HMS"
    };

bvector<bpair<Utf8String, Utf8String>> namesThatMapToFormatsWithOverrides =
    {
    bpair<Utf8String, Utf8String>("Meters4u", "FORMATS:DefaultRealUNS(4)[u:M|m]"),
    bpair<Utf8String, Utf8String>("Feet4u", "FORMATS:DefaultRealUNS(4)[u:FT|']"),
    bpair<Utf8String, Utf8String>("Inches4u", "FORMATS:DefaultRealUNS(4)[u:IN|&quot;]"),
    bpair<Utf8String, Utf8String>("DecimalDeg4", "FORMATS:DefaultRealUNS(4)[u:ARC_DEG|\xC2\xB0]"),
    bpair<Utf8String, Utf8String>("Inches18u", "FORMATS:DefaultRealUNS(8)[u:IN|&quot;]")
    };

bvector<Utf8String> aliases = 
    {
    "real",
    "realu",
    "real2",
    "real3",
    "real4",
    "real2u",
    "real3u",
    "real4u",
    "real6u",
    "real2uns",
    "real3uns",
    "real4uns",
    "real6uns",
    "stop100-2u",
    "stop100-2-4u",
    "stop1000-2-4u",
    "stop1000-2u",
    "stop100-2",
    "stop100-2-4",
    "stop1000-2-4",
    "stop1000-2",
    "realSign",
    "realPth",
    "fract",
    "fractu",
    "fractSign",
    "sci",
    "sciSign",
    "sciN",
    "int",
    "fract4",
    "fract8",
    "fract16",
    "fract32",
    "fract128",
    "fract4u",
    "fract8u",
    "fract16u",
    "fract32u",
    "fract128u",
    "dms",
    "dms8",
    "dm8",
    "myfi4",
    "fi8",
    "fi16",
    "fi32",
    "yfi8",
    "meters4u",
    "feet4u",
    "inches4u",
    "Inches18u",
    "decimalDeg4",
    "stationFt2",
    "stationM4",
    "real",
    "realu",
    "real2",
    "real3",
    "real4",
    "real2u",
    "real3u",
    "real4u",
    "real6u",
    "stop100-2u",
    "stop100-2uz",
    "stop100-2-2z",
    "stop1000-2-3z",
    "stop100-2-4u",
    "stop1000-2-4u",
    "stop1000-2u",
    "stop100-2",
    "stop100-2-4",
    "stop1000-2-4",
    "stop1000-2",
    "realSign",
    "realPth",
    "fract",
    "fractu",
    "fractSign",
    "sci",
    "sciSign",
    "sciN",
    "int",
    "fract4",
    "fract8",
    "fract16",
    "fract32",
    "fract64",
    "fract128",
    "fract4u",
    "fract8u",
    "fract16u",
    "fract32u",
    "fract64u",
    "fract128u",
    "cdms",
    "cdms8",
    "cdm8",
    "myfi4",
    "hms",
    };

// Legacy names that are not the only one mapping to a single FormatString.
bvector<Utf8String> onlyNames = 
    {
    "Real",
    "DefaultInt",
    "RealU",
    "Real6U",
    "Fractional64",
    "Fractional64U",
    "CAngleDMS",
    "CAngleDMS8",
    "CAngleDM8"
    };

bvector<Utf8String> namesWithoutFormatString =
    {
    "Stop100-2",
    "Stop100-2u",
    "Stop100-2uz",
    "Stop100-2-4",
    "Stop100-2-4u",
    "Stop1000-2",
    "Stop1000-2u",
    "Stop1000-2-4",
    "Stop1000-2-4u",
    "SignedReal",
    "ParenthsReal",
    "DefaultFractionalU",
    "Fractional64U",
    "SignedFractional",
    "Fractional4U",
    "Fractional8U",
    "Fractional16U",
    "Fractional32U",
    "Fractional128U",
    "DefaultExp",
    "SignedExp",
    "NormalizedExp",
    "AngleDM8",
    "CAngleDM8",
    "AmerMYFI4",
    "AmerYFI8",
    "StationFt2",
    "StationM4"
    };

bool IsNameWithoutFormatString(Utf8CP tmpName)
    {
    for (auto const& name : namesWithoutFormatString)
        {
        if (0 == name.CompareToI(tmpName))
            return true;
        }
    return false;
    }

bool IsNameThatDoesNotMap(Utf8CP tmpName)
    {
    for (auto const& name : onlyNames)
        {
        if (0 == name.CompareToI(tmpName))
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MappingTest, AliasFormatString)
    {
    for (auto const& name : names)
        {
        auto formatString = LegacyNameMappings::TryGetFormatStringFromLegacyName(name.c_str());
        if (IsNameWithoutFormatString(name.c_str()))
            {
            EXPECT_TRUE(Utf8String::IsNullOrEmpty(formatString)) << "The name " << name.c_str() << " should not have a format string mapping.";
            continue;
            }

        EXPECT_FALSE(Utf8String::IsNullOrEmpty(formatString)) << "Did not find a FormatString mapped to the LegacyName '" << name.c_str() << "'.";

        // Skip the reverse. The name doesn't map back from FormatString -> LegacyName because there are two potential LegacyNames.
        if (IsNameThatDoesNotMap(name.c_str()))
            continue;

        EXPECT_STREQ(name.c_str(), LegacyNameMappings::TryGetLegacyNameFromFormatString(formatString)) << "The mapped FormatString '" << formatString << "' does not map back to the original name '" << name.c_str() << "'.";
        }
    
    for (auto const& alias : aliases)
        {
        auto name = AliasMappings::TryGetNameFromAlias(alias.c_str());
        EXPECT_FALSE(Utf8String::IsNullOrEmpty(name)) << "Did not find a Name mapped to the Alias '" << alias.c_str() << "'.";
        EXPECT_STREQ(alias.c_str(), AliasMappings::TryGetAliasFromName(name)) << "The mapped name '" << name << "' does not map back to the original alias '" << alias.c_str() << "'.";
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(MappingTest, RoundTripMappingsWithFormatOverrides)
    {
    for (auto const& fusAndFormat : namesThatMapToFormatsWithOverrides)
        {
        auto formatString = LegacyNameMappings::TryGetFormatStringFromLegacyName(fusAndFormat.first.c_str());
        EXPECT_STREQ(fusAndFormat.second.c_str(), formatString) << "The fus " << fusAndFormat.first.c_str() << " did not map to the expected format string";

        auto fus = LegacyNameMappings::TryGetLegacyNameFromFormatString(formatString);
        EXPECT_STREQ(fusAndFormat.first.c_str(), fus) << "The format string " << formatString << " did not map back to the original format string";
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(MappingTest, MappingNotFound)
    {
    EXPECT_EQ(nullptr, AliasMappings::TryGetAliasFromName("NotAName"));
    EXPECT_EQ(nullptr, AliasMappings::TryGetNameFromAlias("NotAnAlias"));
    EXPECT_EQ(nullptr, LegacyNameMappings::TryGetFormatStringFromLegacyName("NotALegacyName"));
    EXPECT_EQ(nullptr, LegacyNameMappings::TryGetLegacyNameFromFormatString("NotAFormatString"));
    }

END_BENTLEY_FORMATTEST_NAMESPACE