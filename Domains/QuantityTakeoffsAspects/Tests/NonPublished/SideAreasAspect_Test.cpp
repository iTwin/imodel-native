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

    SideAreasAspect::SideAreas netAreas1{1.0f, 1.0f, 2.0f, 2.0f};
    SideAreasAspect::SideAreas grossAreas1{ 10.0f, 10.0f, 15.0f, 15.0f };

    SideAreasAspectPtr aspect = SideAreasAspect::Create(netAreas1, grossAreas1);
    ASSERT_TRUE(aspect.IsValid());

    SideAreasAspect::SetAspect(*object, *aspect);

    object->Update();

    SideAreasAspectPtr aspect2 = SideAreasAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(netAreas1.bottom, aspect2->GetNetAreas()->bottom);
    ASSERT_EQ(grossAreas1.bottom, aspect2->GetGrossAreas()->bottom);

    SideAreasAspect::SideAreas netAreas2{ 2.0f, 3.0f, 4.0f, 5.0f };
    SideAreasAspect::SideAreas grossAreas2{ 11.0f, 12.0f, 13.0f, 14.0f };

    double topGrossArea = 79.0f;
    double topNetArea = 80.0f;

    aspect2->SetNetAreas(netAreas2);
    aspect2->SetGrossAreas(grossAreas2);

    aspect2->SetTopGrossArea(topGrossArea);
    aspect2->SetTopNetArea(topNetArea);

    object->Update();
    SideAreasAspectCPtr aspect3 = SideAreasAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(netAreas2.bottom, aspect3->GetNetAreas()->bottom);
    ASSERT_EQ(grossAreas2.bottom, aspect3->GetGrossAreas()->bottom);
    ASSERT_EQ(topNetArea, aspect3->GetNetAreas()->top);
    ASSERT_EQ(topGrossArea, aspect3->GetGrossAreas()->top);

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE