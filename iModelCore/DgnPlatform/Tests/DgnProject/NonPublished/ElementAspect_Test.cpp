/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementAspect_Test.cpp $
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
#define TMTEST_TEST_UNIQUE_ASPECT_CLASS_NAME             "TestUniqueAspect"
#define TMTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty     "TestUniqueAspectProperty"
#define TMTEST_TEST_MULTI_ASPECT_CLASS_NAME              "TestMultiAspect"
#define TMTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "TestMultiAspectProperty"

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
struct TestElementHandler : DgnPlatform::dgn_ElementHandler::Element
{
    ELEMENTHANDLER_DECLARE_MEMBERS(TMTEST_TEST_ELEMENT_CLASS_NAME, TestElement, TestElementHandler, DgnPlatform::dgn_ElementHandler::Element, )
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
struct TestUniqueAspect : DgnPlatform::DgnElement::UniqueAspect
{
    DEFINE_T_SUPER(DgnPlatform::DgnElement::UniqueAspect)
private:
    friend struct TestUniqueAspectHandler;

    Utf8String m_testUniqueAspectProperty;

    explicit TestUniqueAspect(Utf8CP prop) : m_testUniqueAspectProperty(prop) {;}

    Utf8String _GetECSchemaName() override {return TMTEST_SCHEMA_NAME;}
    Utf8String _GetECClassName() override {return TMTEST_TEST_UNIQUE_ASPECT_CLASS_NAME;}
    DgnDbStatus _LoadProperties(DgnElementCR el) override;
    DgnDbStatus _UpdateProperties(DgnElementCR el) override;

public:
    static RefCountedPtr<TestUniqueAspect> Create(Utf8CP prop) {return new TestUniqueAspect(prop);}

    static ECN::ECClassCP GetECClass(DgnDbR db) {return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_UNIQUE_ASPECT_CLASS_NAME);}

    Utf8StringCR GetTestUniqueAspectProperty() const {return m_testUniqueAspectProperty;}
    void SetTestUniqueAspectProperty(Utf8CP s) {m_testUniqueAspectProperty = s;}
};

typedef RefCountedPtr<TestUniqueAspect> TestUniqueAspectPtr;
typedef RefCountedCPtr<TestUniqueAspect> TestUniqueAspectCPtr;
typedef TestUniqueAspect& TestUniqueAspectR;
typedef TestUniqueAspect const& TestUniqueAspectCR;
typedef TestUniqueAspect const* TestUniqueAspectCP;
typedef TestUniqueAspect* TestUniqueAspectP;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestUniqueAspectHandler : DgnPlatform::ElementAspectHandler
{
    DOMAINHANDLER_DECLARE_MEMBERS(TMTEST_TEST_UNIQUE_ASPECT_CLASS_NAME, TestUniqueAspectHandler, DgnPlatform::ElementAspectHandler, )
    RefCountedPtr<DgnElement::Aspect> _CreateInstance() override {return new TestUniqueAspect("");}
};

HANDLER_DEFINE_MEMBERS(TestUniqueAspectHandler)

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestMultiAspect : DgnPlatform::DgnElement::MultiAspect
{
    DEFINE_T_SUPER(DgnPlatform::DgnElement::MultiAspect)
private:
    friend struct TestMultiAspectHandler;

    Utf8String m_testMultiAspectProperty;

    explicit TestMultiAspect(Utf8CP prop) : m_testMultiAspectProperty(prop) {;}

    Utf8String _GetECSchemaName() override {return TMTEST_SCHEMA_NAME;}
    Utf8String _GetECClassName() override {return TMTEST_TEST_MULTI_ASPECT_CLASS_NAME;}
    DgnDbStatus _LoadProperties(DgnElementCR el) override;
    DgnDbStatus _UpdateProperties(DgnElementCR el) override;

public:
    static RefCountedPtr<TestMultiAspect> Create(Utf8CP prop) {return new TestMultiAspect(prop);}

    static ECN::ECClassCP GetECClass(DgnDbR db) {return db.Schemas().GetECClass(TMTEST_SCHEMA_NAME, TMTEST_TEST_MULTI_ASPECT_CLASS_NAME);}

    Utf8StringCR GetTestMultiAspectProperty() const {return m_testMultiAspectProperty;}
    void SetTestMultiAspectProperty(Utf8CP s) {m_testMultiAspectProperty = s;}
};

typedef RefCountedPtr<TestMultiAspect> TestMultiAspectPtr;
typedef RefCountedCPtr<TestMultiAspect> TestMultiAspectCPtr;
typedef TestMultiAspect& TestMultiAspectR;
typedef TestMultiAspect const& TestMultiAspectCR;
typedef TestMultiAspect const* TestMultiAspectCP;
typedef TestMultiAspect* TestMultiAspectP;

