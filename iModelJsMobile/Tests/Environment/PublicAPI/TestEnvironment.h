/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Environment/PublicAPI/TestEnvironment.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <iModelJs/iModelJs.h>
#include <iModelJs/iModelJsServicesTier.h>
#include <Bentley/BeTest.h>

#define BEGIN_IMODELJS_UNIT_TESTS_NAMESPACE    BEGIN_BENTLEY_IMODELJS_NAMESPACE namespace UnitTests {
#define END_IMODELJS_UNIT_TESTS_NAMESPACE      } END_BENTLEY_IMODELJS_NAMESPACE
#define USING_IMODELJS_UNIT_TESTS_NAMESPACE    using namespace BentleyApi::iModelJs::UnitTests;

BEGIN_IMODELJS_UNIT_TESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                    Steve.Wilson                8/17
//=======================================================================================
struct iModelJsTestFixture : public ::testing::Test
    {
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    bool m_standalone;

public:
    static void ResetFailedJsAssertCount();
    static size_t GetFailedJsAssertCount();
    static void InstallTestingUtilities (Js::RuntimeR);

    iModelJsTestFixture();
    ~iModelJsTestFixture() override;

    void SetUp() override;
    void TearDown() override;

    void RunSystemMessageLoop();
    bool IsStandalone() const { return m_standalone; }
    };

END_IMODELJS_UNIT_TESTS_NAMESPACE
