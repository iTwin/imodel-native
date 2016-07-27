/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnImportContext_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
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
    ImportTest() { }

    void InsertElement(DgnDbR, DgnModelId, bool is3d, bool expectSuccess);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnMaterialId createTexturedMaterial(DgnDbR dgnDb, Utf8CP materialName, WCharCP pngFileName, JsonRenderMaterial::TextureMap::Units unitMode)
    {
    RgbFactor red = { 1.0, 0.0, 0.0};
    uint32_t width, height;
   
    JsonRenderMaterial renderMaterialAsset;
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

    DgnTexture texture(DgnTexture::CreateParams(dgnDb, materialName/*###TODO unnamed textures*/, imageSource, image.GetWidth(), image.GetHeight()));
    texture.Insert();
    DgnTextureId textureId = texture.GetTextureId();
    EXPECT_TRUE(textureId.IsValid());

    Json::Value     patternMap, mapsMap;

    patternMap[RENDER_MATERIAL_TextureId]        = textureId.GetValue();
    patternMap[RENDER_MATERIAL_PatternScaleMode] = (int) unitMode;
    patternMap[RENDER_MATERIAL_PatternMapping]   = (int) JsonRenderMaterial::TextureMap::Mode::Parametric;

    mapsMap[RENDER_MATERIAL_MAP_Pattern] = patternMap;
    renderMaterialAsset.GetValueR()[RENDER_MATERIAL_Map] = mapsMap;

    DgnMaterial material(DgnMaterial::CreateParams(dgnDb, "Test Palette", materialName));
    material.SetRenderingAsset(renderMaterialAsset.GetValue());
    auto createdMaterial = material.Insert();
    EXPECT_TRUE(createdMaterial.IsValid());
    return createdMaterial.IsValid() ? createdMaterial->GetMaterialId() : DgnMaterialId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static void checkGroupHasOneMemberInModel(DgnModelR model)
{
    BeSQLite::Statement stmt(model.GetDgnDb(), "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE (ECClassId=? AND ModelId=?)");
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
    BeSQLite::Statement stmt(model.GetDgnDb(), "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE (ModelId=?)");
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
static SpatialModelPtr copySpatialModelSameDb(SpatialModelCR model, Utf8CP newName)
{
    return dynamic_cast<SpatialModel*>(DgnModel::CopyModel(model, DgnModel::CreateModelCode(newName)).get());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static SpatialModelPtr createSpatialModel(DgnDbR db, Utf8CP newName)
{
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    SpatialModelPtr model = new SpatialModel(SpatialModel::CreateParams(db, mclassId, DgnModel::CreateModelCode(newName)));
    if (!model.IsValid())
        return nullptr;
    if (DgnDbStatus::Success != model->Insert())
        return nullptr;
    return model;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnDbPtr openCopyOfDb(WCharCP sourceName, WCharCP destName, DgnDb::OpenMode mode, bool importDummySchemaFirst = true)
{
    DgnDbPtr db2;
    DgnDbTestFixture::OpenDb(db2, DgnDbTestFixture::CopyDb(sourceName, destName), mode, true);
    if (!db2.IsValid())
        return nullptr;
    if (importDummySchemaFirst)
        DgnPlatformTestDomain::ImportDummySchema(*db2);
    DgnPlatformTestDomain::ImportSchema(*db2);
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

    SpatialModelPtr model1 = createSpatialModel(*m_db, "Model1");
    ASSERT_TRUE(model1.IsValid());
    {
        // Put a group into moddel1
        TestGroupPtr group = TestGroup::Create(*m_db, model1->GetModelId(), DgnCategory::QueryHighestCategoryId(*m_db));
        ASSERT_TRUE(group.IsValid());
        ASSERT_TRUE(group->Insert().IsValid());

        //  Add a member
        DgnCategoryId mcatid = DgnCategory::QueryHighestCategoryId(*m_db);
        GenericPhysicalObjectPtr member = GenericPhysicalObject::Create(*model1, mcatid);
        ASSERT_TRUE(member.IsValid());
        ASSERT_TRUE(member->Insert().IsValid());
        ASSERT_EQ(DgnDbStatus::Success, group->AddMember(*member));

        checkGroupHasOneMemberInModel(*model1);
    }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
    {
        SpatialModelPtr model2 = copySpatialModelSameDb(*model1, "Model2");
        ASSERT_TRUE(model2.IsValid());

        checkGroupHasOneMemberInModel(*model2);
    }

    //  ******************************
    //  You can't "Import" a model into the same DgnDb. For one thing, the name will conflict.
    if (true)
    {
        DgnImportContext import3(*m_db, *m_db);
        DgnDbStatus stat;
        SpatialModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE(!model3.IsValid());
        ASSERT_NE(DgnDbStatus::Success, stat);
    }

    //  *******************************
    //  Import into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.ibim", L"3dMetricGeneralcc.ibim", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*m_db, *db2);
        DgnDbStatus stat;
        SpatialModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
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
    DgnCategoryId cat = DgnSubCategory::QueryCategoryId(subcat, db);

    DgnElementPtr gelem;
    if (is3d)
        gelem = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASSNAME_PhysicalObject)), cat, Placement3d()));
    else
        gelem = AnnotationElement2d::Create(AnnotationElement2d::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationElement2d)), cat, Placement2d()));

    GeometryBuilderPtr builder = GeometryBuilder::CreateWorld(*gelem->ToGeometrySource());
    builder->Append(subcat);
    if (nullptr != customParms)
        builder->Append(*customParms);
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (SUCCESS != builder->SetGeometryStreamAndPlacement(*gelem->ToGeometrySourceP()))
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
static DgnSubCategory::Appearance createAppearance(ColorDef const& cdef)
    {
    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef(1, 2, 3, 0));
    return appearance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnCategoryId createCategory(DgnDbR db, Utf8CP name, DgnCategory::Scope scope, DgnSubCategory::Appearance const& defaultAppearance)
    {
    DgnCategoryPtr c = new DgnCategory(DgnCategory::CreateParams(db, name, scope));
    DgnCategoryCPtr cat = c->Insert(defaultAppearance);
    if (!cat.IsValid())
        return DgnCategoryId();
    return cat->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static bool areMaterialsEqual(DgnMaterialId lmatid, DgnDbR ldb, DgnMaterialId rmatid, DgnDbR rdb)
    {
    if (!lmatid.IsValid() && !rmatid.IsValid())
        return true;
    if (!lmatid.IsValid() || !rmatid.IsValid())
        return false;
    DgnMaterialCPtr lmat = DgnMaterial::QueryMaterial(lmatid, ldb);
    DgnMaterialCPtr rmat = DgnMaterial::QueryMaterial(rmatid, rdb);
    if (!lmat.IsValid() || !rmat.IsValid())
        return false;
    // Note that textureids will be different. So, we must compare only values that are not IDs.
    // *** NEEDS WORK: Need a way to compare RenderMaterial and its various assets, such as textures
    return lmat->GetMaterialName() == rmat->GetMaterialName()
        && lmat->GetPaletteName() == rmat->GetPaletteName()
        && lmat->GetDescr() == rmat->GetDescr();
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

    DgnCategoryCPtr destCat = DgnCategory::QueryCategory(gdestElem->GetCategoryId(), destDb);
    ASSERT_TRUE(destCat.IsValid() );
    
    ASSERT_NE(destCat->GetCategoryId(), sourceElem.ToGeometrySource()->GetCategoryId() ) << "source element's Category should have been deep-copied and remapped to a new Category in destination DB";

    DgnCategoryCPtr sourceCat = DgnCategory::QueryCategory(sourceElem.ToGeometrySource()->GetCategoryId(), sourceDb);

    ASSERT_EQ( sourceCat->GetCode(), destCat->GetCode() );

    Render::GeometryParams sourceDisplayParams = getFirstGeometryParams(sourceElem);

    Render::GeometryParams destDisplayParams = getFirstGeometryParams(*destElem);
    
    DgnSubCategoryId destSubCategoryId = destDisplayParams.GetSubCategoryId();
    ASSERT_TRUE( destSubCategoryId.IsValid() );
    //ASSERT_TRUE( destSubCategoryId != sourceSubCategory1Id );   don't know what Id it was assigned
    
    DgnSubCategoryCPtr destSubCategory = DgnSubCategory::QuerySubCategory(destSubCategoryId, destDb);
    ASSERT_TRUE(destSubCategory.IsValid());

    ASSERT_TRUE( areDisplayParamsEqual(sourceDisplayParams, sourceDb, destDisplayParams, destDb) );
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
    sourceAppearanceRequested.SetMaterial(createTexturedMaterial(*sourceDb, "Texture1", L"", JsonRenderMaterial::TextureMap::Units::Relative));

    DgnCategoryId sourceCategoryId = createCategory(*sourceDb, s_catName, DgnCategory::Scope::Analytical, sourceAppearanceRequested);
    ASSERT_TRUE( sourceCategoryId.IsValid() );
    DgnSubCategoryId sourceSubCategory1Id = DgnCategory::GetDefaultSubCategoryId(sourceCategoryId);
    ASSERT_TRUE( sourceSubCategory1Id.IsValid() );

    //  Create a custom SubCategory for one of the elements to use
    DgnSubCategory::Appearance sourceAppearance2 = createAppearance(ColorDef(2, 2, 3, 0));
    sourceAppearance2.SetMaterial(createTexturedMaterial(*sourceDb, "Texture2", L"", JsonRenderMaterial::TextureMap::Units::Relative));
    DgnSubCategoryCPtr sourceSubCategory2;
        {
        DgnSubCategoryPtr s = new DgnSubCategory(DgnSubCategory::CreateParams(*sourceDb, sourceCategoryId, "SubCat2", sourceAppearance2));
        sourceSubCategory2 = s->Insert();
        ASSERT_TRUE(sourceSubCategory2.IsValid());
        }
    DgnSubCategoryId sourceSubCategory2Id = sourceSubCategory2->GetSubCategoryId();

    //  Create the source model
    SpatialModelPtr sourcemod = createSpatialModel(*sourceDb, "sourcemod");
    ASSERT_TRUE( sourcemod.IsValid() );

    // Put elements in this category into the source model
    DgnElementCPtr sourceElem = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory1Id, nullptr);   // 1 is based on default subcat
    DgnElementCPtr sourceElem2 = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory2Id, nullptr);  // 2 is based on custom subcat
    Render::GeometryParams customParams;
    customParams.SetCategoryId(sourceCategoryId);
    customParams.SetMaterialId(createTexturedMaterial(*sourceDb, "Texture3", L"", JsonRenderMaterial::TextureMap::Units::Relative));
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
        DgnDbPtr destDb = openCopyOfDb(L"DgnDb/3dMetricGeneral.ibim", L"ImportTest/3dMetricGeneralcc.ibim", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(destDb.IsValid());

        for (int i=0; i<32; ++i)
            {
            // Insert another category into the destination DB, just to make sure that IDs don't line up
            ASSERT_TRUE( createCategory(*destDb, Utf8PrintfString("Unrelated%d",i).c_str(), DgnCategory::Scope::Any, createAppearance(ColorDef(7,8,9,10))).IsValid() );
            }

        SpatialModelPtr destmod = createSpatialModel(*destDb, "destmod");
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
TEST_F(ImportTest, ImportElementsWithAuthorities)
{
    SetupSeedProject();

    // ******************************
    //  Create some Authorities. 
    DgnAuthorityId sourceAuthorityId;
    RefCountedPtr<NamespaceAuthority> auth1;
    {
        auto auth0 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority_NotUsed", *m_db);
        auth1 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority", *m_db);
        auto auth2 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority_AlsoNotUsed", *m_db);
        ASSERT_EQ(DgnDbStatus::Success, auth0->Insert());
        ASSERT_EQ(DgnDbStatus::Success, auth1->Insert());
        ASSERT_EQ(DgnDbStatus::Success, auth2->Insert());

        // We'll use the *second one*, so that the source and destination authority IDs will be different.
        sourceAuthorityId = auth1->GetAuthorityId();
    }

    // ******************************
    //  Create model1

    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    SpatialModelPtr model1 = new SpatialModel(SpatialModel::CreateParams(*m_db, mclassId, DgnModel::CreateModelCode("Model1")));
    ASSERT_EQ(DgnDbStatus::Success, model1->Insert());

    // Put an element with an Item into moddel1
    {
        DgnCode code = auth1->CreateCode("TestElement");
        DgnCategoryId gcatid = DgnCategory::QueryHighestCategoryId(*m_db);
        TestElementPtr tempEl = TestElement::Create(*m_db, model1->GetModelId(), gcatid, code);
        ASSERT_TRUE(m_db->Elements().Insert(*tempEl).IsValid());
        m_db->SaveChanges();
    }

    if (true)
    {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_TRUE(el.IsValid());
        DgnAuthorityId said = el->GetCode().GetAuthority();
        ASSERT_TRUE(said == sourceAuthorityId);
        auto sourceAuthority = m_db->Authorities().GetAuthority(sourceAuthorityId);
        ASSERT_STREQ(sourceAuthority->GetName().c_str(), "TestAuthority");
    }

    //  *******************************
    //  Import model1 into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.ibim", L"ImportTest/3dMetricGeneralcc.ibim", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*m_db, *db2);
        DgnDbStatus stat;
        SpatialModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);

        DgnElementCPtr el = getSingleElementInModel(*model3);
        ASSERT_TRUE(el.IsValid());

        // Verify that Authority was copied over
        DgnAuthorityId daid = el->GetCode().GetAuthority();
        ASSERT_TRUE(daid.IsValid());
        ASSERT_NE(daid, sourceAuthorityId) << "Authority ID should have been remapped";
        auto destAuthority = db2->Authorities().GetAuthority(daid);
        ASSERT_STREQ(destAuthority->GetName().c_str(), "TestAuthority");
        db2->SaveChanges();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportElementsWithDependencies)
{
    SetupSeedProject(L"ImportElementsWithDependencies.bim", Db::OpenMode::ReadWrite, true);
    TestElementDrivesElementHandler::GetHandler().Clear();

    // ******************************
    //  Create model1

    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel));
    SpatialModelPtr model1 = new SpatialModel(SpatialModel::CreateParams(*m_db, mclassId, DgnModel::CreateModelCode("Model1")));
    ASSERT_EQ(DgnDbStatus::Success, model1->Insert());

    // Create 2 elements and make the first depend on the second
    {
        DgnCategoryId gcatid = DgnCategory::QueryHighestCategoryId(*m_db);

        TestElementPtr e1 = TestElement::Create(*m_db, model1->GetModelId(), gcatid, "e1");
        ASSERT_TRUE(m_db->Elements().Insert(*e1).IsValid());

        TestElementPtr e2 = TestElement::Create(*m_db, model1->GetModelId(), gcatid, "e2");
        ASSERT_TRUE(m_db->Elements().Insert(*e2).IsValid());

        TestElementDrivesElementHandler::Insert(*m_db, e2->GetElementId(), e1->GetElementId());

        m_db->SaveChanges();

        ASSERT_EQ(TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1);
        TestElementDrivesElementHandler::GetHandler().Clear();
    }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
    {
        SpatialModelPtr model2 = copySpatialModelSameDb(*model1, "Model2");
        ASSERT_TRUE(model2.IsValid());

        m_db->SaveChanges();
        ASSERT_EQ(TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1);
        TestElementDrivesElementHandler::GetHandler().Clear();
    }

    //  *******************************
    //  Import into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.ibim", L"ImportTest/3dMetricGeneralcc.ibim", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*m_db, *db2);
        DgnDbStatus stat;
        SpatialModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);

        db2->SaveChanges();
        ASSERT_EQ(TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1);
        TestElementDrivesElementHandler::GetHandler().Clear();
    }
}

#if defined (BENTLEY_WIN32) // Relies on getting fonts from the OS; this is Windows Desktop-only.

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ElementGeomIOCausesFontRemap)
    {
    //.............................................................................................
    DgnDbPtr db1;
    DgnDbTestFixture::OpenDb(db1, DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.ibim", L"ImportTest/ElementGeomIOCausesFontRemap-1.bim"), DgnDb::OpenMode::ReadWrite, true);
    ASSERT_TRUE(db1.IsValid());

    DgnFontPtr db1_font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry("Arial");
    ASSERT_TRUE(db1_font.IsValid());
    DgnFontId db1_fontId = db1->Fonts().AcquireId(*db1_font);
    ASSERT_TRUE(db1_fontId.IsValid());
    
    BentleyStatus db1_fontEmbedStatus = DgnFontPersistence::Db::Embed(db1->Fonts().DbFaceData(), *db1_font);
    ASSERT_TRUE(SUCCESS == db1_fontEmbedStatus);

    BentleyApi::ECN::ECClassCP db1_physicalClass = db1->Schemas().GetECClass(GENERIC_DOMAIN_NAME, GENERIC_CLASSNAME_PhysicalObject);
    BeAssert(nullptr != db1_physicalClass);
    DgnClassId db1_physicalDgnClass = DgnClassId(db1_physicalClass->GetId());
    BeAssert(db1_physicalDgnClass.IsValid());

    TextStringPtr db1_text = TextString::Create();
    db1_text->SetText("ImportTest/ElementGeomIOCausesFontRemap-1");
    db1_text->GetStyleR().SetFont(*db1_font);
    db1_text->GetStyleR().SetHeight(1.0);

    GenericPhysicalObjectPtr db1_element = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*db1, db1->Models().QueryFirstModelId(), db1_physicalDgnClass, DgnCategory::QueryFirstCategoryId(*db1)));
    GeometryBuilderPtr db1_builder = GeometryBuilder::CreateWorld(*db1->Models().GetModel(db1->Models().QueryFirstModelId()), DgnCategory::QueryFirstCategoryId(*db1));
    db1_builder->Append(*db1_text);
    db1_builder->SetGeometryStreamAndPlacement(*db1_element->ToGeometrySourceP());

    DgnElementCPtr db1_insertedElement = db1_element->Insert();
    ASSERT_TRUE(db1_insertedElement.IsValid());
    DgnElementId db1_elementId = db1_insertedElement->GetElementId();
    ASSERT_TRUE(db1_insertedElement.IsValid());

    db1->SaveChanges();

    //.............................................................................................
    DgnDbPtr db2;
    DgnDbTestFixture::OpenDb(db2, DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.ibim", L"ImportTest/ElementGeomIOCausesFontRemap-2.bim"), DgnDb::OpenMode::ReadWrite, true);
    ASSERT_TRUE(db2.IsValid());

    ASSERT_TRUE(nullptr == db2->Fonts().FindFontByTypeAndName(db1_font->GetType(), db1_font->GetName().c_str()));

    DgnModelPtr db2_destModel = db2->Models().GetModel(db2->Models().QueryFirstModelId());
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
    DgnDbTestFixture::OpenDb(db3, DgnDbTestFixture::CopyDb(L"DgnDb/3dMetricGeneral.ibim", L"ImportTest/ElementGeomIOCausesFontRemap-3.bim"), DgnDb::OpenMode::ReadWrite, true);
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

    DgnModelPtr db3_destModel = db3->Models().GetModel(db3->Models().QueryFirstModelId());
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
