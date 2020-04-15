//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------

#include "ChangeTestFixture.h"

// #define DEBUG_REVISION_TEST_COMPRESSION 1

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

DgnPlatformSeedManager::SeedDbInfo ChangeTestFixture::s_seedFileInfo;
CodeSpecId ChangeTestFixture::m_defaultCodeSpecId;


//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::SetUpTestCase()
    {
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(true, true));

    //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
    //  Note that we must put our group seed file in a group-specific sub-directory
    ChangeTestFixture::s_seedFileInfo = rootSeedInfo;
    ChangeTestFixture::s_seedFileInfo.fileName.SetName(L"ChangeTestFixture/ChangeTestFixtureSeed.bim");

    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, ChangeTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());

    ASSERT_EQ(SchemaStatus::Success, DgnPlatformTestDomain::GetDomain().ImportSchema(*db));
    TestDataManager::SetAsStandAlone(db, Db::OpenMode::ReadWrite);

    m_defaultCodeSpecId = DgnDbTestUtils::InsertCodeSpec(*db, "TestCodeSpec");
    ASSERT_TRUE(m_defaultCodeSpecId.IsValid());

    db->SaveChanges();
    // Create a dummy revision to purge transaction table for the test
    DgnRevisionPtr rev = db->Revisions().StartCreateRevision();
    BeAssert(rev.IsValid());
    db->Revisions().FinishCreateRevision();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ChangeTestFixture::SetupDgnDb(BeFileName seedFileName, WCharCP newFileName)
{
    m_db = DgnPlatformSeedManager::OpenSeedDbCopy(seedFileName, newFileName);
    ASSERT_TRUE(m_db.IsValid());

    m_defaultModelId = DgnDbTestUtils::QueryFirstGeometricModelId(*m_db);

    m_defaultCodeSpec = m_db->CodeSpecs().GetCodeSpec(m_defaultCodeSpecId);
    ASSERT_TRUE(m_defaultCodeSpec.IsValid());

    m_defaultModel = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
    ASSERT_TRUE(m_defaultModel.IsValid());

    m_defaultCategoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::OpenDgnDb(BeFileName fileName, Db::OpenMode openMode/*=Db::OpenMode::ReadWrite*/)
{
    DbResult openStatus;
    DgnDb::OpenParams openParams(openMode);
    m_db = DgnDb::OpenDgnDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_defaultCodeSpec = m_db->CodeSpecs().GetCodeSpec(m_defaultCodeSpecId);
    ASSERT_TRUE(m_defaultCodeSpec.IsValid());

    m_defaultModel = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
    ASSERT_TRUE(m_defaultModel.IsValid());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CloseDgnDb()
{
    m_db->CloseDb();
    m_db = nullptr;
    m_defaultModel = nullptr;
    m_defaultCodeSpec = nullptr;
}
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnCategoryId ChangeTestFixture::InsertCategory(Utf8CP categoryName)
    {
    SpatialCategory category(m_db->GetDictionaryModel(), categoryName, DgnCategory::Rank::Application);

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef::White());

    auto persistentCategory = category.Insert(appearance);
    BeAssert(persistentCategory.IsValid());

    return persistentCategory->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    11/2016
//---------------------------------------------------------------------------------------
double randFraction()
    {
#ifdef DEBUG_REVISION_TEST_COMPRESSION
    return (double) rand() / RAND_MAX;
#else
    return 0.0;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
DgnElementId ChangeTestFixture::InsertPhysicalElement(DgnDbR db, PhysicalModelR model, DgnCategoryId categoryId, int x, int y, int z)
    {
    GenericPhysicalObjectPtr testElement = GenericPhysicalObject::Create(model, categoryId);

    DPoint3d sizeOfBlock = DPoint3d::From(1 + randFraction(), 1 + randFraction(), 1 + randFraction());
    DgnBoxDetail blockDetail = DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(randFraction(), randFraction(), randFraction()), sizeOfBlock, true);
    ISolidPrimitivePtr testGeomPtr = ISolidPrimitive::CreateDgnBox(blockDetail);
    BeAssert(testGeomPtr.IsValid());

    DPoint3d centerOfBlock = DPoint3d::From(x + randFraction(), y + randFraction(), z + randFraction());
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, centerOfBlock, YawPitchRollAngles());
    builder->Append(*testGeomPtr);
    BentleyStatus status = builder->Finish(*testElement);
    BeAssert(status == SUCCESS);

    DgnElementId elementId = db.Elements().Insert(*testElement)->GetElementId();
    return elementId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
void ChangeTestFixture::CreateDefaultView(DgnDbR db)
    {
    DefinitionModelR dictionary = db.GetDictionaryModel();
    auto categories = new CategorySelector(dictionary, "");
    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        categories->AddCategory(categoryEntry.GetId<DgnCategoryId>());

    auto style = new DisplayStyle3d(dictionary, "");
    auto flags = style->GetViewFlags();
    flags.SetRenderMode(Render::RenderMode::SmoothShade);
    style->SetViewFlags(flags);

    auto models = new ModelSelector(dictionary, "");
    ModelIterator modIter = db.Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_SpatialModel));
    for (ModelIteratorEntryCR entry : modIter)
        {
        auto id = entry.GetModelId();
        auto model = db.Models().GetModel(id);

        if (model.IsValid())
            models->AddModel(id);
        }

    SpatialViewDefinition view(dictionary, "Default", *categories, *style, *models);
    view.SetStandardViewRotation(StandardView::Iso);

    ASSERT_TRUE(view.Insert().IsValid());

    DgnViewId viewId = view.GetViewId();
    db.SaveProperty(DgnViewProperty::DefaultView(), &viewId, (uint32_t) sizeof(viewId));
    }
