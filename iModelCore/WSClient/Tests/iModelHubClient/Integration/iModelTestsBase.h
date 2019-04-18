/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "Helpers.h"
#include "IntegrationTestsBase.h"
#include "RequestBehaviorOptions.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
::testing::TestInfo const& GetTestInfo();
struct iModelTestsBase : public IntegrationTestsBase
{
protected:
    static iModelConnectionPtr  s_connection;
    static iModelInfoPtr        s_info;
    iModelInfoPtr               m_info;

public:
    static void SetUpTestCase(RequestBehaviorOptions behaviourOptions = RequestBehaviorOptions());
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    static Utf8String TestCaseiModelName();
    static void AcquireAndOpenBriefcase(BriefcasePtr& briefcase, iModelInfoPtr info, bool pull = true);
    static BriefcasePtr AcquireAndOpenBriefcase(bool pull = true);
    void CreateNonAdminConnection(iModelConnectionPtr& connection, iModelInfoPtr info);
    iModelConnectionPtr CreateNonAdminConnection();
    void CreateTestiModel();
};
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
