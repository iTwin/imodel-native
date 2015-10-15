/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnImportContext_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static void checkGroupHasOneMemberInModel(DgnModelR model)
    {
    BeSQLite::Statement stmt(model.GetDgnDb(), "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE (ECClassId=? AND ModelId=?)");
    stmt.BindInt64(1, model.GetDgnDb().Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
    stmt.BindId(2, model.GetModelId());
    ASSERT_EQ( BE_SQLITE_ROW , stmt.Step() );
    DgnElementId gid = stmt.GetValueId<DgnElementId>(0);
    ASSERT_EQ( BE_SQLITE_DONE , stmt.Step() );

    ElementGroupCPtr group = model.GetDgnDb().Elements().Get<ElementGroup>(gid);
    ASSERT_TRUE( group.IsValid() );

    DgnElementIdSet members = group->QueryMembers();
    ASSERT_EQ( 1 , members.size() );
    DgnElementCPtr member = model.GetDgnDb().Elements().Get<DgnElement>(*members.begin());
    ASSERT_TRUE( member.IsValid() );
    ASSERT_EQ( model.GetModelId() , member->GetModelId() );
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
static BeFileName copyDb (WCharCP inputFileName, WCharCP outputFileName)
    {
    BeFileName fullInputFileName;
    BeTest::GetHost().GetDocumentsRoot (fullInputFileName);
    fullInputFileName.AppendToPath (inputFileName);

    BeFileName fullOutputFileName;
    BeTest::GetHost().GetOutputRoot(fullOutputFileName);
    fullOutputFileName.AppendToPath(outputFileName);

    if (BeFileNameStatus::Success != BeFileName::BeCopyFile (fullInputFileName, fullOutputFileName))
        return BeFileName();

    return fullOutputFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void openDb (DgnDbPtr& db, BeFileNameCR name, DgnDb::OpenMode mode)
    {
    DbResult result = BE_SQLITE_OK;
    db = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(mode));
    ASSERT_TRUE( db.IsValid() ) << (WCharCP)WPrintfString(L"Failed to open %ls in mode %d => result=%x", name.c_str(), (int)mode, (int)result);
    ASSERT_EQ( BE_SQLITE_OK , result );
    db->Txns().EnableTracking(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static PhysicalModelPtr copyPhysicalModelSameDb(PhysicalModelCR model, Utf8CP newName)
    {
    return dynamic_cast<PhysicalModel*>(DgnModel::CopyModel(model, newName).get());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static PhysicalModelPtr createPhysicalModel(DgnDbR db, Utf8CP newName)
    {
    DgnClassId mclassId = DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model = new PhysicalModel(PhysicalModel::CreateParams(db, mclassId, newName));
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
    ASSERT_TRUE( model1.IsValid() );
        {
        // Put a group into moddel1
        ElementGroupCPtr group;
            {
            DgnClassId gclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementGroup));
            DgnElementCPtr groupEl = ElementGroup::Create(ElementGroup::CreateParams(*db, model1->GetModelId(), gclassid))->Insert();
            group = dynamic_cast<ElementGroupCP>(groupEl.get());
            ASSERT_TRUE( group.IsValid() );
            }

        //  Add a member
        if (true)
            {
            DgnClassId mclassid = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement));
            DgnCategoryId mcatid = db->Categories().QueryHighestId();
            auto member = PhysicalElement::Create(PhysicalElement::CreateParams(*db, model1->GetModelId(), mclassid, mcatid, Placement3d()))->Insert();
            //auto member = PhysicalElement::Create(*model1, mcatid)->Insert();
            ASSERT_TRUE( member.IsValid() );
            ASSERT_EQ( DgnDbStatus::Success , group->InsertMember(*member) );
            }

        checkGroupHasOneMemberInModel(*model1);
        }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
        {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE( model2.IsValid() );

        checkGroupHasOneMemberInModel(*model2);
        }

    //  ******************************
    //  You can't "Import" a model into the same DgnDb. For one thing, the name will conflict.
    if (true)
        {
        DgnImportContext import3(*db, *db);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( !model3.IsValid() );
        ASSERT_NE( DgnDbStatus::Success , stat );
        }

    //  *******************************
    //  Import into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        checkGroupHasOneMemberInModel(*model3);
        db2->SaveChanges();
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static GeometricElementCPtr insertElement(DgnDbR db, DgnModelId mid, bool is3d, DgnSubCategoryId subcat)
    {
    DgnCategoryId cat = db.Categories().QueryCategoryId(subcat);

    GeometricElementPtr gelem;
    if (is3d)
        gelem = PhysicalElement::Create(PhysicalElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "PhysicalElement")), cat, Placement3d()));
    else
        gelem = DrawingElement::Create(DrawingElement::CreateParams(db, mid, DgnClassId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "DrawingElement")), cat, Placement2d()));

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(*gelem);
    builder->Append(subcat);
    builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,0,0))));

    if (SUCCESS != builder->SetGeomStreamAndPlacement(*gelem))
        return nullptr;

    return db.Elements().Insert(*gelem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static void getFirstElemDisplayParams(ElemDisplayParams& ret, GeometricElementCR gel)
    {
    ElementGeometryCollection gcollection(gel);
    gcollection.begin(); // has the side-effect of setting up the current element display params on the collection
    ret = gcollection.GetElemDisplayParams();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnCategories::SubCategory::Appearance createAppearance(ColorDef const& cdef)
    {
    DgnCategories::SubCategory::Appearance appearance;
    appearance.SetColor(ColorDef(1, 2, 3, 0));
    return appearance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
static DgnCategoryId createCategory(DgnDbR db, Utf8CP name, DgnCategories::Scope scope, DgnCategories::SubCategory::Appearance const& defaultAppearance)
    {
    DgnCategories::Category cat(name, scope);
    if (BE_SQLITE_OK != db.Categories().Insert(cat, defaultAppearance))
        return DgnCategoryId();
    return cat.GetCategoryId();
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
    DgnMaterials::Material lmat = ldb.Materials().Query(lmatid);
    DgnMaterials::Material rmat = ldb.Materials().Query(rmatid);
    if (!lmat.IsValid() || !rmat.IsValid())
        return false;
    return lmat.GetValue() == rmat.GetValue();
    }

struct NullContext : ViewContext
{
void _AllocateScanCriteria () override{;}
QvElem* _DrawCached (IStrokeForCache&){return nullptr;}
void _DrawSymbol (IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight) override {}
void _DeleteSymbol (IDisplaySymbol*) override {}
bool _FilterRangeIntersection (GeometricElementCR element) override {return false;}
void _CookDisplayParams (ElemDisplayParamsR, ElemMatSymbR) override {}
void _CookDisplayParamsOverrides (ElemDisplayParamsR, OvrMatSymbR) override {}
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

    NullContext lcontext(ldb);
    lhs.Resolve(lcontext);
    NullContext rcontext(rdb);
    rhs.Resolve(rcontext);

    rhs.SetCategoryId(lhs.GetCategoryId());
    lhs.SetMaterial(DgnMaterialId());
    rhs.SetMaterial(DgnMaterialId());
    lhs.SetLineStyle(nullptr);
    rhs.SetLineStyle(nullptr);
    lhs.SetGradient(nullptr);
    rhs.SetGradient(nullptr);
    lhs.SetPatternParams(nullptr);
    rhs.SetPatternParams(nullptr);

    if (!(lhs == rhs))
        return false;

    if (!areMaterialsEqual(lhs.GetMaterial(), ldb, rhs.GetMaterial(), rdb))
        return false;

    // *** TBD linestyles
    // *** TBD Gradient
    // *** TBD PatternParams

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportElementAndCategory1)
{
    static Utf8CP s_catName="MyCat";

    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP sourceDb = tdm.GetDgnProjectP();

    ASSERT_EQ(DgnDbStatus::Success, DgnPlatformTestDomain::ImportSchema(*sourceDb));

    //  Create a Category for the element. 
    DgnCategories::SubCategory::Appearance sourceAppearanceRequested = createAppearance(ColorDef(1, 2, 3, 0));
    DgnCategoryId sourceCategoryId = createCategory(*sourceDb, s_catName, DgnCategories::Scope::Analytical, sourceAppearanceRequested);
    ASSERT_TRUE( sourceCategoryId.IsValid() );
    DgnSubCategoryId sourceSubCategoryId = sourceDb->Categories().DefaultSubCategoryId(sourceCategoryId);
    ASSERT_TRUE( sourceSubCategoryId.IsValid() );

    //  Create the source model
    PhysicalModelPtr sourcemod = createPhysicalModel(*sourceDb, "sourcemod");
    ASSERT_TRUE( sourcemod.IsValid() );

    // Put an element in this category into the source model
    GeometricElementCPtr sourceElem = insertElement(*sourceDb, sourcemod->GetModelId(), true, sourceSubCategoryId);
    sourceDb->SaveChanges();

    ElemDisplayParams sourceDisplayParams;
    getFirstElemDisplayParams(sourceDisplayParams, *sourceElem);

    ASSERT_EQ( sourceCategoryId , sourceElem->GetCategoryId() ); // check that the source element really was assigned to the Category that I specified above
    ASSERT_EQ( sourceSubCategoryId , sourceDisplayParams.GetSubCategoryId() ); // check that the source element's geometry really was assigned to the SubCategory that I specified above
//*** Have to "cook" first    ASSERT_TRUE( sourceDisplayParams.GetLineColor() == sourceAppearanceRequested.GetColor() ); // check that the source element's geometry has the requested appearance
//*** Have to "cook" first    ASSERT_TRUE( sourceDisplayParams.GetMaterial() == sourceAppearanceRequested.GetMaterial() ); // check that the source element's geometry has the requested appearance

    if (true)
    {
        //  Create a second Db
        DgnDbPtr destDb = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE(destDb.IsValid());

        // Insert another category into the destination DB, just to make sure that IDs don't line up
        ASSERT_TRUE( createCategory(*destDb, "Unrelated", DgnCategories::Scope::Any, createAppearance(ColorDef(7,8,9,10))).IsValid() );

        PhysicalModelPtr destmod = createPhysicalModel(*destDb, "destmod");
        ASSERT_TRUE( destmod.IsValid() );

        DgnImportContext importContext(*sourceDb, *destDb);

        DgnDbStatus istatus;
        DgnElementCPtr destElem = sourceElem->Import(&istatus, *destmod, importContext);
        ASSERT_TRUE( destElem.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , istatus );

        DgnElementCPtr destElemFound = getSingleElementInModel(*destmod);
        ASSERT_TRUE(destElemFound.IsValid());
        ASSERT_EQ(destElemFound->GetElementId(), destElem->GetElementId());

        GeometricElementCP gdestElem = destElem->ToGeometricElement();
        ASSERT_TRUE(nullptr != gdestElem);
        DgnCategories::Category destcat = destDb->Categories().Query(gdestElem->GetCategoryId());
        ASSERT_TRUE( destcat.IsValid() );
        ASSERT_NE( destcat.GetCategoryId() , sourceCategoryId ) << "source element's Category should have been deep-copied and remapped to a new Category in destination DB";
        ASSERT_STREQ(s_catName, destcat.GetCode());
        ElemDisplayParams destDisplayParams;
        getFirstElemDisplayParams(destDisplayParams, *gdestElem);
        DgnSubCategoryId destSubCategoryId = destDisplayParams.GetSubCategoryId();
        ASSERT_TRUE( destSubCategoryId.IsValid() );
        //ASSERT_TRUE( destSubCategoryId != sourceSubCategoryId );   don't know what Id it was assigned
        DgnCategories::SubCategory destSubCategory = destDb->Categories().QuerySubCategory(destSubCategoryId);
        ASSERT_TRUE( destSubCategory.IsValid() );
        ASSERT_TRUE( areDisplayParamsEqual(sourceDisplayParams, *sourceDb, destDisplayParams, *destDb) );
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

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

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
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model1"));
    ASSERT_EQ( DgnDbStatus::Success , model1->Insert() );

    // Put an element with an Item into moddel1
        {
        DgnElement::Code code = auth1->CreateCode("TestElement");
        DgnCategoryId gcatid = db->Categories().QueryHighestId();
        TestElementPtr tempEl = TestElement::Create(*db, model1->GetModelId(), gcatid, code);
        DgnElement::Item::SetItem(*tempEl, *TestItem::Create("Line"));
        ASSERT_TRUE( db->Elements().Insert(*tempEl).IsValid() );
        db->SaveChanges();
        }

    if (true)
        {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_TRUE( el.IsValid() );
        DgnAuthorityId said = el->GetCode().GetAuthority();
        ASSERT_TRUE( said == sourceAuthorityId );
        auto sourceAuthority = db->Authorities().GetAuthority(sourceAuthorityId);
        ASSERT_STREQ( sourceAuthority->GetName().c_str(), "TestAuthority" );
        }

    //  *******************************
    //  Import model1 into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        DgnElementCPtr el = getSingleElementInModel(*model3);
        ASSERT_TRUE( el.IsValid() );

        // Verify that Authority was copied over
        DgnAuthorityId daid = el->GetCode().GetAuthority();
        ASSERT_TRUE( daid.IsValid() );
        ASSERT_NE( daid , sourceAuthorityId ) << "Authority ID should have been remapped";
        auto destAuthority = db2->Authorities().GetAuthority(daid);
        ASSERT_STREQ( destAuthority->GetName().c_str(), "TestAuthority" );
        db2->SaveChanges();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportElementsWithItems)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

    // ******************************
    //  Create model1
        
    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model1"));
    ASSERT_EQ( DgnDbStatus::Success , model1->Insert() );

    // Put an element with an Item into moddel1
        {
        DgnCategoryId gcatid = db->Categories().QueryHighestId();
        TestElementPtr tempEl = TestElement::Create(*db, model1->GetModelId(), gcatid, "TestElement");
        DgnElement::Item::SetItem(*tempEl, *TestItem::Create("Line"));
        ASSERT_TRUE( db->Elements().Insert(*tempEl).IsValid() );
        db->SaveChanges();
        }

    if (true)
        {
        DgnElementCPtr el = getSingleElementInModel(*model1);
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*el) );
        }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
        {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE( model2.IsValid() );

        DgnElementCPtr el = getSingleElementInModel(*model2);
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*el) );
        }

    //  *******************************
    //  Import into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        DgnElementCPtr el = getSingleElementInModel(*model3);
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*el) );
        db2->SaveChanges();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      05/15
//---------------------------------------------------------------------------------------
TEST_F(ImportTest, ImportElementsWithDependencies)
    {
    DgnDbTestDgnManager tdm(L"3dMetricGeneral.idgndb", __FILE__, Db::OpenMode::ReadWrite);
    DgnDbP db = tdm.GetDgnProjectP();
    db->Txns().EnableTracking(true);

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*db) );

    TestElementDrivesElementHandler::GetHandler().Clear();

    // ******************************
    //  Create model1
        
    DgnClassId mclassId = DgnClassId(db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr model1 = new PhysicalModel(PhysicalModel::CreateParams(*db, mclassId, "Model1"));
    ASSERT_EQ( DgnDbStatus::Success , model1->Insert() );

    // Create 2 elements and make the first depend on the second
        {
        DgnCategoryId gcatid = db->Categories().QueryHighestId();

        TestElementPtr e1 = TestElement::Create(*db, model1->GetModelId(), gcatid, "e1");
        ASSERT_TRUE( db->Elements().Insert(*e1).IsValid() );

        TestElementPtr e2 = TestElement::Create(*db, model1->GetModelId(), gcatid, "e2");
        ASSERT_TRUE( db->Elements().Insert(*e2).IsValid() );

        TestElementDrivesElementHandler::Insert(*db, e2->GetElementId(), e1->GetElementId());

        db->SaveChanges();

        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
        TestElementDrivesElementHandler::GetHandler().Clear();
        }

    //  ******************************
    //  Create model2 as a copy of model1
    if (true)
        {
        PhysicalModelPtr model2 = copyPhysicalModelSameDb(*model1, "Model2");
        ASSERT_TRUE( model2.IsValid() );

        db->SaveChanges();
        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
        TestElementDrivesElementHandler::GetHandler().Clear();
        }

    //  *******************************
    //  Import into separate db
    if (true)
        {
        DgnDbPtr db2 = openCopyOfDb(L"DgnDb/3dMetricGeneral.idgndb", L"3dMetricGeneralcc.idgndb", DgnDb::OpenMode::ReadWrite);
        ASSERT_TRUE( db2.IsValid() );

        DgnImportContext import3(*db, *db2);
        DgnDbStatus stat;
        PhysicalModelPtr model3 = DgnModel::Import(&stat, *model1, import3);
        ASSERT_TRUE( model3.IsValid() );
        ASSERT_EQ( DgnDbStatus::Success , stat );

        db2->SaveChanges();
        ASSERT_EQ( TestElementDrivesElementHandler::GetHandler().m_relIds.size(), 1 );
        TestElementDrivesElementHandler::GetHandler().Clear();
        }
    }
