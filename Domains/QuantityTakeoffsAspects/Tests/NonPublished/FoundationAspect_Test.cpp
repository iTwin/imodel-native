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
#include <QuantityTakeoffsAspects/Elements/FoundationAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for FoundationAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct FoundationAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        FoundationAspectTestFixture() {};
        ~FoundationAspectTestFixture() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(FoundationAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = CreateAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    FoundationType type1 = FoundationType::SpreadFooting;

    FoundationAspectPtr aspect = FoundationAspect::Create(type1);
    ASSERT_TRUE(aspect.IsValid());

    FoundationAspect::SetAspect(*object, *aspect);

    object->Update();

    FoundationAspectPtr aspect2 = FoundationAspect::GetP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(type1, aspect2->GetType());

    FoundationType type2 = FoundationType::StripFooting;

    aspect2->SetType(type2);

    object->Update();
    FoundationAspectCPtr aspect3 = FoundationAspect::GetCP(*object);
    ASSERT_TRUE(aspect3.IsValid());

    ASSERT_EQ(type2, aspect3->GetType());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE