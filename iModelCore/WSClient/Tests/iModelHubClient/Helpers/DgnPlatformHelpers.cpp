/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/DgnPlatformHelpers.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnCoreAPI.h>
#include "DgnPlatformHelpers.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
Dgn::DgnCategoryId CreateCategory(Utf8CP name, DgnDbR db)
    {
    SpatialCategory category(db.GetDictionaryModel(), name, DgnCategory::Rank::User, "");

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    auto persistentCategory = category.Insert(appearance);
    BeAssert(persistentCategory.IsValid());

    return persistentCategory->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 11/2016
//---------------------------------------------------------------------------------------
PhysicalPartitionPtr CreateModeledElement(Utf8CP name, DgnDbR db)
    {
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), name);
    EXPECT_TRUE(partition.IsValid());
    auto result = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::Success, result);
    return partition;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 11/2016
//---------------------------------------------------------------------------------------
DgnElementCPtr CreateAndInsertModeledElement(Utf8CP name, DgnDbR db)
    {
    auto modeledElement = CreateModeledElement(name, db);
    auto persistentModeledElement = modeledElement->Insert();
    EXPECT_TRUE(persistentModeledElement.IsValid());
    return persistentModeledElement;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Andrius.Zonys                 11/2016
//---------------------------------------------------------------------------------------
PhysicalModelPtr CreateModel(PhysicalPartitionCR partition, DgnDbR db)
    {
    PhysicalModelPtr model = PhysicalModel::Create(partition);
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().PrepareForModelInsert(req, *model, IBriefcaseManager::PrepareAction::Acquire));

    auto status = model->Insert();
    EXPECT_EQ(DgnDbStatus::Success, status);
    return DgnDbStatus::Success == status ? model : nullptr;
    }

PhysicalModelPtr CreateModel(Utf8CP name, DgnDbR db)
    {
    PhysicalPartitionPtr partition = CreateModeledElement(name, db);
    EXPECT_TRUE(partition->Insert().IsValid());

    PhysicalModelPtr model = CreateModel(*partition, db);
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db.SaveChanges());
    return model;
    }

DgnElementPtr Create3dElement(DgnModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnCategoryId catId = (*SpatialCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
    return GenericPhysicalObject::Create(*model.ToPhysicalModelP(), catId);
    }

DgnElementPtr Create2dElement(DgnModelR model)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Annotation2d::GetHandler());
    DgnCategoryId catId = (*SpatialCategory::MakeIterator(db).begin()).GetId<DgnCategoryId>();
    return AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, model.GetModelId(), classId, catId));
    }

DgnElementCPtr CreateElement(DgnModelR model, bool acquireLocks)
    {
    auto elem = model.Is3d() ? Create3dElement(model) : Create2dElement(model);
    if (acquireLocks)
        {
        IBriefcaseManager::Request req;
        EXPECT_EQ(RepositoryStatus::Success, model.GetDgnDb().BriefcaseManager().PrepareForElementInsert(req, *elem, IBriefcaseManager::PrepareAction::Acquire));
        }

    auto persistentElem = elem->Insert();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, model.GetDgnDb().SaveChanges());
    return persistentElem;
    }

void OpenDgnDb(DgnDbPtr& db, BeFileNameCR path)
    {
    ASSERT_TRUE(path.DoesPathExist());
    db = DgnDb::OpenDgnDb(nullptr, path, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes));
    ASSERT_TRUE(db.IsValid());
    }

void OpenReadOnlyDgnDb(DgnDbPtr& db, BeFileNameCR path)
    {
    ASSERT_TRUE(path.DoesPathExist());
    EXPECT_TRUE(path.IsFileReadOnly());
    db = DgnDb::OpenDgnDb(nullptr, path, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    ASSERT_TRUE(db.IsValid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCode MakeStyleCode(Utf8CP name, DgnDbR db)
    {
    AnnotationTextStyle style(db);
    style.SetName(name);
    return style.GetCode();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnDbStatus InsertStyle(AnnotationTextStylePtr style, DgnDbR db, bool expectSuccess)
    {
    IBriefcaseManager::Request req;

    RepositoryStatus statusR = db.BriefcaseManager().PrepareForElementInsert(req, *style, IBriefcaseManager::PrepareAction::Acquire);
    EXPECT_EQ(expectSuccess, RepositoryStatus::Success == statusR);

    DgnDbStatus status;
    style->DgnElement::Insert(&status);
    return status;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnDbStatus InsertStyle(Utf8CP name, DgnDbR db, bool expectSuccess)
    {
    AnnotationTextStylePtr style;
    style = AnnotationTextStyle::Create(db);
    style->SetName(name);

    return InsertStyle(style, db, expectSuccess);
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
