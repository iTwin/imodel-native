/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/IntegrationTestsBase.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "Helpers.h"
#include "RequestBehaviorOptions.h"
#include <Bentley/BeTest.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
::testing::TestInfo const& GetTestInfo();
struct IntegrationTestsBase : public ::testing::Test
    {
    private:
        static BeFileName s_seed;
        static DgnCategoryId s_defaultCategoryId;
        static void CreateSeedDb();

    protected:
        Utf8String m_imodelName;
        static ClientPtr s_client;
        static Utf8String s_projectId;

    public:
        static void SetUpTestCase(RequestBehaviorOptions behaviourOptions = RequestBehaviorOptions());
        static void TearDownTestCase();
        virtual void SetUp() override;
        virtual void TearDown() override;

        static Dgn::DgnDbPtr CreateTestDb(Utf8StringCR name);

        static iModelResult CreateiModel(DgnDbPtr db, bool expectSuccess = true);
        static iModelConnectionPtr CreateiModelConnection(iModelInfoPtr info);

        virtual Utf8String GetTestiModelName();
        Dgn::DgnDbPtr CreateTestDb();
        static BeFileName OutputDir();
        Utf8String TestCodeName(int number = 0) const;
        ClientPtr CreateNonAdminClient() const;
        static IRepositoryManagerP _GetRepositoryManager(DgnDbR db);
    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
