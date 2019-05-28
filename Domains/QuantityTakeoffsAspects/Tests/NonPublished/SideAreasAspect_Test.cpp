/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "QuantityTakeoffsAspectsTestFixtureBase.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <BeSQLite/BeSQLite.h>
#include <QuantityTakeoffsAspects/Elements/SideAreasAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for SideAreasAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct SideAreasAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        SideAreasAspectTestFixture() {};
        ~SideAreasAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SideAreasAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    SideAreasAspect::SideAreas netAreas{1.0f, 1.0f, 2.0f, 2.0f};
    SideAreasAspect::SideAreas grossAreas{ 10.0f, 10.0f, 15.0f, 15.0f };

    SideAreasAspectPtr aspect = SideAreasAspect::Create(netAreas, grossAreas);
    ASSERT_TRUE(aspect.IsValid());

    SideAreasAspect::SetAspect(*object, *aspect);

    object->Update();

    SideAreasAspectCPtr aspect2 = SideAreasAspect::GetCP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(netAreas.bottom, aspect2->GetNetAreas()->bottom);
    ASSERT_EQ(grossAreas.bottom, aspect2->GetGrossAreas()->bottom);

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE