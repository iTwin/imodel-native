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
#define TMTEST_TEST_ITEM_CLASS_NAME                      "TestItem"
#define TMTEST_TEST_ITEM_TestItemProperty                "TestItemProperty"
#define TMTEST_TEST_ASPECT_CLASS_NAME                    "TestAspect"
#define TMTEST_TEST_ASPECT_TestAspectProperty            "TestAspectProperty"

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
    RefCountedPtr<DgnElement::Aspect> _CreateInstance() override {return new TestItem("");}
};

HANDLER_DEFINE_MEMBERS(TestItemHandler)

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestAspect : DgnPlatform::DgnElement::Aspect1_1
{
    DEFINE_T_SUPER(DgnPlatform::DgnElement::Aspect1_1)
private:
    friend struct TestAspectHandler;

    Utf8String m_testAspectProperty;

    explicit TestAspect(Utf8CP prop) : m_testAspectProperty(prop) {;}

    Utf8String _GetECSchemaName() override {return TMTEST_SCHEMA_NAME;}
    Utf8String _GetECClassName() override {return TMTEST_TEST_ASPECT_CLASS_NAME;}
    DgnDbStatus _LoadProperties(DgnElementCR el) override;
    DgnDbStatus _UpdateProperties(DgnElementCR el) override;

public:
    static RefCountedPtr<TestAspect> Create(Utf8CP prop) {return new TestAspect(prop);}

    static ECN::ECClassCP GetECClass(DgnDbR db) {return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_ASPECT_CLASS_NAME);}

    Utf8StringCR GetTestAspectProperty() const {return m_testAspectProperty;}
    void SetTestAspectProperty(Utf8CP s) {m_testAspectProperty = s;}
};

typedef RefCountedPtr<TestAspect> TestAspectPtr;
typedef RefCountedCPtr<TestAspect> TestAspectCPtr;
typedef TestAspect& TestAspectR;
typedef TestAspect const& TestAspectCR;
typedef TestAspect const* TestAspectCP;
typedef TestAspect* TestAspectP;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestAspectHandler : DgnPlatform::ElementAspectHandler
{
    DOMAINHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ASPECT_CLASS_NAME, TestAspectHandler, DgnPlatform::ElementAspectHandler, )
    RefCountedPtr<DgnElement::Aspect> _CreateInstance() override {return new TestAspect("");}
};

HANDLER_DEFINE_MEMBERS(TestAspectHandler)

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct ElementItemTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(ElementItemTestDomain, )
public:
    ElementItemTestDomain();
    };

DOMAIN_DEFINE_MEMBERS(ElementItemTestDomain)

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
ElementItemTestDomain::ElementItemTestDomain() : DgnDomain(TMTEST_SCHEMA_NAME, "DgnProject Test Schema", 1)
    {
    RegisterHandler(TestElementHandler::GetHandler());
    RegisterHandler(TestItemHandler::GetHandler());
    RegisterHandler(TestAspectHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementItemTests::ElementItemTests()
    {
    // Must register my domain whenever I initialize a host
    DgnDomains::RegisterDomain(ElementItemTestDomain::GetDomain()); 
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

    BentleyStatus status = ElementItemTestDomain::GetDomain().ImportSchema(*m_db, schemaFile);
    ASSERT_TRUE(BentleyStatus::SUCCESS == status);

    auto schema = m_db->Schemas().GetECSchema(TMTEST_SCHEMA_NAME, true);
    ASSERT_NE( nullptr , schema );
    ASSERT_TRUE( TestElement::QueryClassId(*m_db).IsValid() );
    ASSERT_NE( nullptr , TestAspect::GetECClass(*m_db) );

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
DgnDbStatus TestAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " TMTEST_TEST_ASPECT_TestAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, el.GetElementId());
    
    if (BeSQLite::EC::ECSqlStepStatus::HasRow != stmt->Step())
        return DgnDbStatus::ElementReadError;

    m_testAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestAspect::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " TMTEST_TEST_ASPECT_TestAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, el.GetElementId());
    
    return (BeSQLite::EC::ECSqlStepStatus::Done != stmt->Step())? DgnDbStatus::ElementWriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, ItemCRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ItemCRUD.idgndb", Db::OPEN_ReadWrite);

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
        item->SetTestItemProperty("Circle");
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

    ASSERT_TRUE( el.IsValid() );

    if (true)
        {
        //  Delete the item
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        DgnElement::Item::DeleteItem(*tempEl);
        ASSERT_EQ( nullptr , DgnElement::Item::Get<TestItem>(*tempEl) ) << "Item should not be returned when scheduled for drop";
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    ASSERT_TRUE( el.IsValid() );
    ASSERT_EQ( nullptr , DgnElement::Item::Get<TestItem>(*el) ) << "Item should now be gone";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, Aspect_1_1_CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"Aspect1_1CRUD.idgndb", Db::OPEN_ReadWrite);
    ECN::ECClassCR aclass = *TestAspect::GetECClass(*m_db);
    TestElementCPtr el;
    if (true)
        {
        //  Insert an element ...
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        ASSERT_EQ( nullptr , DgnElement::Aspect1_1::GetAspect(*tempEl, aclass) ) << "element should not yet have an aspect";
        //  ... with an aspect
        DgnElement::Aspect1_1::SetAspect(*tempEl, *TestAspect::Create("Line"));  // Initial geometry should be a line
        ASSERT_NE( nullptr , DgnElement::Aspect1_1::GetAspect(*tempEl, aclass) ) << "element should have a scheduled aspect";
        el = m_db->Elements().Insert(*tempEl);
        }

    ASSERT_TRUE( el.IsValid() );

    if (true)
        {
        //  Verify that aspect was saved in the Db
        TestAspectCP aspect = DgnElement::Aspect1_1::Get<TestAspect>(*el, aclass);
        ASSERT_NE( nullptr , aspect ) << "element should have a peristent aspect";
        ASSERT_STREQ( "Line" , aspect->GetTestAspectProperty().c_str() );
        }

    if (true)
        {
        //  Update the aspect
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        TestAspectP aspect = DgnElement::Aspect1_1::GetP<TestAspect>(*tempEl, aclass);
        aspect->SetTestAspectProperty("Circle");
        TestAspectCP originalAspect1_1 = DgnElement::Aspect1_1::Get<TestAspect>(*el, aclass);
        ASSERT_STREQ( "Line" , originalAspect1_1->GetTestAspectProperty().c_str() ) << "persistent aspect should remain unchanged until I call Update on the host element";
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    ASSERT_TRUE( el.IsValid() );
    
    if (true)
        {
        //  Verify that persistent aspect was changed
        TestAspectCP aspect = DgnElement::Aspect1_1::Get<TestAspect>(*el, aclass);
        ASSERT_NE( nullptr , aspect ) << "element should have a peristent aspect";
        ASSERT_STREQ( "Circle" , aspect->GetTestAspectProperty().c_str() ) << "I should see the changed value of the aspect now";
        }

    ASSERT_TRUE( el.IsValid() );

    if (true)
        {
        //  Delete the aspect
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        DgnElement::Aspect1_1::DeleteAspect(*tempEl, aclass);
        ASSERT_EQ( nullptr , DgnElement::Aspect1_1::Get<TestAspect>(*tempEl, aclass) ) << "Aspect1_1 should not be returned when scheduled for drop";
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    ASSERT_TRUE( el.IsValid() );
    ASSERT_EQ( nullptr , DgnElement::Aspect1_1::Get<TestAspect>(*el, aclass) ) << "Aspect1_1 should now be gone";
    }

