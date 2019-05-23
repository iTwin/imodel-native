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
#include <QuantityTakeoffsAspects/Elements/PileAspect.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

//=======================================================================================
// Sets up environment for PileAspect testing.
// @bsiclass                                    Elonas.Seviakovas                05/2019
//=======================================================================================
struct PileAspectTestFixture : public QuantityTakeoffsAspectsTestFixtureBase
    {
    public:
        PileAspectTestFixture() {};
        ~PileAspectTestFixture() {};
    };

SpatialCategoryCPtr createAndInsertCategory(DgnDbR db, Utf8StringCR name)
    {
    Dgn::DgnSubCategory::Appearance appearance;
    appearance.SetColor(Dgn::ColorDef::White());
    Dgn::SpatialCategory category(db.GetDictionaryModel(), name, Dgn::DgnCategory::Rank::Domain);
    return category.Insert(appearance);
    }

PhysicalModelPtr createAndInsertModel(DgnDbR db, Utf8StringCR name)
    {
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*rootSubject, name);
    db.Elements().Insert(*partition);
    return PhysicalModel::CreateAndInsert(*partition);
    }

GenericPhysicalObjectPtr createAndInsertObject(DgnDbR db)
    {
    PhysicalModelPtr model = createAndInsertModel(db, "TestModel");
    if (model.IsNull())
        return nullptr;

    SpatialCategoryCPtr category = createAndInsertCategory(db, "TestObject");
    if (category.IsNull())
        return nullptr;

    GenericPhysicalObjectPtr object = GenericPhysicalObject::Create(*model, category->GetCategoryId());
    if (object->Insert().IsNull())
        return nullptr;

    return object;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PileAspectTestFixture, Create)
    {
    DgnDbR db = GetDgnDb();
    db.BriefcaseManager().StartBulkOperation();

    GenericPhysicalObjectPtr object = createAndInsertObject(db);
    ASSERT_TRUE(object.IsValid());

    PileAspectPtr aspect = PileAspect::Create(5);
    ASSERT_TRUE(aspect.IsValid());

    PileAspect::SetAspect(*object, *aspect);

    object->Update();

    auto claSS = PileAspect::QueryECClass(db);
    auto name = claSS->GetFullName();

    PileAspectCPtr aspect2 = PileAspect::GetCP(*object);
    ASSERT_TRUE(aspect2.IsValid());

    ASSERT_EQ(5, aspect2->GetEmbedmentDepth());

    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE