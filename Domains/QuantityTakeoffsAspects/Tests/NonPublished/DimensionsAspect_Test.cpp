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
#include <QuantityTakeoffsAspects/Elements/DimensionsAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for DimensionsAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct DimensionsAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        DimensionsAspectTestFixture() {};
        ~DimensionsAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DimensionsAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    Point3d dimensions1{1, 2, 3};

    DimensionsAspectPtr aspect = DimensionsAspect::Create(dimensions1.x, dimensions1.y, dimensions1.z);
    ASSERT_TRUE(aspect.IsValid());

    DimensionsAspect::SetAspect(*object, *aspect);

    object->Update();

    DimensionsAspectPtr aspect2 = DimensionsAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(dimensions1.x, aspect2->GetLength());
    ASSERT_EQ(dimensions1.y, aspect2->GetWidth());
    ASSERT_EQ(dimensions1.z, aspect2->GetHeight());

    Point3d dimensions2{ 7, 8, 9 };

    aspect2->SetLength(dimensions2.x);
    aspect2->SetWidth(dimensions2.y);
    aspect2->SetHeight(dimensions2.z);

    ASSERT_EQ(dimensions2.x, aspect2->GetLength());
    ASSERT_EQ(dimensions2.y, aspect2->GetWidth());
    ASSERT_EQ(dimensions2.z, aspect2->GetHeight());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE