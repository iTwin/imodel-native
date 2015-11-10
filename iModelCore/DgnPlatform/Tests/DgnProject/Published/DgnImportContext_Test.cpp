/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnImportContext_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass                                                    Julija.Suboc     07/2013
//----------------------------------------------------------------------------------------
struct ImportTest : public testing::Test
{
    ScopedDgnHost m_autoDgnHost;
    DgnDbPtr m_dgndb;    
    DgnModelPtr m_model;

    ImportTest()
        {
        // Must register my domain whenever I initialize a host
        DgnPlatformTestDomain::Register(); 
        }

    void InsertElement(DgnDbR, DgnModelId, bool is3d, bool expectSuccess);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnMaterialId     createTexturedMaterial (DgnDbR dgnDb, Utf8CP materialName, WCharCP pngFileName, RenderMaterialMap::Units unitMode)
    {
    Json::Value                     renderMaterialAsset;
    RgbFactor                       red = { 1.0, 0.0, 0.0};
    bvector <Byte>                  fileImageData, imageData;
    uint32_t                        width, height;
    ImageUtilities::RgbImageInfo    rgbImageInfo;
    BeFile                          imageFile;

    
    RenderMaterialUtil::SetColor (renderMaterialAsset, RENDER_MATERIAL_Color, red);
    renderMaterialAsset[RENDER_MATERIAL_FlagHasBaseColor] = true;
    

    if (BeFileStatus::Success == imageFile.Open (pngFileName, BeFileAccess::Read) &&
        SUCCESS == ImageUtilities::ReadImageFromPngFile (fileImageData, rgbImageInfo, imageFile))
        {
        width = rgbImageInfo.width;
        height = rgbImageInfo.height;

        imageData.resize (width * height * 4);

        for (size_t i=0, j=0; i<imageData.size(); )
            {
            imageData[i++] = fileImageData[j++];
            imageData[i++] = fileImageData[j++];
            imageData[i++] = fileImageData[j++];
            imageData[i++] = 255;     // Alpha.
            }
        }
    else
        {
        width = height = 512;
        imageData.resize (width * height * 4);

        size_t      value = 0;
        for (auto& imageByte : imageData)
            imageByte = ++value % 0xff;        
        }

    DgnTexture::Data textureData(DgnTexture::Format::RAW, &imageData.front(), imageData.size(), width, height);
    DgnTexture texture(DgnTexture::CreateParams(dgnDb, materialName/*###TODO unnamed textures*/, textureData));
    texture.Insert();
    DgnTextureId textureId = texture.GetTextureId();
    EXPECT_TRUE(textureId.IsValid());

    Json::Value     patternMap, mapsMap;

    patternMap[RENDER_MATERIAL_TextureId]        = textureId.GetValue();
    patternMap[RENDER_MATERIAL_PatternScaleMode] = (int) unitMode;
    patternMap[RENDER_MATERIAL_PatternMapping]   = (int) RenderMaterialMap::Mode::Parametric;

    mapsMap[RENDER_MATERIAL_MAP_Pattern] = patternMap;
    renderMaterialAsset[RENDER_MATERIAL_Map] = mapsMap;

    DgnMaterial material(DgnMaterial::CreateParams(dgnDb, "Test Palette", materialName));
    material.SetRenderingAsset (renderMaterialAsset);
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
    stmt.BindInt64(1, model.GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
    stmt.BindId(2, model.GetModelId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    DgnElementId gid = stmt.GetValueId<DgnElementId>(0);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ElementGroupCPtr group = model.GetDgnDb().Elements().Get<ElementGroup>(gid);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName copyDb(WCharCP inputFileName, WCharCP outputFileName)
{
    BeFileName fullInputFileName;
    BeTest::GetHost().GetDocumentsRoot(fullInputFileName);
    fullInputFileName.AppendToPath(inputFileName);

    BeFileName fullOutputFileName;
    BeTest::GetHost().GetOutputRoot(fullOutputFileName);
    fullOutputFileName.AppendToPath(outputFileName);

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile(fullInputFileName, fullOutputFileName))
        return BeFileName();

    return fullOutputFileName;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void openDb(DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode)
{
    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    ASSERT_TRUE(db.IsValid()) << (WCharCP)WPrintfString(L"Failed to open %ls in mode %d => result=%x", name.c_str(), (int)mode, (int)result);
    ASSERT_EQ(BE_SQLITE_OK, result);
    db->Txns().EnableTracking(true);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static PhysicalModelPtr copyPhysicalModelSameDb(PhysicalModelCR model, Utf8CP newName)
{
    return dynamic_cast<PhysicalModel*>(DgnModel::CopyModel(model, DgnModel::CreateModelCode(newName)).get());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static PhysicalModelPtr createPhysicalModel(DgnDbR db, Utf8CP newName)
{
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model = new PhysicalModel(PhysicalModel::CreateParams(db, mclassId, DgnModel::CreateModelCode(newName)));
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
    openDb(db2, copyDb(sourceName, destName), mode);
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
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    // ******************************
    //  Create model1

    PhysicalModelPtr model1 = createPhysicalModel(*db, "Model1");
    ASSERT_TRUE(model1.IsValid());
    {
        // Put a group into moddel1
        ElementGroupCPtr group;
        {
            DgnClassId gclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
            DgnElementCPtr groupEl = ElementGroup::Create(ElementGroup::CreateParams(*db, model1->GetModelId(), gclassid))->Insert();
            group = dynamic_cast<ElementGroupCP>(groupEl.get());
            ASSERT_TRUE(group.IsValid());
        }

        //  Add a member
        if (true)
        {
            DgnClassId mclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
            DgnCategoryId mcatid = DgnCategory::QueryHighestCategoryId(*db);
            auto member = PhysicalElement::Create(PhysicalElement::CreateParams(*db, model1->GetModelId(), mclassid, mcatid, Placement3d()))->Insert();
            //auto member = PhysicalElement::Create(*model1, mcatid)->Insert();
            ASSERT_TRUE(member.IsValid());
            ASSERT_EQ(DgnDbStatus::Success, group->InsertMember(*member));
        }

        checkGroupHasOneMemberInModel(*model1);
    }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
    {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE(model2.IsValid());

        checkGroupHasOneMemberInModel(*model2);
    }

    //  ******************************
    //  You can't "Import" a model into the same DgnDb. For one thing, the name will conflict.
    if (true)
    {
        DgnImportContext import3(*db, *db);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE(!model3.IsValid());
        ASSERT_NE(DgnDbStatus::Success, stat);
    }

    //  *******************************
    //  Import into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);

        checkGroupHasOneMemberInModel(*model3);
        db2->SaveChanges();
    }

}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnElementCPtr insertElement(DgnDbR db, DgnModelId mid, bool is3d, DgnSubCategoryId subcat, ElemDisplayParams* customParms)
    {
    DgnCategoryId cat = DgnSubCategory::QueryCategoryId(subcat, db);

    DgnElementPtr gelem;
    if (is3d)
        gelem = PhysicalElement::Create(PhysicalElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "PhysicalElement")), cat, Placement3d()));
    else
        gelem = DrawingElement::Create(DrawingElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "DrawingElement")), cat, Placement2d()));

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*gelem->ToGeometrySource());
    builder->Append(subcat);
    if (nullptr != customParms)
        builder->Append(*customParms);
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*gelem->ToGeometrySourceP()))
        return nullptr;

    return db.Elements().Insert(*gelem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static void getFirstElemDisplayParams(ElemDisplayParams& ret, DgnElementCR gel)
    {
    ElementGeometryCollection gcollection(*gel.ToGeometrySource());
    gcollection.begin(); // has the side-effect of setting up the current element display params on the collection
    ret = gcollection.GetElemDisplayParams();
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
    return lmat->GetValue() == rmat->GetValue();
    }

struct NullContext : ViewContext
{
void _AllocateScanCriteria () override{;}
QvElem* _DrawCached (IStrokeForCache&){return nullptr;}
void _DrawSymbol (IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight) override {}
void _DeleteSymbol (IDisplaySymbol*) override {}
bool _FilterRangeIntersection (GeometrySourceCR element) override {return false;}
void _CookDisplayParams (ElemDisplayParamsR, ElemMatSymbR) override {}
void _SetupOutputs () override {SetIViewDraw (*m_IViewDraw);}
NullContext (DgnDbR db) {m_dgnDb=&db; m_IViewDraw = nullptr; m_IDrawGeom = nullptr; m_ignoreViewRange = true; }
}; // NullContext

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static bool areDisplayParamsEqual(ElemDisplayParamsCR lhsUnresolved, DgnDbR ldb, ElemDisplayParamsCR rhsUnresolved, DgnDbR rdb)
    {
    // stub out data that we cannot compare by value
    ElemDisplayParams lhs(lhsUnresolved);
    ElemDisplayParams rhs(rhsUnresolved);

    //  We must "resolve" each ElemDisplayParams object before we can ask for its properties.
    NullContext lcontext(ldb);
    lhs.Resolve(lcontext);
    NullContext rcontext(rdb);
    rhs.Resolve(rcontext);

    //  Use custom logic to compare the complex properties 
    if (!areMaterialsEqual(lhs.GetMaterial(), ldb, rhs.GetMaterial(), rdb))
        return false;

    // *** TBD linestyles
    // *** TBD Gradient
    // *** TBD PatternParams

    //  Compare the rest of the simple properites
    rhs.SetCategoryId(lhs.GetCategoryId());
    rhs.SetSubCategoryId(lhs.GetSubCategoryId());
    lhs.SetMaterial(DgnMaterialId());
    rhs.SetMaterial(DgnMaterialId());
    lhs.SetLineStyle(nullptr);
    rhs.SetLineStyle(nullptr);
    lhs.SetGradient(nullptr);
    rhs.SetGradient(nullptr);
    lhs.SetPatternParams(nullptr);
    rhs.SetPatternParams(nullptr);

    return lhs == rhs;
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

    ElemDisplayParams sourceDisplayParams;
    getFirstElemDisplayParams(sourceDisplayParams, sourceElem);

    ElemDisplayParams destDisplayParams;
    getFirstElemDisplayParams(destDisplayParams, *destElem);
    
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

    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP sourceDb = tdm.GetDgnProjectP();

    ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*sourceDb));

    //  Create a Category for the elements. 
    DgnSubCategory::Appearance sourceAppearanceRequested = createAppearance(ColorDef(1, 2, 3, 0));
    sourceAppearanceRequested.SetMaterial(createTexturedMaterial(*sourceDb, "Texture1", L"", RenderMaterialMap::Units::Relative));
    DgnCategoryId sourceCategoryId = createCategory(*sourceDb, s_catName, DgnCategory::Scope::Analytical, sourceAppearanceRequested);
    ASSERT_TRUE( sourceCategoryId.IsValid() );
    DgnSubCategoryId sourceSubCategory1Id = DgnCategory::GetDefaultSubCategoryId(sourceCategoryId);
    ASSERT_TRUE( sourceSubCategory1Id.IsValid() );

    //  Create a custom SubCategory for one of the elements to use
    DgnSubCategory::Appearance sourceAppearance2 = createAppearance(ColorDef(2, 2, 3, 0));
    sourceAppearance2.SetMaterial(createTexturedMaterial(*sourceDb, "Texture2", L"", RenderMaterialMap::Units::Relative));
    DgnSubCategoryCPtr sourceSubCategory2;
        {
        DgnSubCategoryPtr s = new DgnSubCategory(DgnSubCategory::CreateParams(*sourceDb, sourceCategoryId, "SubCat2", sourceAppearance2));
        sourceSubCategory2 = s->Insert();
        ASSERT_TRUE(sourceSubCategory2.IsValid());
        }
    DgnSubCategoryId sourceSubCategory2Id = sourceSubCategory2->GetSubCategoryId();

    //  Create the source model
    PhysicalModelPtr sourcemod = createPhysicalModel(*sourceDb, "sourcemod");
    ASSERT_TRUE( sourcemod.IsValid() );

    // Put elements in this category into the source model
    DgnElementCPtr sourceElem = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory1Id, nullptr);   // 1 is based on default subcat
    DgnElementCPtr sourceElem2 = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory2Id, nullptr);  // 2 is based on custom subcat
    ElemDisplayParams customParams;
    customParams.SetCategoryId(sourceCategoryId);
    customParams.SetMaterial(createTexturedMaterial(*sourceDb, "Texture3", L"", RenderMaterialMap::Units::Relative));
    DgnElementCPtr sourceElem3 = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategory1Id, &customParams); // 3 is based on default subcat with custom display params
    sourceDb->SaveChanges();

    ElemDisplayParams sourceDisplayParams;
    getFirstElemDisplayParams(sourceDisplayParams, *sourceElem);

    ASSERT_EQ( sourceCategoryId , sourceElem->ToGeometrySource3d()->GetCategoryId() ); // check that the source element really was assigned to the Category that I specified above
    ASSERT_EQ( sourceSubCategory1Id , sourceDisplayParams.GetSubCategoryId() ); // check that the source element's geometry really was assigned to the SubCategory that I specified above
