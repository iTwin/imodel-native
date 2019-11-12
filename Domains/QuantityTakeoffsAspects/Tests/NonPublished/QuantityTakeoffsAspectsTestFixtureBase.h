/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

        static Dgn::SpatialCategoryCPtr CreateAndInsertCategory(Dgn::DgnDbR db, Utf8StringCR name);
        static Dgn::PhysicalModelPtr CreateAndInsertModel(Dgn::DgnDbR db, Utf8StringCR name);
        static Dgn::GenericPhysicalObjectPtr CreateAndInsertObject(Dgn::DgnDbR db);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
