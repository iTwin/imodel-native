/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/DgnDbTestUtils/DgnDbTestUtils.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//=======================================================================================
// WARNING: Must be careful of dependencies within this file as the OBJ produced from this 
// WARNING:   source file is also used in DgnDisplayTests, DgnClientFxTests, etc.
//=======================================================================================

#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/Render.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

#define MUST_HAVE_HOST(BAD_RETURN) if (nullptr == DgnPlatformLib::QueryHost())\
        {\
        EXPECT_FALSE(true) << "Your TC_SETUP function must set up a host. Just put an instance of ScopedDgnHost on the stack at the top of your function.";\
        return BAD_RETURN;\
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnModelId DgnDbTestUtils::QueryFirstGeometricModelId(DgnDbR db)
    {
    for (auto const& modelEntry : db.Models().MakeIterator())
        {
        if ((DgnModel::RepositoryModelId() == modelEntry.GetModelId()) || (DgnModel::DictionaryId() == modelEntry.GetModelId()))
            continue;
        
        DgnModelPtr model = db.Models().GetModel(modelEntry.GetModelId());
        if (model->IsGeometricModel())
            return modelEntry.GetModelId();
        }

    BeAssert(false && "No GeometricModel found");
    return DgnModelId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DocumentListModelPtr DgnDbTestUtils::InsertDocumentListModel(DgnDbR db, DgnCodeCR modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SubjectCPtr modelSubject = Subject::CreateAndInsert(*rootSubject, Utf8PrintfString("Subject for %s", modelCode.GetValueCP()).c_str()); // create a placeholder Subject for this DgnModel to describe
    EXPECT_TRUE(modelSubject.IsValid());
    DocumentListModelPtr model = DocumentListModel::CreateAndInsert(*modelSubject, modelCode);
    EXPECT_TRUE(model.IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DrawingPtr DgnDbTestUtils::InsertDrawing(DocumentListModelCR model, DgnCodeCR code, Utf8CP label)
    {
    MUST_HAVE_HOST(nullptr);
    DrawingPtr drawing = Drawing::Create(model, code, label);
    EXPECT_TRUE(drawing.IsValid());
    EXPECT_TRUE(drawing->Insert().IsValid());
    return drawing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DrawingModelPtr DgnDbTestUtils::InsertDrawingModel(DrawingCR drawing, DgnCodeCR modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    DrawingModelPtr model = DrawingModel::Create(drawing, modelCode);
    EXPECT_TRUE(model.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, model->Insert());
    EXPECT_TRUE(model->GetModelId().IsValid());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                           Sam.Wilson             01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr DgnDbTestUtils::InsertPhysicalModel(DgnDbR db, DgnCodeCR modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SubjectCPtr modelSubject = Subject::CreateAndInsert(*rootSubject, Utf8PrintfString("Subject for %s", modelCode.GetValueCP()).c_str()); // create a placeholder Subject for this DgnModel to describe
    EXPECT_TRUE(modelSubject.IsValid());
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(*modelSubject, modelCode);
    EXPECT_TRUE(model.IsValid());
    return model;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                           Umar.Hayat             08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SheetModelPtr DgnDbTestUtils::InsertSheetModel(DgnDbR db, DgnCode modelCode)
    {
    MUST_HAVE_HOST(nullptr);
    SubjectCPtr rootSubject = db.Elements().GetRootSubject();
    SubjectCPtr modelSubject = Subject::CreateAndInsert(*rootSubject, modelCode.GetValueCP()); // create a placeholder Subject for this DgnModel to describe
    EXPECT_TRUE(modelSubject.IsValid());
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetModel));
    SheetModelPtr model = new SheetModel(SheetModel::CreateParams(db, mclassId, modelSubject->GetElementId(), modelCode, DPoint2d::From(2.0, 2.0)));
    DgnDbStatus status = model->Insert();
    EXPECT_EQ(DgnDbStatus::Success, status) << WPrintfString(L"%ls - insert into %ls failed with %x", modelCode.GetValue().c_str(), db.GetFileName().c_str(), (int)status).c_str();
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::UpdateProjectExtents(DgnDbR db)
    {
    AxisAlignedBox3d physicalExtents;
    physicalExtents = db.Units().ComputeProjectExtents();
    db.Units().SetProjectExtents(physicalExtents);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void DgnDbTestUtils::FitView(DgnDbR db, DgnViewId viewId)
    {
    SpatialViewDefinitionCPtr view = dynamic_cast<SpatialViewDefinitionCP>(ViewDefinition::QueryView(viewId, db).get());
    ASSERT_TRUE(view.IsValid());

    ViewControllerPtr viewController = view->LoadViewController(ViewDefinition::FillModels::No);
    viewController->LookAtVolume(db.Units().GetProjectExtents());
    ASSERT_EQ(DgnDbStatus::Success, viewController->Save());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertCategory(DgnDbR db, Utf8CP categoryName, DgnSubCategory::Appearance const& appearance, DgnCategory::Scope scope, DgnCategory::Rank rank)
    {
    MUST_HAVE_HOST(DgnCategoryId());

    DgnCategory category(DgnCategory::CreateParams(db, categoryName, scope, rank));

    DgnCategoryCPtr persistentCategory = category.Insert(appearance);
    EXPECT_TRUE(persistentCategory.IsValid()) << WPrintfString(L"%ls - Category insert into %ls failed", WString(categoryName,BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str()).c_str();

    return persistentCategory.IsValid()? persistentCategory->GetCategoryId(): DgnCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Shaun.Sewall           09/2016
//---------------------------------------------------------------------------------------
DgnCategoryId DgnDbTestUtils::InsertCategory(DgnDbR db, Utf8CP categoryName, ColorDefCR color, DgnCategory::Scope scope, DgnCategory::Rank rank)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(color);
    return InsertCategory(db, categoryName, appearance, scope, rank);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
DgnAuthorityId DgnDbTestUtils::InsertNamespaceAuthority(DgnDbR db, Utf8CP authorityName)
    {
    MUST_HAVE_HOST(DgnAuthorityId());
    
    DgnAuthorityPtr authority = NamespaceAuthority::CreateNamespaceAuthority(authorityName, db);
    DgnDbStatus status = authority->Insert();
    EXPECT_TRUE(DgnDbStatus::Success == status) << WPrintfString(L"%ls - Authority insert into %ls failed with %x", WString(authorityName,BentleyCharEncoding::Utf8).c_str(), db.GetFileName().c_str(), (int)status).c_str();
    return authority.IsValid()? authority->GetAuthorityId(): DgnAuthorityId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      06/16
//---------------------------------------------------------------------------------------
ModelSelectorCPtr DgnDbTestUtils::InsertNewModelSelector(DgnDbR db, Utf8CP name, DgnModelId model)
    {
    ModelSelector modSel(db, name);
    modSel.SetModelId(model);
    auto modSelPersist = db.Elements().Insert(modSel);
    if (!modSelPersist.IsValid())
        {
        EXPECT_TRUE(false) << " Failed to insert model selector with name =" << name;
        return nullptr;
        }

    auto models = modSelPersist->GetModelIds();
    EXPECT_EQ(1, models.size());
    EXPECT_EQ(model, *models.begin());

    return modSelPersist;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      06/16
//---------------------------------------------------------------------------------------
CategorySelectorCPtr DgnDbTestUtils::InsertNewCategorySelector(DgnDbR db, Utf8CP name, DgnCategoryIdSet const* categoriesIn)
    {
    CategorySelector catSel(db, name);

    DgnCategoryIdSet const* categories = categoriesIn;
    DgnCategoryIdSet _categories;
    if (nullptr == categories)
        {
        for (auto const& categoryId : DgnCategory::QueryCategories(db))
            _categories.insert(categoryId);
        categories = &_categories;
        }

    if (!categories->empty())
        catSel.SetCategoryIds(*categories);

    CategorySelectorCPtr catSelPersist = db.Elements().Insert(catSel);
    if (!catSelPersist.IsValid())
        {
        EXPECT_TRUE(false) << " Insertion of CategorySelector with name " << name << " failed";
        return nullptr;
        }

    EXPECT_EQ(catSelPersist.get(), db.Elements().GetElement(catSel.GetElementId()).get());
    
    auto categoriesStored = catSelPersist->GetCategoryIds();
    EXPECT_EQ(categoriesStored, *categories);

    return catSelPersist;
    }
