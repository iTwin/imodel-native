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
#include <QuantityTakeoffsAspects/Elements/ThicknessAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for ThicknessAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct ThicknessAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        ThicknessAspectTestFixture() {};
        ~ThicknessAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ThicknessAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    double thickness = 7.66f;

    ThicknessAspectPtr aspect = ThicknessAspect::Create(thickness);
    ASSERT_TRUE(aspect.IsValid());

    ThicknessAspect::SetAspect(*object, *aspect);

    object->Update();

    ThicknessAspectCPtr aspect2 = ThicknessAspect::GetCP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(thickness, aspect2->GetThickness());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE