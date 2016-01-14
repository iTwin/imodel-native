/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementAspect_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <Bentley/BeTimeUtilities.h>
#include <ECDb/ECSqlBuilder.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/WebMercator.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      06/15
+===============+===============+===============+===============+===============+======*/
struct ElementAspectTests : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAspectTests, UniqueAspect_CRUD)
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
TEST_F(ElementAspectTests, MultiAspect_CRUD)
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