//*** Have to "cook" first    ASSERT_TRUE( sourceDisplayParams.GetLineColor() == sourceAppearanceRequested.GetColor() ); // check that the source element's geometry has the requested appearance
//*** Have to "cook" first    ASSERT_TRUE( sourceDisplayParams.GetMaterial() == sourceAppearanceRequested.GetMaterial() ); // check that the source element's geometry has the requested appearance

    if (true)
    {
        //  Create a second Db
        DgnDbPtr destDb = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(destDb.IsValid());

        for (int i=0; i<32; ++i)
            {
            // Insert another category into the destination DB, just to make sure that IDs don't line up
            ASSERT_TRUE( createCategory(*destDb, Utf8PrintfString("Unrelated%d",i), DgnCategory::Scope::Any, createAppearance(ColorDef(7,8,9,10))).IsValid() );
            }

        PhysicalModelPtr destmod = createPhysicalModel(*destDb, "destmod");
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
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*db));

    // ******************************
    //  Create some Authorities. 
    DgnAuthorityId sourceAuthorityId;
    RefCountedPtr<NamespaceAuthority> auth1;
    {
        auto auth0 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority_NotUsed", *db);
        auth1 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority", *db);
        auto auth2 = NamespaceAuthority::CreateNamespaceAuthority("TestAuthority_AlsoNotUsed", *db);
        ASSERT_EQ(DgnDbStatus::Success, auth0->Insert());
        ASSERT_EQ(DgnDbStatus::Success, auth1->Insert());
        ASSERT_EQ(DgnDbStatus::Success, auth2->Insert());

        // We'll use the *second one*, so that the source and destination authority IDs will be different.
        sourceAuthorityId = auth1->GetAuthorityId();
    }

    // ******************************
    //  Create model1

    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, DgnModel::CreateModelCode("Model1")));
    ASSERT_EQ(DgnDbStatus::Success, model1->Insert());

    // Put an element with an Item into moddel1
    {
        DgnElement::Code code = auth1->CreateCode("TestElement");
        DgnCategoryId gcatid = DgnCategory::QueryHighestCategoryId(*db);
        TestElementPtr tempEl = TestElement::Create(*db, model1->GetModelId(), gcatid, code);
        ASSERT_TRUE(db->Elements().Insert(*tempEl).IsValid());
        db->SaveChanges();
    }

    if (true)
    {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_TRUE(el.IsValid());
        DgnAuthorityId said = el->GetCode().GetAuthority();
        ASSERT_TRUE(said == sourceAuthorityId);
        auto sourceAuthority = db->Authorities().GetAuthority(sourceAuthorityId);
        ASSERT_STREQ(sourceAuthority->GetName().c_str(), "TestAuthority");
    }

    //  *******************************
    //  Import model1 into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
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
#ifdef WIP_ELEMENT_ITEM // *** pending redesign
TEST_F(ImportTest, ImportElementsWithItems)
{
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*db));

    // ******************************
    //  Create model1

    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, DgnModel::CreateModelCode("Model1")));
    ASSERT_EQ(DgnDbStatus::Success, model1->Insert());

    // Put an element with an Item into moddel1
    {
        DgnCategoryId gcatid = DgnCategory::QueryHighestCategoryId(*db);
        TestElementPtr tempEl = TestElement::Create(*db, model1->GetModelId(), gcatid, "TestElement");
        DgnElement::Item::SetItem(*tempEl, *TestItem::Create("Line"));
        ASSERT_TRUE(db->Elements().Insert(*tempEl).IsValid());
        db->SaveChanges();
    }

    if (true)
    {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_NE(nullptr, DgnElement::Item::GetItem(*el));
    }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
    {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE(model2.IsValid());

        DgnElementCPtr el = getSingleElementInModel(*model2);
        ASSERT_NE(nullptr, DgnElement::Item::GetItem(*el));
    }

    //  *******************************
    //  Import into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);

        DgnElementCPtr el = getSingleElementInModel(*model3);
        ASSERT_NE(nullptr, DgnElement::Item::GetItem(*el));
        db2->SaveChanges();
    }
}
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportElementsWithDependencies)
{
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();
    db->Txns().EnableTracking(true);

    ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*db));

    TestElementDrivesElementHandler::GetHandler().Clear();

    // ******************************
    //  Create model1

    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, DgnModel::CreateModelCode("Model1")));
    ASSERT_EQ(DgnDbStatus::Success, model1->Insert());

    // Create 2 elements and make the first depend on the second
    {
        DgnCategoryId gcatid = DgnCategory::QueryHighestCategoryId(*db);

        TestElementPtr e1 = TestElement::Create(*db, model1->GetModelId(), gcatid, "e1");
        ASSERT_TRUE(db->Elements().Insert(*e1).IsValid());

        TestElementPtr e2 = TestElement::Create(*db, model1->GetModelId(), gcatid, "e2");
        ASSERT_TRUE(db->Elements().Insert(*e2).IsValid());

        TestElementDrivesElementHandler::Insert(*db, e2->GetElementId(), e1->GetElementId());

        db->SaveChanges();

        ASSERT_EQ(TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1);
        TestElementDrivesElementHandler::GetHandler().Clear();
    }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
    {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE(model2.IsValid());

        db->SaveChanges();
        ASSERT_EQ(TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1);
        TestElementDrivesElementHandler::GetHandler().Clear();
    }

    //  *******************************
    //  Import into separate db
    if (true)
    {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(db2.IsValid());

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE(model3.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);

        db2->SaveChanges();
        ASSERT_EQ(TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1);
        TestElementDrivesElementHandler::GetHandler().Clear();
    }
}

