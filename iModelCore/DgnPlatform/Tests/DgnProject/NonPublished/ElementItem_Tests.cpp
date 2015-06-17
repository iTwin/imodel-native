/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementItem_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include <DgnPlatform/DgnCore/WebMercator.h>

#define TMTEST_SCHEMA_NAME                               "DgnPlatformTest"
#define TMTEST_SCHEMA_NAMEW                             L"DgnPlatformTest"
#define TMTEST_TEST_ELEMENT_CLASS_NAME                   "TestElement"
#define TMTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME    "TestElementDrivesElement"
#define TMTEST_TEST_ELEMENT_TestElementProperty          "TestElementProperty"
#define TMTEST_TEST_ITEM_CLASS_NAME                       "TestItem"
#define TMTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_UNNAMED_NAMESPACE

struct TestElementHandler;
struct TestItemHandler;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestElement : DgnPlatform::PhysicalElement
{
    DEFINE_T_SUPER(DgnPlatform::PhysicalElement)

    friend struct TestElementHandler;
public:
    TestElement(CreateParams const& params) : T_Super(params) {} 

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(TMTEST_SCHEMA_NAME, TMTEST_TEST_ELEMENT_CLASS_NAME));}
    static RefCountedPtr<TestElement> Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode);
};

typedef RefCountedPtr<TestElement> TestElementPtr;
typedef RefCountedCPtr<TestElement> TestElementCPtr;
typedef TestElement& TestElementR;
typedef TestElement const& TestElementCR;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestElementHandler : DgnPlatform::ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT_CLASS_NAME, TestElement, TestElementHandler, DgnPlatform::ElementHandler, )
};

HANDLER_DEFINE_MEMBERS(TestElementHandler)

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestItem : DgnPlatform::DgnElement::Item
{
    DEFINE_T_SUPER(DgnPlatform::DgnElement::Item)
private:
    friend struct TestItemHandler;

    Utf8String m_testItemProperty;

    explicit TestItem(Utf8CP prop) : m_testItemProperty(prop) {;}

    Utf8String _GetECSchemaName() override {return TMTEST_SCHEMA_NAME;}
    Utf8String _GetECClassName() override {return TMTEST_TEST_ITEM_CLASS_NAME;}
    DgnDbStatus _GenerateElementGeometry(GeometricElementR el) override;
    DgnDbStatus _LoadProperties(DgnElementCR el) override;
    DgnDbStatus _UpdateProperties(DgnElementCR el) override;

public:
    static RefCountedPtr<TestItem> Create(Utf8CP prop) {return new TestItem(prop);}

    Utf8StringCR GetTestItemProperty() const {return m_testItemProperty;}
    void SetTestItemProperty(Utf8CP s) {m_testItemProperty = s;}
};

typedef RefCountedPtr<TestItem> TestItemPtr;
typedef RefCountedCPtr<TestItem> TestItemCPtr;
typedef TestItem& TestItemR;
typedef TestItem const& TestItemCR;
typedef TestItem const* TestItemCP;
typedef TestItem* TestItemP;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestItemHandler : DgnPlatform::ElementAspectHandler
{
    DOMAINHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ITEM_CLASS_NAME, TestItemHandler, DgnPlatform::ElementAspectHandler, )
    RefCountedPtr<DgnElement::AspectBase> _CreateInstance() override {return new TestItem("");}
};

HANDLER_DEFINE_MEMBERS(TestItemHandler)

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TransactionManagerTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(TransactionManagerTestDomain, )
public:
    TransactionManagerTestDomain();
    };

DOMAIN_DEFINE_MEMBERS(TransactionManagerTestDomain)

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      06/15
+===============+===============+===============+===============+===============+======*/
struct ElementItemTests : public ::testing::Test
{
public:
    ScopedDgnHost m_host;
    DgnDbPtr      m_db;
    DgnModelId    m_defaultModelId;
    DgnCategoryId m_defaultCategoryId;

    ElementItemTests();
    ~ElementItemTests();
    void CloseDb() {m_db->CloseDb();}
    DgnModelR GetDefaultModel() {return *m_db->Models().GetModel(m_defaultModelId);}
    void SetupProject(WCharCP projFile, WCharCP testFile, Db::OpenMode mode);
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TransactionManagerTestDomain::TransactionManagerTestDomain() : DgnDomain(TMTEST_SCHEMA_NAME, "DgnProject Test Schema", 1)
    {
    RegisterHandler(TestElementHandler::GetHandler());
    RegisterHandler(TestItemHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementItemTests::ElementItemTests()
    {
    // Must register my domain whenever I initialize a host
    DgnDomains::RegisterDomain(TransactionManagerTestDomain::GetDomain()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementItemTests::~ElementItemTests()
    {
    }

/*---------------------------------------------------------------------------------**//**
* set up method that opens an existing .dgndb project file after copying it to out
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementItemTests::SetupProject(WCharCP projFile, WCharCP testFile, Db::OpenMode mode)
    {
    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, projFile, testFile, __FILE__));
    DbResult result;
    m_db = DgnDb::OpenDgnDb(&result, outFileName, DgnDb::OpenParams(mode));
    ASSERT_TRUE(m_db.IsValid());
    ASSERT_TRUE( result == BE_SQLITE_OK);

    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(L"ECSchemas/" TMTEST_SCHEMA_NAMEW L".01.00.ecschema.xml");

    BentleyStatus status = TransactionManagerTestDomain::GetDomain().ImportSchema(*m_db, schemaFile);
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);

    auto schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    ASSERT_NE( nullptr , schema );
    ASSERT_TRUE( TestElement::QueryClassId(*m_db).IsValid() );

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelP defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_NE( nullptr , defaultModel );
    GetDefaultModel().FillModel();

    m_defaultCategoryId = m_db->Categories().MakeIterator().begin().GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_GenerateElementGeometry(GeometricElementR el)
    {
    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::CreateWorld(el);
    
    // We make the element geometry depend on the item's property. 
    //  In a real app, of course, this property would be something realistic, not a string.
    if (m_testItemProperty.EqualsI("Line"))
        builder->Append(*ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::FromZero(), DPoint3d::From(1,1,1))));
    else if (m_testItemProperty.EqualsI("Circle"))
        builder->Append(*ICurvePrimitive::CreateArc(DEllipse3d::FromXYMajorMinor(0,0,0, 10,10, 0,0, Angle::PiOver2())));
    else
        return DgnDbStatus::ElementWriteError;
    
    if (BSISUCCESS != builder->SetGeomStreamAndPlacement(el))
        return DgnDbStatus::ElementWriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementPtr TestElement::Create(DgnDbR db, DgnModelId mid, DgnCategoryId categoryId, Utf8CP elementCode)
    {
    DgnModelP model = db.Models().GetModel(mid);

    TestElementPtr testElement = new TestElement(CreateParams(*model, QueryClassId(db), categoryId));

    return testElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " TMTEST_TEST_ITEM_TestItemProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, el.GetElementId());
    
    if (BeSQLite::EC::ECSqlStepStatus::HasRow != stmt->Step())
        return DgnDbStatus::ElementReadError;

    m_testItemProperty = stmt->GetValueText(0);
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestItem::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " TMTEST_TEST_ITEM_TestItemProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testItemProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, el.GetElementId());
    
    return (BeSQLite::EC::ECSqlStepStatus::Done != stmt->Step())? DgnDbStatus::ElementWriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, ItemCRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"TransactionManagerTests_StreetMapModel.idgndb", Db::OPEN_ReadWrite);

    TestElementCPtr el;
    if (true)
        {
        //  Insert an element ...
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        ASSERT_EQ( nullptr , DgnElement::Item::GetItem(*tempEl) ) << "element should not yet have an item";
        //  ... with an item
        DgnElement::Item::SetItem(*tempEl, *TestItem::Create("Line"));  // Initial geometry should be a line
        ASSERT_NE( nullptr , DgnElement::Item::GetItem(*tempEl) ) << "element should have a scheduled item";
        el = m_db->Elements().Insert(*tempEl);
        }

    ASSERT_TRUE( el.IsValid() );

    if (true)
        {
        //  Verify that item was saved in the Db
        TestItemCP item = DgnElement::Item::Get<TestItem>(*el);
        ASSERT_NE( nullptr , item ) << "element should have a peristent item";
        ASSERT_STREQ( "Line" , item->GetTestItemProperty().c_str() );
    
        //  Verify that item generated a line
        size_t count=0;
        for (ElementGeometryPtr geom : ElementGeometryCollection (*el))
            {
            ICurvePrimitivePtr curve = geom->GetAsICurvePrimitive();
            ASSERT_TRUE( curve.IsValid() );
            ASSERT_TRUE( curve->GetLineStringCP() != nullptr );
            ++count;
            }
        ASSERT_EQ( 1 , count );
        }

    if (true)
        {
        //  Update the item
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        TestItemP item = DgnElement::Item::GetP<TestItem>(*tempEl);
        item->SetTestItemProperty("2");
        TestItemCP originalItem = DgnElement::Item::Get<TestItem>(*el);
        ASSERT_STREQ( "Line" , originalItem->GetTestItemProperty().c_str() ) << "persistent item should remain unchanged until I call Update on the host element";
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    ASSERT_TRUE( el.IsValid() );
    if (true)
        {
        //  Verify that persistent item was changed
        TestItemCP item = DgnElement::Item::Get<TestItem>(*el);
        ASSERT_NE( nullptr , item ) << "element should have a peristent item";
        ASSERT_STREQ( "Circle" , item->GetTestItemProperty().c_str() ) << "I should see the changed value of the item now";

        //  Verify that item generated a circle
        size_t count=0;
        for (ElementGeometryPtr geom : ElementGeometryCollection (*el))
            {
            ICurvePrimitivePtr curve = geom->GetAsICurvePrimitive();
            ASSERT_TRUE( curve.IsValid() );
            ASSERT_TRUE( curve->GetArcCP() != nullptr );
            ++count;
            }
        ASSERT_EQ( 1 , count );
        }
    }

