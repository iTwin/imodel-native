/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/LocaleTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct LocaleTests : public ::testing::Test {};

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocaleTests, LocaleTest)
    {
    LOG.infov("\n================  Locale Test ===========================");

    LocaleProperties lop = LocaleProperties::DefaultAmerican();
    Json::Value jval = lop.ToJson();
    LOG.infov("American Default %s", jval.ToString().c_str());
    LocaleProperties lop1 = LocaleProperties(jval);
    LOG.infov("American Default origin %s", lop.ToText().c_str());
    LOG.infov("American Default restor %s", lop1.ToText().c_str());

    lop = LocaleProperties::DefaultEuropean();
    jval = lop.ToJson();
    LOG.infov("European Default %s", jval.ToString().c_str());
    lop1 = LocaleProperties(jval);
    LOG.infov("European Default origin %s", lop.ToText().c_str());
    LOG.infov("European Default restor %s", lop1.ToText().c_str());

    lop = LocaleProperties::DefaultEuropean(true);
    jval = lop.ToJson();
    LOG.infov("European1 Default %s", jval.ToString().c_str());
    lop1 = LocaleProperties(jval);
    LOG.infov("European1 Default origin %s", lop.ToText().c_str());
    LOG.infov("European1 Default restor %s", lop1.ToText().c_str());

    LOG.infov("================  Locale Test (end) ===========================\n");

    LOG.infov("================  Formatting Log (completed) ===========================\n\n\n");
    }

END_BENTLEY_FORMATTEST_NAMESPACE
