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
#include <QuantityTakeoffsAspects/Elements/PerimeterAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for PerimeterAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct PerimeterAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        PerimeterAspectTestFixture() {};
        ~PerimeterAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PerimeterAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    double perimeter1 = 100.5f;

    PerimeterAspectPtr aspect = PerimeterAspect::Create(perimeter1);
    ASSERT_TRUE(aspect.IsValid());

    PerimeterAspect::SetAspect(*object, *aspect);

    object->Update();

    PerimeterAspectPtr aspect2 = PerimeterAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(perimeter1, aspect2->GetPerimeter());

    double perimeter2 = 75.95f;

    aspect2->SetPerimeter(perimeter2);

    object->Update();
    PerimeterAspectCPtr aspect3 = PerimeterAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(perimeter2, aspect3->GetPerimeter());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE