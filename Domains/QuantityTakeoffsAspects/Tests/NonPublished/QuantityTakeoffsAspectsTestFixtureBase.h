/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingShared/SharedRepositoryManagerTestBase.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct QuantityTakeoffsAspectsTestFixtureBase : BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::SharedRepositoryManagerTestBase
    {
    public:
        BeSQLite::L10N::SqlangFiles _GetApplicationSqlangs() override;
        virtual void RegisterDomains() override;
        virtual BeFileName GetGivenProjectPath() override;
        virtual void CopyFile(BeFileName projectPath) override;
        static void SetUpTestCase ();
        static Dgn::DgnDbR GetDgnDb();

        void SetUp() override;
        void TearDown() override;
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
