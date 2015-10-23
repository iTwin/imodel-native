/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/MissingHandler_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
static PhysicalElement::CreateParams makeCreateParams(DgnDbR db, DgnModelId model, DgnClassId classId, DgnCategoryId cat, DgnElementId parentId=DgnElementId())
    {
    return PhysicalElement::CreateParams(db, model, classId, cat, Placement3d(), DgnElement::Code(), DgnElementId(), parentId);
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

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(MHTEST_SCHEMA, MHTEST_ELEM1_CLASS)); }

    static uint64_t GetRestrictions()
        {
        return RestrictedAction::Delete
             | RestrictedAction::InsertChild
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

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(MHTEST_SCHEMA, MHTEST_ELEM2_CLASS)); }

    static uint64_t GetRestrictions()
        {
        return RestrictedAction::Clone
             | RestrictedAction::SetParent
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
public:
    MissingHandlerDomain() : DgnDomain(MHTEST_SCHEMA, "Missing Handlers domain", 1)
        {
        RegisterHandler(Elem1Handler::GetHandler());
        RegisterHandler(Elem2Handler::GetHandler());
        }

    void ImportSchema(DgnDbR db, BeFileNameCR schemasDir)
        {
        BeFileName schemaFile = schemasDir;
        schemaFile.AppendToPath(L"ECSchemas/" MHTEST_SCHEMAW L".01.00.ecschema.xml");
        ASSERT_TRUE(DgnDbStatus::Success == DgnBaseDomain::GetDomain().ImportSchema(db, schemaFile));
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
    typedef PhysicalElement::RestrictedAction Restriction;

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

    template<typename T> void CreateElement(ElemInfo& info, DgnDbR db);
    DgnElementId CreatePhysicalElement(DgnDbR db, DgnElementId parentId = DgnElementId());
    void InitDb(DgnDbR db);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId MissingHandlerTest::CreatePhysicalElement(DgnDbR db, DgnElementId parentId)
    {
    PhysicalElement elem(makeCreateParams(db, m_defaultModelId, PhysicalElement::QueryClassId(db), m_defaultCategoryId, parentId));
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
    m_defaultModelId = db.Models().QueryFirstModelId();
    DgnModelPtr defaultModel = db.Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(defaultModel.IsValid());
    ASSERT_TRUE(nullptr != defaultModel->ToPhysicalModel());

    m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(db);
    DgnCategory altCat(DgnCategory::CreateParams(db, "AltCategory", DgnCategory::Scope::Any));
    ASSERT_TRUE(altCat.Insert(DgnSubCategory::Appearance()).IsValid());
    m_alternateCategoryId = altCat.GetCategoryId();
    ASSERT_TRUE(m_alternateCategoryId.IsValid());

    CreateElement<Elem1>(m_elem1Info, db);
    CreateElement<Elem2>(m_elem2Info, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MissingHandlerTest, HandlerRestrictions)
    {
    static const WCharCP s_dbName = L"HandlerRestrictions.idgndb";
    BeFileName fullDgnDbFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(fullDgnDbFileName, L"3dMetricGeneral.idgndb", s_dbName, __FILE__));

    // Create a new dgndb, with our domain + handlers loaded
        {
        ScopedDgnHost host;
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, fullDgnDbFileName, DgnDb::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());

        // register domain and handlers
        MissingHandlerDomain domain;
        DgnDomains::RegisterDomain(domain);
        domain.ImportSchema(*db, T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());

        // Populate the db with elements belonging to our domain
        InitDb(*db);

        ASSERT_TRUE(nullptr != db->Domains().FindDomain(MHTEST_SCHEMA));

        // Host terminated; DgnDb closed; domain and handlers no longer registered
        }

    // Reopen the dgndb, without our domain + handlers loaded
        {
        ScopedDgnHost host;
        DgnDbPtr db = DgnDb::OpenDgnDb(nullptr, fullDgnDbFileName, DgnDb::OpenParams(BeSQLite::Db::OpenMode::ReadWrite));
        ASSERT_TRUE(db.IsValid());

        ASSERT_TRUE(nullptr == db->Domains().FindDomain(MHTEST_SCHEMA));
        }
    }

