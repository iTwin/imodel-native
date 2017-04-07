/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/MissingHandler_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTest.h>

#define MHTEST_SCHEMA "DgnPlatformTest"
#define MHTEST_SCHEMAW L"DgnPlatformTest"
#define MHTEST_ELEM1_CLASS "RestrictedElement1"
#define MHTEST_ELEM2_CLASS "RestrictedElement2"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static PhysicalElement::CreateParams makeCreateParams(DgnDbR db, DgnModelId model, DgnClassId classId, DgnCategoryId cat, DgnElementId parentId=DgnElementId(), DgnClassId parentRelClassId=DgnClassId())
    {
    PhysicalElement::CreateParams createParams(db, model, classId, cat, Placement3d(), DgnCode(), nullptr, parentId);
    if (parentId.IsValid())
        createParams.m_parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalElementAssemblesElements);

    return createParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct Elem1 : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(MHTEST_ELEM1_CLASS, PhysicalElement);
public:
    Elem1(T_Super::CreateParams const& params) : T_Super(params) { }
    Elem1(DgnDbR db, DgnModelId model, DgnCategoryId category, DgnElementId parentId=DgnElementId())
        : Elem1(makeCreateParams(db, model, QueryClassId(db), category, parentId)) { }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetClassId(MHTEST_SCHEMA, MHTEST_ELEM1_CLASS)); }

    static uint64_t GetRestrictions()
        {
        return RestrictedAction::Delete
             | RestrictedAction::Insert
             | RestrictedAction::SetCode
             | RestrictedAction::DeleteChild
             | RestrictedAction::Move
             | RestrictedAction::SetGeometry;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct Elem2 : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(MHTEST_ELEM2_CLASS, PhysicalElement);
public:
    Elem2(T_Super::CreateParams const& params) : T_Super(params) { }
    Elem2(DgnDbR db, DgnModelId model, DgnCategoryId category, DgnElementId parentId=DgnElementId())
        : Elem2(makeCreateParams(db, model, QueryClassId(db), category, parentId)) { }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetClassId(MHTEST_SCHEMA, MHTEST_ELEM2_CLASS)); }

    static uint64_t GetRestrictions()
        {
        return RestrictedAction::Clone
             | RestrictedAction::SetParent
             | RestrictedAction::InsertChild
             | RestrictedAction::UpdateChild
             | RestrictedAction::SetCategory;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct Elem1Handler : dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS(MHTEST_ELEM1_CLASS, Elem1, Elem1Handler, dgn_ElementHandler::Physical, );
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct Elem2Handler : dgn_ElementHandler::Physical
{
    ELEMENTHANDLER_DECLARE_MEMBERS(MHTEST_ELEM2_CLASS, Elem2, Elem2Handler, dgn_ElementHandler::Physical, );
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct MissingHandlerDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(MissingHandlerDomain, );

private:
    WCharCP _GetSchemaRelativePath() const override { return L"ECSchemas/" MHTEST_SCHEMAW L".ecschema.xml"; }

public:
    MissingHandlerDomain() : DgnDomain(MHTEST_SCHEMA, "Missing Handlers domain", 1)
        {
        RegisterHandler(Elem1Handler::GetHandler());
        RegisterHandler(Elem2Handler::GetHandler());
        }

};

HANDLER_DEFINE_MEMBERS(Elem1Handler);
HANDLER_DEFINE_MEMBERS(Elem2Handler);
DOMAIN_DEFINE_MEMBERS(MissingHandlerDomain);

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct MissingHandlerTest : public ::testing::Test
{
public: 
    static void SetUpTestCase();
    typedef PhysicalElement::RestrictedAction Restriction;
    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;

    struct ElemInfo
    {
        DgnElementId    m_elemId;
        DgnElementId    m_parentId;
        DgnElementId    m_childId;
    };

    DgnModelId      m_defaultModelId;
    DgnCategoryId   m_defaultCategoryId;
    DgnCategoryId   m_alternateCategoryId;
    ElemInfo        m_elem1Info;
    ElemInfo        m_elem2Info;
    CodeSpecId      m_codeSpecId;

    template<typename T> void CreateElement(ElemInfo& info, DgnDbR db);
    DgnElementId CreatePhysicalElement(DgnDbR db, DgnElementId parentId = DgnElementId());
    void InitDb(DgnDbR db);
    void TestRestrictions(ElemInfo const& info, DgnDbR db, uint64_t restrictions);
    void TestDelete(DgnElementId elemId, DgnDbR db, bool allowed);
};

DgnPlatformSeedManager::SeedDbInfo MissingHandlerTest::s_seedFileInfo;
//---------------------------------------------------------------------------------------
// Do one-time setup for all tests in this group
// In this case, I just request the (root) seed file that my tests will use and make a note of it.
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
void MissingHandlerTest::SetUpTestCase()
    {
    ScopedDgnHost tempHost;
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(false, true));

    MissingHandlerTest::s_seedFileInfo = rootSeedInfo;
    MissingHandlerTest::s_seedFileInfo.fileName.SetName(L"MissingHandlerTest/Test.bim");

    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, MissingHandlerTest::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId MissingHandlerTest::CreatePhysicalElement(DgnDbR db, DgnElementId parentId)
    {
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());
    GenericPhysicalObject elem(makeCreateParams(db, m_defaultModelId, classId, m_defaultCategoryId, parentId));
    elem.Insert();
    return elem.GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void MissingHandlerTest::CreateElement(ElemInfo& info, DgnDbR db)
    {
    // An element of one of our types, with a PhysicalElement parent and a PhysicalElement child
    info.m_parentId = CreatePhysicalElement(db);
    ASSERT_TRUE(info.m_parentId.IsValid());

    T elem(db, m_defaultModelId, m_defaultCategoryId, info.m_parentId);
    ASSERT_TRUE(elem.Insert().IsValid());
    info.m_elemId = elem.GetElementId();
    ASSERT_TRUE(info.m_elemId.IsValid());

    info.m_childId = CreatePhysicalElement(db, info.m_elemId);
    ASSERT_TRUE(info.m_childId.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MissingHandlerTest::InitDb(DgnDbR db)
    {
    m_defaultModelId = DgnDbTestUtils::QueryFirstGeometricModelId(db);
    DgnModelPtr defaultModel = db.Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(defaultModel.IsValid());
    ASSERT_TRUE(defaultModel->IsPhysicalModel());

    m_defaultCategoryId = DgnDbTestUtils::GetFirstSpatialCategoryId(db);
    m_alternateCategoryId = DgnDbTestUtils::InsertSpatialCategory(db, "AltCategory");

    auto codeSpec = CodeSpec::Create(db, "MissingHandlerTest");
    ASSERT_TRUE(codeSpec.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, db.CodeSpecs().Insert(*codeSpec));
    m_codeSpecId = codeSpec->GetCodeSpecId();
    ASSERT_TRUE(m_codeSpecId.IsValid());

    CreateElement<Elem1>(m_elem1Info, db);
    CreateElement<Elem2>(m_elem2Info, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MissingHandlerTest::TestDelete(DgnElementId elemId, DgnDbR db, bool allowed)
    {
    auto cpElem = db.Elements().Get<GeometricElement3d>(elemId);
    ASSERT_TRUE(cpElem.IsValid());
    auto status = cpElem->Delete();
    if (allowed)
        EXPECT_EQ(DgnDbStatus::Success, status);
    else
        EXPECT_EQ(DgnDbStatus::MissingHandler, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr computeShape(double len)
    {
    DPoint3d pts[6];
    pts[0] = DPoint3d::From(-len, -len);
    pts[1] = DPoint3d::From(+len, -len);
    pts[2] = DPoint3d::From(+len, +len);
    pts[3] = DPoint3d::From(-len, +len);
    pts[4] = pts[0];
    pts[5] = pts[0];
    pts[5].z = 1;

    return CurveVector::CreateLinear(pts, _countof(pts), CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MissingHandlerTest::TestRestrictions(ElemInfo const& info, DgnDbR db, uint64_t restrictions)
    {
#define ALLOWED(ACTION) (0 == (restrictions & ACTION))

    // Look up the element
    DgnElementCPtr cpElem = db.Elements().Get<GeometricElement3d>(info.m_elemId);
    ASSERT_TRUE(cpElem.IsValid());

    // Verify handler
    EXPECT_EQ(0 != restrictions, cpElem->GetElementHandler()._IsMissingHandler());
    EXPECT_TRUE(nullptr != dynamic_cast<dgn_ElementHandler::Geometric3d*>(&cpElem->GetElementHandler()));

    // Clone
    DgnDbStatus status;
    auto clone = cpElem->Clone(&status);
    EXPECT_EQ(clone.IsValid(), ALLOWED(Restriction::Clone));
    EXPECT_EQ(DgnDbStatus::MissingHandler == status, !ALLOWED(Restriction::Clone));
    if (clone.IsValid())
        {
        EXPECT_EQ(&clone->GetElementHandler(), &cpElem->GetElementHandler());
        EXPECT_EQ(clone->GetElementClassId(), cpElem->GetElementClassId());
        }

    // Change parent
    auto pElem = cpElem->MakeCopy<GeometricElement3d>();
    ASSERT_TRUE(pElem.IsValid());
    DgnElementId parentId;
    DgnClassId parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
    ASSERT_TRUE(parentRelClassId.IsValid());
    status = pElem->SetParentId(parentId, parentRelClassId);
    EXPECT_EQ(DgnDbStatus::MissingHandler == status, !ALLOWED(Restriction::SetParent));
    EXPECT_EQ(ALLOWED(Restriction::SetParent), pElem->GetParentId() == parentId);

    // Change code
    static char s_codeChar = 'A';
    Utf8String codeValue(1, s_codeChar++);
    auto code = db.CodeSpecs().GetCodeSpec(m_codeSpecId)->CreateCode(codeValue);
    status = pElem->SetCode(code);
    EXPECT_EQ(DgnDbStatus::MissingHandler == status, !ALLOWED(Restriction::SetCode));

    // Change category
    status = pElem->SetCategoryId(m_alternateCategoryId);
    EXPECT_EQ(DgnDbStatus::MissingHandler == status, !ALLOWED(Restriction::SetCategory));
    EXPECT_EQ(ALLOWED(Restriction::SetCategory), pElem->GetCategoryId() == m_alternateCategoryId);

    // Change placement
    Placement3d placement;
    status = pElem->SetPlacement(placement);
    EXPECT_EQ(DgnDbStatus::MissingHandler == status, !ALLOWED(Restriction::Move));

    // Insert child
    DgnElementId newChildId = CreatePhysicalElement(db, info.m_elemId);
    EXPECT_EQ(newChildId.IsValid(), ALLOWED(Restriction::InsertChild));

    // Modify child
    auto cpChild = db.Elements().Get<GeometricElement3d>(info.m_childId);
    auto pChild = cpChild.IsValid() ? cpChild->MakeCopy<GeometricElement3d>() : nullptr;
    ASSERT_TRUE(pChild.IsValid());
    EXPECT_EQ(DgnDbStatus::Success, pChild->SetCategoryId(m_alternateCategoryId));
    EXPECT_EQ(pChild->Update().IsValid(), ALLOWED(Restriction::UpdateChild));

    // Delete child (schema is set up so that one element only supports insert, the other only supports delete - so try to delete the newly-added child
    if (newChildId.IsValid())
        {
        cpChild = db.Elements().Get<GeometricElement3d>(newChildId);
        ASSERT_TRUE(cpChild.IsValid());
        status = cpChild->Delete();
        if (ALLOWED(Restriction::DeleteChild))
            EXPECT_EQ(status, DgnDbStatus::Success);
        else
            EXPECT_EQ(status, DgnDbStatus::ParentBlockedChange);
        }

    // Change geometry
    auto model = db.Models().Get<SpatialModel>(m_defaultModelId);
    ASSERT_TRUE(model.IsValid());
    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, pElem->GetCategoryId(), placement.GetOrigin());
    ASSERT_TRUE(builder.IsValid());
    CurveVectorPtr shape = computeShape(100);
    builder->Append(*shape);
    DgnConeDetail cylinderDetail(DPoint3d::From(0,0,0), DPoint3d::From(0,0,3), 1.5, 1.5, true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    ASSERT_TRUE(cylinder.IsValid());
    builder->Append(*cylinder);
    EXPECT_EQ(SUCCESS == builder->Finish(*pElem), ALLOWED(Restriction::SetGeometry));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MissingHandlerTest, HandlerRestrictions)
    {
    BeFileName fullDgnDbFileName;

    // Create a new dgndb, with our domain + handlers loaded
        {
        ScopedDgnHost host;

        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(MissingHandlerTest::s_seedFileInfo.fileName, L"HandlerRestrictions");
        ASSERT_TRUE(db.IsValid());
        fullDgnDbFileName = db->GetFileName();


        // register domain and handlers
        MissingHandlerDomain domain;
        DgnDomains::RegisterDomain(domain, DgnDomain::Required::No, DgnDomain::Readonly::No);
		
		MissingHandlerDomain::GetDomain().ImportSchema(*db);

        // Populate the db with elements belonging to our domain
        InitDb(*db);

        ASSERT_TRUE(nullptr != db->Domains().FindDomain(MHTEST_SCHEMA));

        // Confirm all operations are permitted while handlers are loaded
        TestRestrictions(m_elem1Info, *db, 0);
        TestRestrictions(m_elem2Info, *db, 0);

        db->SaveChanges();
        // Host terminated; DgnDb closed; domain and handlers no longer registered
        }

    // Reopen the dgndb, without our domain + handlers loaded
        {
        ScopedDgnHost host;
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, fullDgnDbFileName, DgnDb::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());

        ASSERT_TRUE(nullptr == db->Domains().FindDomain(MHTEST_SCHEMA));

        // Confirm restricted operations are prohibited when handler not available
        TestRestrictions(m_elem1Info, *db, Elem1::GetRestrictions());
        TestRestrictions(m_elem2Info, *db, Elem2::GetRestrictions());

        // Confirm deletion restriction (NOTE: will result in our Elem2 element being deleted
        TestDelete(m_elem1Info.m_elemId, *db, false);
        TestDelete(m_elem2Info.m_elemId, *db, true);
        db->SaveChanges();
        }

    // Reopen the dgndb once more, with handlers loaded again
        {
        ScopedDgnHost host;

        // register domain and handlers
        MissingHandlerDomain domain;
        DgnDomains::RegisterDomain(domain, DgnDomain::Required::No, DgnDomain::Readonly::No);

        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, fullDgnDbFileName, DgnDb::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());

        // Confirm operations are all supported again now that handler is available
        TestRestrictions(m_elem1Info, *db, 0);
        TestDelete(m_elem1Info.m_elemId, *db, true);
        db->SaveChanges();
        }
    }

