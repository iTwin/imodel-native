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
#include <QuantityTakeoffsAspects/Elements/MaterialAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for MaterialAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct MaterialAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        MaterialAspectTestFixture() {};
        ~MaterialAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MaterialAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    Utf8String material1 = "Iron";
    double density1 = 7.35f;
    double weight1 = 60.0f;

    MaterialAspectPtr aspect = MaterialAspect::Create(material1, density1, weight1);
    ASSERT_TRUE(aspect.IsValid());

    MaterialAspect::SetAspect(*object, *aspect);

    object->Update();

    MaterialAspectPtr aspect2 = MaterialAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(material1, *aspect2->GetMaterial());
    ASSERT_EQ(density1, aspect2->GetMaterialDensity());
    ASSERT_EQ(weight1, aspect2->GetWeight());

    Utf8String material2 = "Gold";
    double density2 = 19.32f;
    double weight2 = 70.0f;

    aspect2->SetMaterial(material2);
    aspect2->SetMaterialDensity(density2);
    aspect2->SetWeight(weight2);

    object->Update();
    MaterialAspectCPtr aspect3 = MaterialAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(material2, *aspect3->GetMaterial());
    ASSERT_EQ(density2, aspect3->GetMaterialDensity());
    ASSERT_EQ(weight2, aspect3->GetWeight());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE