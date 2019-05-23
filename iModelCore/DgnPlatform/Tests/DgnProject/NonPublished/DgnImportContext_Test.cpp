/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>
#include <DgnPlatform/NullContext.h>
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnFontData.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_RENDER

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct ImportTest : DgnDbTestFixture
{
    ImportTest() {}

    void InsertElement(DgnDbR, DgnModelId, bool is3d, bool expectSuccess);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static RenderMaterialId createTexturedMaterial(DgnDbR dgnDb, Utf8CP materialName, WCharCP pngFileName, RenderingAsset::TextureMap::Units unitMode)
    {
    RgbFactor red = {1.0, 0.0, 0.0};
    uint32_t width, height;
   
    RenderingAsset renderMaterialAsset;
    renderMaterialAsset.SetColor(RENDER_MATERIAL_Color, red);
    renderMaterialAsset.SetBool(RENDER_MATERIAL_FlagHasBaseColor, true);

    Image image;
    ImageSource imageSource;
    BeFile imageFile;
    if (BeFileStatus::Success == imageFile.Open(pngFileName, BeFileAccess::Read))
        {
        ByteStream pngBytes;
        imageFile.ReadEntireFile(pngBytes);
        imageSource = ImageSource(ImageSource::Format::Png, std::move(pngBytes));
        image = Image(imageSource);
        }
    else
        {
        width = height = 512;
        ByteStream data(width * height * 3);

        size_t      value = 0;
        Byte* imageByte=data.GetDataP();
        for (uint32_t i=0; i<data.GetSize(); ++i)
            *imageByte++ = ++value % 0xff;        

        image = Image(width, height, std::move(data), Image::Format::Rgb);
        imageSource = ImageSource(image, ImageSource::Format::Png);
        }

    EXPECT_TRUE(imageSource.IsValid());
    EXPECT_TRUE(image.IsValid());

    DefinitionModelR dictionary = dgnDb.GetDictionaryModel();
    DgnTexture texture(DgnTexture::CreateParams(dictionary, materialName/*###TODO unnamed textures*/, imageSource, image.GetWidth(), image.GetHeight()));
    texture.Insert();
    DgnTextureId textureId = texture.GetTextureId();
    EXPECT_TRUE(textureId.IsValid());

    Json::Value     patternMap, mapsMap;

    patternMap[RENDER_MATERIAL_TextureId]        = textureId.ToHexStr();
    patternMap[RENDER_MATERIAL_PatternScaleMode] = (int) unitMode;
    patternMap[RENDER_MATERIAL_PatternMapping]   = (int) Render::TextureMapping::Mode::Parametric;

    mapsMap[RENDER_MATERIAL_MAP_Pattern] = patternMap;
    renderMaterialAsset.GetValueR(RENDER_MATERIAL_Map) = mapsMap;

    RenderMaterial material(dictionary, "Test Palette", materialName);
    material.SetRenderingAsset(renderMaterialAsset);
    auto createdMaterial = material.Insert();
    EXPECT_TRUE(createdMaterial.IsValid());
    return createdMaterial.IsValid() ? createdMaterial->GetMaterialId() : RenderMaterialId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static void checkGroupHasOneMemberInModel(DgnModelR model)
{
    BeSQLite::Statement stmt(model.GetDgnDb(), "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE (ECClassId=? AND ModelId=?)");
    stmt.BindId(1, model.GetDgnDb().Domains().GetClassId(TestGroupHandler::GetHandler()));
    stmt.BindId(2, model.GetModelId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    DgnElementId groupId = stmt.GetValueId<DgnElementId>(0);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    TestGroupCPtr group = model.GetDgnDb().Elements().Get<TestGroup>(groupId);
    ASSERT_TRUE(group.IsValid());

    DgnElementIdSet members = group->QueryMembers();
    ASSERT_EQ(1, members.size());
    DgnElementCPtr member = model.GetDgnDb().Elements().Get<DgnElement>(*members.begin());
    ASSERT_TRUE(member.IsValid());
    ASSERT_EQ(model.GetModelId(), member->GetModelId());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnElementCPtr getSingleElementInModel(DgnModelR model)
{
    BeSQLite::Statement stmt(model.GetDgnDb(), "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE (ModelId=?)");
    stmt.BindId(1, model.GetModelId());
    if (BE_SQLITE_ROW != stmt.Step())
        return nullptr;
    DgnElementId gid = stmt.GetValueId<DgnElementId>(0);
    if (BE_SQLITE_DONE != stmt.Step())
        return nullptr;

    return model.GetDgnDb().Elements().Get<DgnElement>(gid);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static PhysicalModelPtr copyPhysicalModelSameDb(PhysicalModelCR model, DgnElementId newModeledElementId)
    {
    return dynamic_cast<PhysicalModel*>(DgnModel::CopyModel(model, newModeledElementId).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbPtr initDb(WCharCP fileName, Db::OpenMode mode = Db::OpenMode::ReadWrite, bool needBriefCase = true)
    {
    BeFileName dbName;
    EXPECT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(dbName, fileName));
    DgnDbPtr db;
    DgnDbTestFixture::OpenDb(db, dbName, mode, needBriefCase);
    return db;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnDbPtr openCopyOfDb(WCharCP destName, DgnDb::OpenMode mode = Db::OpenMode::ReadWrite)
    {
    DgnDbPtr db2;
    db2 = initDb(destName,mode,true);
    if (!db2.IsValid())
        return nullptr;
    DgnPlatformTestDomain::GetDomain().ImportSchema(*db2);
    return db2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportGroups)
{
    SetupSeedProject();

    // ******************************
    //  Create model1

    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model1");
    ASSERT_TRUE(model1.IsValid());
    {
        // Put a group into moddel1
        DgnCategoryId categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
        TestGroupPtr group = TestGroup::Create(*m_db, model1->GetModelId(), categoryId);
        ASSERT_TRUE(group.IsValid());
        ASSERT_TRUE(group->Insert().IsValid());

        //  Add a member
        GenericPhysicalObjectPtr member = GenericPhysicalObject::Create(*model1, categoryId);
        ASSERT_TRUE(member.IsValid());
        ASSERT_TRUE(member->Insert().IsValid());
        ASSERT_EQ(DgnDbStatus::Success, group->AddMember(*member));

        checkGroupHasOneMemberInModel(*model1);
    }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
    {
        PhysicalPartitionCPtr partition2 = PhysicalPartition::CreateAndInsert(*m_db->Elements().GetRootSubject(), "Model2");
        ASSERT_TRUE(partition2.IsValid());
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, partition2->GetElementId());
        ASSERT_TRUE(model2.IsValid());

        checkGroupHasOneMemberInModel(*model2);
    }

    //  ******************************
    //  Test "importing" a model into the same DgnDb.
    if (true)
    {
        DgnImportContext import3(*m_db, *m_db);
        PhysicalPartitionCPtr partition3 = PhysicalPartition::CreateAndInsert(*m_db->Elements().GetRootSubject(), "Model3");
        ASSERT_TRUE(partition3.IsValid());
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3, *partition3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);
    }

    //  *******************************
    //  Import into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb( L"3dMetricGeneralcc.bim");
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*m_db, *db2);
        PhysicalPartitionCPtr partition3 = PhysicalPartition::CreateAndInsert(*db2->Elements().GetRootSubject(), "Partition3");
        ASSERT_TRUE(partition3.IsValid());
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3, *partition3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);

        checkGroupHasOneMemberInModel(*model3);
        db2->SaveChanges();
    }

}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnElementCPtr insertElement(DgnDbR db, DgnModelId mid, bool is3d, DgnSubCategoryId subcat, Render::GeometryParams* customParms)
    {
    DgnCategoryId cat = DgnSubCategory::QueryCategoryId(db, subcat);

    DgnElementPtr gelem;
    if (is3d)
        gelem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, mid, DgnClassId(db.Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject)), cat, Placement3d()));
    else
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_AnnotationElement2d)), cat, Placement2d()));

    GeometryBuilderPtr builder = GeometryBuilder::Create(*gelem->ToGeometrySource());
    builder->Append(subcat);
    if (nullptr != customParms)
        builder->Append(*customParms);
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (SUCCESS != builder->Finish(*gelem->ToGeometrySourceP()))
        return nullptr;

    return db.Elements().Insert(*gelem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static Render::GeometryParams getFirstGeometryParams(DgnElementCR gel)
    {
    GeometryCollection gcollection(*gel.ToGeometrySource());
    return gcollection.begin().GetGeometryParams(); // has the side-effect of setting up the current element display params on the collection
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnSubCategory::Appearance createAppearance(ColorDef const& colorDef)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(colorDef);
    return appearance;
    }

static bool areMaterialsEqual(RenderMaterialId lmatid, DgnDbR ldb, RenderMaterialId rmatid, DgnDbR rdb)
    {
    if (!lmatid.IsValid() && !rmatid.IsValid())
        return true;
    if (!lmatid.IsValid() || !rmatid.IsValid())
        return false;
    RenderMaterialCPtr lmat = RenderMaterial::Get(ldb, lmatid);
    RenderMaterialCPtr rmat = RenderMaterial::Get(rdb, rmatid);
    if (!lmat.IsValid() || !rmat.IsValid())
        return false;
    // Note that textureids will be different. So, we must compare only values that are not IDs.
    // *** NEEDS WORK: Need a way to compare RenderMaterial and its various assets, such as textures
    return lmat->GetMaterialName() == rmat->GetMaterialName()
        && lmat->GetPaletteName() == rmat->GetPaletteName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static bool areDisplayParamsEqual(Render::GeometryParamsCR lhsUnresolved, DgnDbR ldb, Render::GeometryParamsCR rhsUnresolved, DgnDbR rdb)
    {
    // stub out data that we cannot compare by value
    Render::GeometryParams lhs(lhsUnresolved);
    Render::GeometryParams rhs(rhsUnresolved);

    //  We must "resolve" each GeometryParams object before we can ask for its properties.
    Dgn::NullContext lcontext;
    lcontext.SetDgnDb(ldb);
    lhs.Resolve(lcontext);
    NullContext rcontext;
    rcontext.SetDgnDb(rdb);
    rhs.Resolve(rcontext);

    //  Use custom logic to compare the complex properties 
    if (!areMaterialsEqual(lhs.GetMaterialId(), ldb, rhs.GetMaterialId(), rdb))
        return false;

    // *** TBD linestyles
    // *** TBD Gradient
    // *** TBD PatternParams

    //  Compare the rest of the simple properites
#define EXPECT_PARAMS_EQ(FUNC) if (lhs. FUNC () != rhs. FUNC ()) return false

    EXPECT_PARAMS_EQ(GetLineColor);
    EXPECT_PARAMS_EQ(GetWeight);
    EXPECT_PARAMS_EQ(GetGeometryClass);
    EXPECT_PARAMS_EQ(GetNetDisplayPriority);
    EXPECT_PARAMS_EQ(GetDisplayPriority);
    EXPECT_PARAMS_EQ(GetFillColor);
    EXPECT_PARAMS_EQ(GetFillDisplay);
    EXPECT_PARAMS_EQ(GetTransparency);
    EXPECT_PARAMS_EQ(GetNetTransparency);
    EXPECT_PARAMS_EQ(GetFillTransparency);
    EXPECT_PARAMS_EQ(GetNetFillTransparency);

#undef EXPECT_PARAMS_EQ

    return true;
    }

//---------------------------------------------------------------------------------------
// Check that imported element is equivalent to source element
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static void checkImportedElement(DgnElementCPtr destElem, DgnElementCR sourceElem)
    {
    GeometrySourceCP gdestElem = destElem->ToGeometrySource();
    ASSERT_TRUE(nullptr != gdestElem);
    DgnDbR destDb = destElem->GetDgnDb();

    DgnDbR sourceDb = sourceElem.GetDgnDb();

    DgnCategoryCPtr destCat = DgnCategory::Get(destDb, gdestElem->GetCategoryId());
    ASSERT_TRUE(destCat.IsValid() );
    
    ASSERT_NE(destCat->GetCategoryId(), sourceElem.ToGeometrySource()->GetCategoryId() ) << "source element's Category should have been deep-copied and remapped to a new Category in destination DB";

    DgnCategoryCPtr sourceCat = DgnCategory::Get(sourceDb, sourceElem.ToGeometrySource()->GetCategoryId());

    ASSERT_EQ(sourceCat->GetCode(), destCat->GetCode());

    Render::GeometryParams sourceDisplayParams = getFirstGeometryParams(sourceElem);

    Render::GeometryParams destDisplayParams = getFirstGeometryParams(*destElem);
    
    DgnSubCategoryId destSubCategoryId = destDisplayParams.GetSubCategoryId();
    ASSERT_TRUE( destSubCategoryId.IsValid() );
    //ASSERT_TRUE( destSubCategoryId != sourceSubCategory1Id );   don't know what Id it was assigned
    
    DgnSubCategoryCPtr destSubCategory = DgnSubCategory::Get(destDb, destSubCategoryId);
    ASSERT_TRUE(destSubCategory.IsValid());

    ASSERT_TRUE(areDisplayParamsEqual(sourceDisplayParams, sourceDb, destDisplayParams, destDb));
    }

//---------------------------------------------------------------------------------------
// Check that category, subcategory, and its appearance are deep-copied and remapped
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportElementAndCategory1)
{
    static Utf8CP s_catName="MyCat";

    SetupSeedProject();
    DgnDbP sourceDb = m_db.get();

    //  Create a Category for the elements. 
    DgnSubCategory::Appearance sourceAppearanceRequested = createAppearance(ColorDef(1, 2, 3, 0));
    sourceAppearanceRequested.SetRenderMaterial(createTexturedMaterial(*sourceDb, "Texture1", L"", RenderingAsset::TextureMap::Units::Relative));

    DgnCategoryId sourceCategoryId = DgnDbTestUtils::InsertSpatialCategory(*sourceDb, s_catName, sourceAppearanceRequested);
    ASSERT_TRUE( sourceCategoryId.IsValid() );
    DgnSubCategoryId sourceSubCategory1Id = DgnCategory::GetDefaultSubCategoryId(sourceCategoryId);
    ASSERT_TRUE( sourceSubCategory1Id.IsValid() );

    //  Create a custom SubCategory for one of the elements to use
    DgnSubCategory::Appearance sourceAppearance2 = createAppearance(ColorDef(2, 2, 3, 0));
    sourceAppearance2.SetRenderMaterial(createTexturedMaterial(*sourceDb, "Texture2", L"", RenderingAsset::TextureMap::Units::Relative));
    DgnSubCategoryCPtr sourceSubCategory2;
        {
        DgnSubCategoryPtr s = new DgnSubCategory(DgnSubCategory::CreateParams(*sourceDb, sourceCategoryId, "SubCat2", sourceAppearance2));
        sourceSubCategory2 = s->Insert();
        ASSERT_TRUE(sourceSubCategory2.IsValid());
        }
    DgnSubCategoryId sourceSubCategory2Id = sourceSubCategory2->GetSubCategoryId();

    //  Create the source model
    PhysicalModelPtr sourcemod = DgnDbTestUtils::InsertPhysicalModel(*sourceDb, "sourcemod");
    ASSERT_TRUE( sourcemod.IsValid() );

    // Put elements in this category into the source model
    DgnElementCPtr sourceElem = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory1Id, nullptr);   // 1 is based on default subcat
    DgnElementCPtr sourceElem2 = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory2Id, nullptr);  // 2 is based on custom subcat
    Render::GeometryParams customParams;
    customParams.SetCategoryId(sourceCategoryId);
    customParams.SetMaterialId(createTexturedMaterial(*sourceDb, "Texture3", L"", RenderingAsset::TextureMap::Units::Relative));
    DgnElementCPtr sourceElem3 = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory1Id, &customParams); // 3 is based on default subcat with custom display params
    sourceDb->SaveChanges();

    Render::GeometryParams sourceDisplayParams = getFirstGeometryParams(*sourceElem);

    ASSERT_EQ( sourceCategoryId , sourceElem->ToGeometrySource3d()->GetCategoryId() ); // check that the source element really was assigned to the Category that I specified above
    ASSERT_EQ( sourceSubCategory1Id , sourceDisplayParams.GetSubCategoryId() ); // check that the source element's geometry really was assigned to the SubCategory that I specified above
//*** Have to "cook" first    ASSERT_TRUE( sourceDisplayParams.GetLineColor() == sourceAppearanceRequested.GetColor() ); // check that the source element's geometry has the requested appearance
//*** Have to "cook" first    ASSERT_TRUE( sourceDisplayParams.GetMaterial() == sourceAppearanceRequested.GetMaterial() ); // check that the source element's geometry has the requested appearance

    if (true)
    {
        //  Create a second Db
        DgnDbPtr destDb = openCopyOfDb(L"3dMetricGeneralcc.bim");
        ASSERT_TRUE(destDb.IsValid());

        for (int i=0; i<32; ++i)
            {
            // Insert another category into the destination DB, just to make sure that IDs don't line up
            ASSERT_TRUE( DgnDbTestUtils::InsertSpatialCategory(*destDb, Utf8PrintfString("Unrelated%d",i).c_str(), ColorDef(7,8,9,10)).IsValid() );
            }

        PhysicalModelPtr destmod = DgnDbTestUtils::InsertPhysicalModel(*destDb, "destmod");
        ASSERT_TRUE( destmod.IsValid() );

        DgnImportContext importContext(*sourceDb, *destDb);

        // Import the elements one by one
        DgnDbStatus istatus;
        if (true)
            {
            //  This is the one that is based on the default subcategory
            DgnElementCPtr destElem = sourceElem->Import(&istatus, *destmod, importContext);
            ASSERT_TRUE( destElem.IsValid() );
            ASSERT_EQ( DgnDbStatus::Success , istatus );
            checkImportedElement(destElem, *sourceElem);
            }

        if (true)
            {
            //  This is the one that is based on the custom subcategory
            DgnElementCPtr destElem = sourceElem2->Import(&istatus, *destmod, importContext);
            ASSERT_TRUE( destElem.IsValid() );
            ASSERT_EQ( DgnDbStatus::Success , istatus );
            checkImportedElement(destElem, *sourceElem2);
            }

        if (true)
            {
            //  This is the one that is based on custom elemdisplayparams (and default subcategory)
            DgnElementCPtr destElem = sourceElem3->Import(&istatus, *destmod, importContext);
            ASSERT_TRUE( destElem.IsValid() );
            ASSERT_EQ( DgnDbStatus::Success , istatus );
            checkImportedElement(destElem, *sourceElem3);
            }

        destDb->SaveChanges();
    }
}


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportElementsWithCodeSpecs)
{
    SetupSeedProject();

    // ******************************
    //  Create some CodeSpecs. 
    CodeSpecId sourceCodeSpecId;
    RefCountedPtr<CodeSpec> codeSpec1;
    {
        auto codeSpec0 = CodeSpec::Create(*m_db, "TestCodeSpec_NotUsed");
        codeSpec1 = CodeSpec::Create(*m_db, "TestCodeSpec");
        auto codeSpec2 = CodeSpec::Create(*m_db, "TestCodeSpec_AlsoNotUsed");
        ASSERT_EQ(DgnDbStatus::Success, codeSpec0->Insert());
        ASSERT_EQ(DgnDbStatus::Success, codeSpec1->Insert());
        ASSERT_EQ(DgnDbStatus::Success, codeSpec2->Insert());

        // We'll use the *second one*, so that the source and destination CodeSpecIds will be different.
        sourceCodeSpecId = codeSpec1->GetCodeSpecId();
    }

    // ******************************
    //  Create model1

    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model1");

    // Put an element with an Item into moddel1
    {
        DgnCode code = codeSpec1->CreateCode("TestElement");
        DgnCategoryId gcatid = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
        TestElementPtr tempEl = TestElement::Create(*m_db, model1->GetModelId(), gcatid, code);
        ASSERT_TRUE(m_db->Elements().Insert(*tempEl).IsValid());
        m_db->SaveChanges();
    }

    if (true)
    {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_TRUE(el.IsValid());
        CodeSpecId said = el->GetCode().GetCodeSpecId();
        ASSERT_TRUE(said == sourceCodeSpecId);
        auto sourceCodeSpec = m_db->CodeSpecs().GetCodeSpec(sourceCodeSpecId);
        ASSERT_STREQ(sourceCodeSpec->GetName().c_str(), "TestCodeSpec");
    }

    //  *******************************
    //  Import model1 into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"3dMetricGeneralcc.bim");
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*m_db, *db2);
        PhysicalPartitionCPtr partition3 = PhysicalPartition::CreateAndInsert(*db2->Elements().GetRootSubject(), "Partition3");
        ASSERT_TRUE(partition3.IsValid());
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3, *partition3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);

        DgnElementCPtr el = getSingleElementInModel(*model3);
        ASSERT_TRUE(el.IsValid());

        // Verify that CodeSpec was copied over
        CodeSpecId daid = el->GetCode().GetCodeSpecId();
        ASSERT_TRUE(daid.IsValid());
        ASSERT_NE(daid, sourceCodeSpecId) << "CodeSpec ID should have been remapped";
        auto destCodeSpec = db2->CodeSpecs().GetCodeSpec(daid);
        ASSERT_STREQ(destCodeSpec->GetName().c_str(), "TestCodeSpec");
        db2->SaveChanges();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, OpenStatementsProblem)
{
    // This just opens the DgnDb
    SetupSeedProject(L"ImportElementsWithDependencies.bim", Db::OpenMode::ReadWrite, true);

    // Create and insert a model
    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model1");
    
    // Create and insert an element
    DgnCategoryId gcatid = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);
    //auto e1 = insertElement(*m_db, model1->GetModelId(), true, DgnCategory::GetDefaultSubCategoryId(gcatid), nullptr);
    //auto e1 = GenericPhysicalObject::Create(*model1, gcatid);
    auto e1 = TestElement::Create(*m_db, model1->GetModelId(), gcatid, "e1");       // only TestElement causes the problem!
    ASSERT_TRUE(e1->Insert().IsValid());

    //  Create and insert a second model
    auto partition2 = PhysicalPartition::CreateAndInsert(*m_db->Elements().GetRootSubject(), "Partition2");
    ASSERT_TRUE(partition2.IsValid());

    //  Copy the contents of the first model to the second
    PhysicalModelPtr model2 = dynamic_cast<PhysicalModel*>(DgnModel::CopyModel(*model1, partition2->GetElementId()).get());
    ASSERT_TRUE(model2.IsValid());
}

#if defined JS_TXN_MGR_CHANGE
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle      02/18
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, InsertOtherElementsInElementDrivesElementOnRootChanged)
    {
    struct TestElementDependency final : TestElementDrivesElementHandler::Callback
        {
        bvector<DgnElementId> m_createdElements;

        TestElementDependency() { TestElementDrivesElementHandler::SetCallback(this); }
        ~TestElementDependency() { TestElementDrivesElementHandler::SetCallback(nullptr); }

        void _OnRootChanged(DgnDbR dgndb, ECInstanceId relId, DgnElementId rootId, DgnElementId depId) override
            {
            auto root = dgndb.Elements().Get<TestElement>(rootId);
            auto dep = dgndb.Elements().Get<TestElement>(depId);
            ASSERT_TRUE(root.IsValid() && dep.IsValid());

            TestElementPtr e = TestElement::Create(dgndb, dep->GetModelId(), dep->GetCategoryId(), "");
            e->SetUserLabel("Inserted In _OnRootChanged");
            ASSERT_TRUE(dgndb.Elements().Insert(*e).IsValid());
            m_createdElements.push_back(e->GetElementId());
            }

        void _ProcessDeletedDependency(DgnDbR, dgn_TxnTable::ElementDep::DepRelData const&) override {}
        };

    SetupSeedProject(L"InsertOtherElementsInElementDrivesElementOnRootChanged.bim", Db::OpenMode::ReadWrite, true);

    TestElementDrivesElementHandler::GetHandler().Clear();
    TestElementDependency dep;

    PhysicalModelPtr model1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model1");
    DgnCategoryId gcatid = DgnDbTestUtils::GetFirstSpatialCategoryId(*m_db);

    // Create 2 elements and make the first depend on the second
    TestElementPtr e1 = TestElement::Create(*m_db, model1->GetModelId(), gcatid, "e1");
    ASSERT_TRUE(m_db->Elements().Insert(*e1).IsValid());
    TestElementPtr e2 = TestElement::Create(*m_db, model1->GetModelId(), gcatid, "e2");
    ASSERT_TRUE(m_db->Elements().Insert(*e2).IsValid());

    ASSERT_TRUE(dep.m_createdElements.empty());
    ECInstanceKey edeKey = TestElementDrivesElementHandler::Insert(*m_db, e2->GetElementId(), e1->GetElementId());
    ASSERT_TRUE(edeKey.IsValid());
    ASSERT_EQ(0, dep.m_createdElements.size()) << "_OnRootChanged not expected to be called yet";
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges());
    ASSERT_EQ(1, dep.m_createdElements.size()) << "Commit triggers _OnRootChanged which is expected to successfully create another element";
    DgnElementId elementCreatedByDependency = dep.m_createdElements[0];

    //in reality (BuildingConceptStation where this issue was reported) more changes could happen which are discarded though
    //if the user didn't save them. This tests mimicks that by just calling AbandonChanges which would clear the cached id sequence
    ASSERT_EQ(BE_SQLITE_OK, m_db->AbandonChanges());

    //now insert another element to make sure it doesn't cause a PK violation, i.e. to make sure the sequence has been properly
    //incremented in _OnCommit.
    TestElementPtr e3 = TestElement::Create(*m_db, model1->GetModelId(), gcatid, "e3");
    ASSERT_TRUE(m_db->Elements().Insert(*e3).IsValid()) << "Sequence is expected to be in sync with  anything inserted during _OnRootChanged";
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges());

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, "SELECT UserLabel FROM bis.Element WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementCreatedByDependency));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("Inserted In _OnRootChanged", stmt.GetValueText(0));
    }

    {
    //verify element id sequence in Db
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*m_db, "SELECT CAST(Val AS INTEGER) FROM be_Local WHERE Name='bis_elementidsequence'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(e3->GetElementId().GetValue(), stmt.GetValueUInt64(0));
    stmt.Finalize();

    //verify element id sequence in cache
    size_t blvCacheIndex = 0;
    ASSERT_TRUE(m_db->GetBLVCache().TryGetIndex(blvCacheIndex, "bis_elementidsequence"));
    uint64_t sequenceVal = 0;
    ASSERT_EQ(BE_SQLITE_OK, m_db->GetBLVCache().QueryValue(sequenceVal, blvCacheIndex));
    ASSERT_EQ(e3->GetElementId().GetValue(), sequenceVal);
    }

    TestElementDrivesElementHandler::GetHandler().Clear();
    }
