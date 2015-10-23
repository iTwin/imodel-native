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
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnCore/WebMercator.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

BEGIN_UNNAMED_NAMESPACE

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
ElementItemTests::ElementItemTests()
    {
    DgnPlatformTestDomain::Register(); 
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

    ASSERT_EQ( DgnDbStatus::Success , DgnPlatformTestDomain::ImportSchema(*m_db) );

    m_defaultModelId = m_db->Models().QueryFirstModelId();
    DgnModelPtr defaultModel = m_db->Models().GetModel(m_defaultModelId);
    ASSERT_TRUE(defaultModel.IsValid());
    GetDefaultModel().FillModel();

    m_defaultCategoryId = DgnCategory::QueryFirstCategoryId(*m_db);
    }

#ifdef WIP_ELEMENT_ITEM // *** pending redesign
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, ElementsOwnsItemTest)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ElementsOwnsItemTest.idgndb", Db::OpenMode::ReadWrite);

    // Define an element with an item
    TestElementCPtr el;
    if (true)
        {
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        DgnElement::Item::SetItem(*tempEl, *TestItem::Create("Line"));
        el = m_db->Elements().Insert(*tempEl);
        m_db->SaveChanges();
        }

    //  Delete the element. 
    DgnElementId eid = el->GetElementId();
    m_db->Elements().Delete(*el);

    // Item should have been deleted for me.
    Statement findItem;
    findItem.Prepare(*m_db, "SELECT ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_ElementItem) " WHERE(ElementId=?)");
    findItem.BindId(1, eid);
    bool itemIsGone = (BE_SQLITE_ROW != findItem.Step());
    ASSERT_TRUE(itemIsGone);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, ItemCRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"ItemCRUD.idgndb", Db::OpenMode::ReadWrite);

    TestElementCPtr el;
    if (true)
        {
        //  Insert an element ...
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        ASSERT_EQ( nullptr , DgnElement::Item::GetItem(*tempEl) ) << "element should not yet have an item";
        //  ... with an item
        RefCountedPtr<TestItem> newItem = TestItem::Create("Line");
        BeTest::SetFailOnAssert(false);
        DgnElement::UniqueAspect::SetAspect(*tempEl, *newItem);  // THIS SHOULD FAIL -- wrong API
        BeTest::SetFailOnAssert(true);
        ASSERT_EQ( nullptr , DgnElement::Item::GetItem(*tempEl) ) << "element should not yet have an item";
        DgnElement::Item::SetItem(*tempEl, *newItem);  // Initial geometry should be a line
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
    
        BeTest::SetFailOnAssert(false);
        void const* wrong = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, *item->GetECClass(*m_db));
        BeTest::SetFailOnAssert(true);
        ASSERT_EQ( nullptr , wrong ) << "You should only be able to access an item through the item API";

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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementItemTests, UniqueAspect_CRUD)
    {
    SetupProject(L"3dMetricGeneral.idgndb", L"UniqueAspectCRUD.idgndb", Db::OpenMode::ReadWrite);
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
        stmt->BindId(1, aspect->GetAspectInstanceId(*el));
        ASSERT_EQ(BE_SQLITE_ROW , stmt->Step() );
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
        stmt->BindId(1, aspect->GetAspectInstanceId(*el));
        ASSERT_EQ(BE_SQLITE_ROW , stmt->Step() );
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
    SetupProject(L"3dMetricGeneral.idgndb", L"MultiAspectCRUD.idgndb", Db::OpenMode::ReadWrite);
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
        a1id = a1->GetAspectInstanceId();
        a2id = a2->GetAspectInstanceId();
        }

    ASSERT_TRUE( el.IsValid() );
    ASSERT_TRUE( a1id.IsValid() );
    ASSERT_TRUE( a2id.IsValid() );

    m_db->SaveChanges();

    if (true)
        {
        // Verify that aspects were written to the Db and that the foreign key relationships were put into place
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement(
            "SELECT aspect.ECInstanceId,aspect.TestMultiAspectProperty FROM DgnPlatformTest.TestMultiAspect aspect WHERE aspect.ElementId=?");
        stmt->BindId(1, el->GetElementId());

        bool has1=false;
        bool has2=false;
        int count=0;

        while (BE_SQLITE_ROW == stmt->Step())
            {
            ++count;
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

        ASSERT_TRUE( has1 && has2 && (2 == count) );
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
        ASSERT_EQ(BE_SQLITE_ROW , stmt->Step() );
        ASSERT_STREQ( "1" , stmt->GetValueText(0) );
        stmt->Reset();
        stmt->ClearBindings();
        stmt->BindId(1, a2id);
        ASSERT_EQ(BE_SQLITE_ROW , stmt->Step() );
        ASSERT_STREQ( "2 is Changed" , stmt->GetValueText(0) );
        }

    if (true)
        {
        // Delete an aspect
        }
    }
