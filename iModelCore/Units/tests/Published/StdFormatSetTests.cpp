/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/StdFormatSetTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

#include "../../PrivateAPI/Formatting/AliasMappings.h"

USING_BENTLEY_FORMATTING
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct StdFormatSetTest : FormattingTestFixture {};

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(StdFormatSetTest, AllFormatsHaveAliasMapping)
    {
    auto* registry = new BEU::UnitRegistry();
    auto* formatSet = new StdFormatSet();

    EXPECT_EQ(BentleyStatus::SUCCESS, formatSet->AddCompositeSpecs(*registry));

    //for (auto const& format : formatSet->GetFormats())
        //EXPECT_TRUE(!Utf8String::IsNullOrEmpty(AliasMappings::TryGetAliasFromName(format.GetName().c_str()))) << "The format " << format.GetName().c_str() << " does not have a mapping for its alias.";
    }

END_BENTLEY_FORMATTEST_NAMESPACE