#endif

#if defined (BENTLEY_WIN32) // Relies on getting fonts from the OS; this is Windows Desktop-only.

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ElementGeomIOCausesFontRemap)
    {
    //.............................................................................................
    DgnDbPtr db1;
    db1 = initDb(L"ElementGeomIOCausesFontRemap-1.bim");
    ASSERT_TRUE(db1.IsValid());

    DgnFontPtr db1_font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry("Arial");
    ASSERT_TRUE(db1_font.IsValid());
    DgnFontId db1_fontId = db1->Fonts().AcquireId(*db1_font);
    ASSERT_TRUE(db1_fontId.IsValid());
    
    BentleyStatus db1_fontEmbedStatus = DgnFontPersistence::Db::Embed(db1->Fonts().DbFaceData(), *db1_font);
    ASSERT_TRUE(SUCCESS == db1_fontEmbedStatus);

    BentleyApi::ECN::ECClassCP db1_physicalClass = db1->Schemas().GetClass(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);
    BeAssert(nullptr != db1_physicalClass);
    DgnClassId db1_physicalDgnClass = DgnClassId(db1_physicalClass->GetId());
    BeAssert(db1_physicalDgnClass.IsValid());

    TextStringPtr db1_text = TextString::Create();
    db1_text->SetText("ImportTest/ElementGeomIOCausesFontRemap-1");
    db1_text->GetStyleR().SetFont(*db1_font);
    db1_text->GetStyleR().SetHeight(1.0);

    DgnCategoryId categoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(*db1);
    GenericPhysicalObjectPtr db1_element = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*db1, DgnDbTestUtils::QueryFirstGeometricModelId(*db1), db1_physicalDgnClass, categoryId));
    GeometryBuilderPtr db1_builder = GeometryBuilder::CreateWithAutoPlacement(*db1->Models().GetModel(DgnDbTestUtils::QueryFirstGeometricModelId(*db1)), categoryId);
    db1_builder->Append(*db1_text, GeometryBuilder::CoordSystem::World);
    db1_builder->Finish(*db1_element->ToGeometrySourceP());

    DgnElementCPtr db1_insertedElement = db1_element->Insert();
    ASSERT_TRUE(db1_insertedElement.IsValid());
    DgnElementId db1_elementId = db1_insertedElement->GetElementId();
    ASSERT_TRUE(db1_insertedElement.IsValid());

    db1->SaveChanges();

    //.............................................................................................
    DgnDbPtr db2;
    db2 = initDb(L"ElementGeomIOCausesFontRemap-2.bim");
    ASSERT_TRUE(db2.IsValid());

    ASSERT_TRUE(nullptr == db2->Fonts().FindFontByTypeAndName(db1_font->GetType(), db1_font->GetName().c_str()));

    DgnModelPtr db2_destModel = db2->Models().GetModel(DgnDbTestUtils::QueryFirstGeometricModelId(*db2));
    ASSERT_TRUE(db2_destModel.IsValid());

    DgnImportContext import1to2(*db1, *db2);
    DgnDbStatus import1to2Status = DgnDbStatus::BadArg;
    DgnElementCPtr db2_insertedElement = db1_insertedElement->Import(&import1to2Status, *db2_destModel, import1to2);
    ASSERT_TRUE(DgnDbStatus::Success == import1to2Status);
    ASSERT_TRUE(db2_insertedElement.IsValid());

    ASSERT_TRUE(nullptr != db2->Fonts().FindFontByTypeAndName(db1_font->GetType(), db1_font->GetName().c_str()));

    db2->SaveChanges();

    //.............................................................................................
    DgnDbPtr db3;
    db3 = initDb(L"ElementGeomIOCausesFontRemap-3.bim");
    ASSERT_TRUE(db3.IsValid());

    ASSERT_TRUE(nullptr == db3->Fonts().FindFontByTypeAndName(db1_font->GetType(), db1_font->GetName().c_str()));

    DgnFontPtr db3_font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry("Courier New");
    ASSERT_TRUE(db3_font.IsValid());
    DgnFontId db3_fontId = db3->Fonts().AcquireId(*db3_font);
    ASSERT_TRUE(db3_fontId.IsValid());

    auto db3_fontIter1 = db3->Fonts().DbFontMap().MakeIterator();
    size_t db3_numFonts = 0;
    for (auto iter = db3_fontIter1.begin(); iter != db3_fontIter1.end(); ++iter)
        ++db3_numFonts;

    EXPECT_TRUE(db3_numFonts > 0);

    DgnModelPtr db3_destModel = db3->Models().GetModel(DgnDbTestUtils::QueryFirstGeometricModelId(*db3));
    ASSERT_TRUE(db3_destModel.IsValid());

    DgnImportContext import1to3(*db1, *db3);
    DgnDbStatus import1to3Status = DgnDbStatus::BadArg;
    DgnElementCPtr db3_insertedElement = db1_insertedElement->Import(&import1to3Status, *db3_destModel, import1to3);
    ASSERT_TRUE(DgnDbStatus::Success == import1to3Status);
    ASSERT_TRUE(db3_insertedElement.IsValid());

    ASSERT_TRUE(nullptr != db3->Fonts().FindFontByTypeAndName(db1_font->GetType(), db1_font->GetName().c_str()));

    auto db3_fontIter2 = db3->Fonts().DbFontMap().MakeIterator();
    size_t db3_numFonts2 = 0;
    for (auto iter = db3_fontIter2.begin(); iter != db3_fontIter2.end(); ++iter)
        ++db3_numFonts2;

    ASSERT_TRUE((db3_numFonts + 1) == db3_numFonts2);

    db3->SaveChanges();
    }

#endif
