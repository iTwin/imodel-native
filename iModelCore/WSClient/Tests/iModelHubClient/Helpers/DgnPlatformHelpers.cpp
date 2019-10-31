/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnCoreAPI.h>
#include "DgnPlatformHelpers.h"
#include <Bentley/BeTest.h>
#include <DgnPlatform/GenericDomain.h>

USING_NAMESPACE_BENTLEY_DGN
BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2019
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId CreateCategory(Utf8CP name, DgnDbR db)
    {
    SpatialCategory category(db.GetDictionaryModel(), name, DgnCategory::Rank::User, "");

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    if (db.IsBriefcase())
        {
        IBriefcaseManager::Request req;
        db.BriefcaseManager().AcquireForElementInsert(category);
        }

    auto persistentCategory = category.Insert(appearance);
    BeAssert(persistentCategory.IsValid());
    db.SaveChanges();

    return persistentCategory->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             01/2019
//---------------------------------------------------------------------------------------
Dgn::DgnCategoryId GetOrCreateDefaultCategory(DgnDbR db)
    {
    auto iterator = SpatialCategory::MakeIterator(db);

    // If category aleady exists - return it
    if (iterator.begin() != iterator.end())
        {
        return (*iterator.begin()).GetId<DgnCategoryId>();
        }

    return CreateCategory("DefaultCategory", db);
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

DgnElementPtr Create3dElement(DgnModelR model, DgnCodeCR code)
    {
    DgnDbR db = model.GetDgnDb();
    DgnCategoryId catId = GetOrCreateDefaultCategory(db);
    DgnClassId classId = model.GetDgnDb().Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());
    if (!classId.IsValid() || !catId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }
    GenericPhysicalObject::CreateParams createParams(db, model.GetModelId(), classId, catId, Placement3d(), code);
    return GenericPhysicalObject::Create(createParams);
    }

DgnElementPtr Create2dElement(DgnModelR model, DgnCodeCR code)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Annotation2d::GetHandler());
    DgnCategoryId catId = GetOrCreateDefaultCategory(db);
    if (!classId.IsValid() || !catId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }
    AnnotationElement2d::CreateParams createParams(db, model.GetModelId(), classId, catId, Placement2d(), code);
    return AnnotationElement2d::Create(createParams);
    }

DgnElementCPtr CreateElement(DgnModelR model, DgnCodeCR code, bool acquireLocks)
    {
    auto elem = model.Is3d() ? Create3dElement(model, code) : Create2dElement(model, code);
    if (acquireLocks)
        {
        IBriefcaseManager::Request req;
        EXPECT_EQ(RepositoryStatus::Success, model.GetDgnDb().BriefcaseManager().PrepareForElementInsert(req, *elem, IBriefcaseManager::PrepareAction::Acquire));
        }

    auto insertedElement = elem->Insert();
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, model.GetDgnDb().SaveChanges());
    return insertedElement;
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
    AnnotationTextStyle style(db.GetDictionaryModel());
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
    style = AnnotationTextStyle::Create(db.GetDictionaryModel());
    style->SetName(name);

    return InsertStyle(style, db, expectSuccess);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Andrius.Zonys                   04/2018
//---------------------------------------------------------------------------------------
void InsertSpatialView(SpatialModelR model, Utf8CP name, bool isPrivate)
    {
    DgnDbR db = model.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();
    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "");
    modelSelector->AddModel(model.GetModelId());

    OrthographicViewDefinitionPtr viewDef = new OrthographicViewDefinition(dictionary, name, *new CategorySelector(dictionary, ""), *new DisplayStyle3d(dictionary, ""), *modelSelector);
    ASSERT_TRUE(viewDef.IsValid());

    GetOrCreateDefaultCategory(db);
    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        viewDef->GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    viewDef->SetIsPrivate(isPrivate);
    viewDef->SetStandardViewRotation(StandardView::Iso);
    viewDef->LookAtVolume(model.QueryElementsRange());

    if (db.IsBriefcase())
        {
        IBriefcaseManager::Request req;
        db.BriefcaseManager().AcquireForElementInsert(*viewDef);
        }

    viewDef->Insert();
    ASSERT_TRUE(viewDef->GetViewId().IsValid());
    }
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