//=======================================================================================
// @bsiclass                                                     Sam.Wilson      06/15
//=======================================================================================
struct TestMultiAspectHandler : DgnPlatform::ElementAspectHandler
{
    DOMAINHANDLER_DECLARE_MEMBERS(TMTEST_TEST_MULTI_ASPECT_CLASS_NAME, TestMultiAspectHandler, DgnPlatform::ElementAspectHandler, )
    RefCountedPtr<DgnElement::Aspect> _CreateInstance() override {return new TestMultiAspect("");}
};

HANDLER_DEFINE_MEMBERS(TestMultiAspectHandler)

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
    RegisterHandler(TestUniqueAspectHandler::GetHandler());
    RegisterHandler(TestMultiAspectHandler::GetHandler());
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
    ASSERT_NE( nullptr , TestUniqueAspect::GetECClass(*m_db) );
    ASSERT_NE( nullptr , TestMultiAspect::GetECClass(*m_db) );

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
DgnDbStatus TestUniqueAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " TMTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetInstanceId());
    if (BeSQLite::EC::ECSqlStepStatus::HasRow != stmt->Step())
        return DgnDbStatus::ElementReadError;
    m_testUniqueAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestUniqueAspect::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " TMTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testUniqueAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, GetInstanceId());
    return (BeSQLite::EC::ECSqlStepStatus::Done != stmt->Step())? DgnDbStatus::ElementWriteError: DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_LoadProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT " TMTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty " FROM %s WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindId(1, GetInstanceId());
    if (BeSQLite::EC::ECSqlStepStatus::HasRow != stmt->Step())
        return DgnDbStatus::ElementReadError;
    m_testMultiAspectProperty = stmt->GetValueText(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TestMultiAspect::_UpdateProperties(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("UPDATE %s SET " TMTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "=? WHERE(ECInstanceId=?)", GetFullEcSqlClassName().c_str()));
    stmt->BindText(1, m_testMultiAspectProperty.c_str(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
    stmt->BindId(2, GetInstanceId());
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
        TestItemP item = DgnElement::Item::GetP<TestItem>(*tempEl);
        item->Delete();
        ASSERT_EQ( nullptr , DgnElement::Item::Get<TestItem>(*tempEl) ) << "Item should not be returned when scheduled for drop";
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    ASSERT_TRUE( el.IsValid() );
    ASSERT_EQ( nullptr , DgnElement::Item::Get<TestItem>(*el) ) << "Item should now be gone";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, UniqueAspect_CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"UniqueAspectCRUD.idgndb", Db::OPEN_ReadWrite);
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass(*m_db);
    TestElementCPtr el;
    if (true)
        {
        //  Insert an element ...
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        ASSERT_EQ( nullptr , DgnElement::UniqueAspect::GetAspect(*tempEl, aclass) ) << "element should not yet have an aspect";
        //  ... with an aspect
        DgnElement::UniqueAspect::SetAspect(*tempEl, *TestUniqueAspect::Create("Initial Value"));
        ASSERT_NE( nullptr , DgnElement::UniqueAspect::GetAspect(*tempEl, aclass) ) << "element should have a scheduled aspect";
        el = m_db->Elements().Insert(*tempEl);
        }

    ASSERT_TRUE( el.IsValid() );

    if (true)
        {
        //  Verify that aspect was saved in the Db
        TestUniqueAspectCP aspect = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, aclass);
        ASSERT_NE( nullptr , aspect ) << "element should have a peristent aspect";
        ASSERT_STREQ( "Initial Value" , aspect->GetTestUniqueAspectProperty().c_str() );

        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspect WHERE (ECInstanceId=?)");
        stmt->BindId(1, aspect->GetInstanceId());
        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::HasRow , stmt->Step() );
        ASSERT_STREQ( "Initial Value" , stmt->GetValueText(0) );
        }

    if (true)
        {
        //  Update the aspect
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        TestUniqueAspectP aspect = DgnElement::UniqueAspect::GetP<TestUniqueAspect>(*tempEl, aclass);
        ASSERT_EQ( aspect , DgnElement::UniqueAspect::GetP<TestUniqueAspect>(*tempEl, aclass) ) << "GetP should return the same instance each time we call it";
        ASSERT_EQ( aspect , DgnElement::UniqueAspect::GetP<TestUniqueAspect>(*tempEl, aclass) ) << "GetP should return the same instance each time we call it";
        aspect->SetTestUniqueAspectProperty("Changed Value");
        TestUniqueAspectCP originalUniqueAspect = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, aclass);
        ASSERT_STREQ( "Initial Value" , originalUniqueAspect->GetTestUniqueAspectProperty().c_str() ) << "persistent aspect should remain unchanged until I call Update on the host element";
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    ASSERT_TRUE( el.IsValid() );
    
    if (true)
        {
        //  Verify that persistent aspect was changed
        TestUniqueAspectCP aspect = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, aclass);
        ASSERT_NE( nullptr , aspect ) << "element should have a peristent aspect";
        ASSERT_STREQ( "Changed Value" , aspect->GetTestUniqueAspectProperty().c_str() ) << "I should see the changed value of the aspect now";

        // Verify that the aspect's property was changed in the Db
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspect WHERE (ECInstanceId=?)");
        stmt->BindId(1, aspect->GetInstanceId());
        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::HasRow , stmt->Step() );
        ASSERT_STREQ( "Changed Value" , stmt->GetValueText(0) );
        }

    ASSERT_TRUE( el.IsValid() );

    if (true)
        {
        //  Delete the aspect
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        TestUniqueAspectP aspect = DgnElement::UniqueAspect::GetP<TestUniqueAspect>(*tempEl, aclass);
        aspect->Delete();
        ASSERT_EQ( nullptr , DgnElement::UniqueAspect::Get<TestUniqueAspect>(*tempEl, aclass) ) << "UniqueAspect should not be returned when scheduled for drop";
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    ASSERT_TRUE( el.IsValid() );
    ASSERT_EQ( nullptr , DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, aclass) ) << "UniqueAspect should now be gone";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, MultiAspect_CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"MultiAspectCRUD.idgndb", Db::OPEN_ReadWrite);
    ECN::ECClassCR aclass = *TestMultiAspect::GetECClass(*m_db);
    TestElementCPtr el;
    EC::ECInstanceId a1id, a2id;
    if (true)
        {
        //  Insert an element ...
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        //  ... with an aspect
        TestMultiAspectPtr a1 = TestMultiAspect::Create("1");
        TestMultiAspectPtr a2 = TestMultiAspect::Create("2");
        DgnElement::MultiAspect::AddAspect(*tempEl, *a1);
        DgnElement::MultiAspect::AddAspect(*tempEl, *a2);
        el = m_db->Elements().Insert(*tempEl);
        a1id = a1->GetInstanceId();
        a2id = a2->GetInstanceId();
        }

    ASSERT_TRUE( el.IsValid() );
    ASSERT_TRUE( a1id.IsValid() );
    ASSERT_TRUE( a2id.IsValid() );

    m_db->SaveChanges();

    if (true)
        {
        // Verify that aspects were written to the Db and that the ElementOwnsAspects relationships were put into place
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement(
            "SELECT Aspect.ECInstanceId, Aspect.TestMultiAspectProperty FROM DgnPlatformTest.TestMultiAspect Aspect"
            " JOIN " DGN_SCHEMA(DGN_RELNAME_ElementOwnsAspects) " ON SourceECInstanceId=? AND TargetECClassId=?");
        stmt->BindId(1, el->GetElementId());
        stmt->BindInt64(2, aclass.GetId());
        bool has1=false;
        bool has2=false;
        // Note: select will often return the same aspect multiple times. Write the loop so that we don't care about that.
        while (stmt->Step() == BeSQLite::EC::ECSqlStepStatus::HasRow)
            {
            EC::ECInstanceId aspectId = stmt->GetValueId<EC::ECInstanceId>(0);
            Utf8CP propVal = stmt->GetValueText(1);
            if (a1id == aspectId)
                {
                has1 = true;
                ASSERT_STREQ( "1" , propVal );
                }
            else if (a2id == aspectId)
                {
                has2 = true;
                ASSERT_STREQ( "2" , propVal );
                }
            else
                {
                FAIL() << "Unknown aspect found";
                }
            }
        ASSERT_TRUE( has1 && has2 );
        }

    if (true)
        {
        // Modify an aspect
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        TestMultiAspectP aspect = DgnElement::MultiAspect::GetP<TestMultiAspect>(*tempEl, aclass, a2id);
        ASSERT_EQ( aspect , DgnElement::MultiAspect::GetP<TestMultiAspect>(*tempEl, aclass, a2id) ) << "GetP should return the same instance each time we call it";
        ASSERT_EQ( aspect , DgnElement::MultiAspect::GetP<TestMultiAspect>(*tempEl, aclass, a2id) ) << "GetP should return the same instance each time we call it";
        aspect->SetTestMultiAspectProperty("2 is Changed");
        ASSERT_TRUE( m_db->Elements().Update(*tempEl).IsValid() );
        }

    if (true)
        {
        // Verify that the a1's property is unchanged and a2's property was changed in the Db
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestMultiAspectProperty FROM DgnPlatformTest.TestMultiAspect WHERE (ECInstanceId=?)");
        stmt->BindId(1, a1id);
        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::HasRow , stmt->Step() );
        ASSERT_STREQ( "1" , stmt->GetValueText(0) );
        stmt->Reset();
        stmt->ClearBindings();
        stmt->BindId(1, a2id);
        ASSERT_EQ( BeSQLite::EC::ECSqlStepStatus::HasRow , stmt->Step() );
        ASSERT_STREQ( "2 is Changed" , stmt->GetValueText(0) );
        }

    if (true)
        {
        // Delete an aspect
        }
    }
