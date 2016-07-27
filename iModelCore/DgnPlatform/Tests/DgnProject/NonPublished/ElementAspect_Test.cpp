/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementAspect_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <Bentley/BeTimeUtilities.h>
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
    DgnAuthorityPtr Create(Utf8CP name, bool insert = true)
    {
        DgnAuthorityPtr auth = NamespaceAuthority::CreateNamespaceAuthority(name, *m_db);
        if (insert)
        {
            EXPECT_EQ(DgnDbStatus::Success, auth->Insert());
            auto authId = auth->GetAuthorityId();
            EXPECT_TRUE(authId.IsValid());
        }

        return auth;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAspectTests, UniqueAspect_CRUD)
    {
    SetupSeedProject();
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ElementAspectTests, UniqueAspect_Uniqueness)
    {
    SetupSeedProject();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass(*m_db);
    TestElementCPtr el;

    //  Insert an element ...
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    ASSERT_EQ(nullptr, DgnElement::UniqueAspect::GetAspect(*tempEl, aclass)) << "element should not yet have an aspect";
    //  ... with an aspect
    DgnElement::UniqueAspect::SetAspect(*tempEl, *TestUniqueAspect::Create("Initial Value"));
    
    // Set Aspect again and this will replace existing one.
    DgnElement::UniqueAspect::SetAspect(*tempEl, *TestUniqueAspect::Create("Latest Value"));
    ASSERT_NE(nullptr, DgnElement::UniqueAspect::GetAspect(*tempEl, aclass)) << "element should have a scheduled aspect";
    el = m_db->Elements().Insert(*tempEl);

    ASSERT_TRUE(el.IsValid());

    //Verify the latest one is there.
    TestUniqueAspectCP aspect = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, aclass);
    ASSERT_NE(nullptr, aspect) << "element should have a peristent aspect";
    ASSERT_STREQ("Latest Value", aspect->GetTestUniqueAspectProperty().c_str());
    ASSERT_STREQ("TestUniqueAspect", aspect->GetECClassName());
    ASSERT_EQ(el->GetElementId(), aspect->GetAspectInstanceId(*el));

    BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspect WHERE (ECInstanceId=?)");
    stmt->BindId(1, aspect->GetAspectInstanceId(*el));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ("Latest Value", stmt->GetValueText(0));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAspectTests, MultiAspect_CRUD)
    {
    SetupSeedProject();
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
        TestElementPtr tempEl = el->MakeCopy<TestElement>();
        TestMultiAspectP aspect = DgnElement::MultiAspect::GetP<TestMultiAspect>(*tempEl, aclass, a2id);
        aspect->Delete();
        ASSERT_TRUE(m_db->Elements().Update(*tempEl).IsValid());
        m_db->SaveChanges();

        //Verify that it is not in the db now and other one is still there.
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestMultiAspectProperty FROM DgnPlatformTest.TestMultiAspect WHERE (ECInstanceId=?)");
        stmt->BindId(1, a1id);
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        ASSERT_STREQ("1", stmt->GetValueText(0));
        stmt->Reset();
        stmt->ClearBindings();
        stmt->BindId(1, a2id);
        ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ElementAspectTests, ExternalKeyAspect_DiffAuthority)
    {
    SetupSeedProject();

    TestElementCPtr el;
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");

    //Create some authorities
    DgnAuthorityId auth1Id = Create("Auth1")->GetAuthorityId();
    DgnAuthorityId auth2Id = Create("Auth2")->GetAuthorityId();

    //Add aspects
    DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(auth1Id, "TestExtKey");
    ASSERT_TRUE(extkeyAspect.IsValid());
    tempEl->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());

    DgnElement::ExternalKeyAspectPtr extkeyAspect2 = DgnElement::ExternalKeyAspect::Create(auth2Id, "TestExtKey2");
    ASSERT_TRUE(extkeyAspect2.IsValid());
    tempEl->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect2.get());

    //Insert Element and aspects should be added
    el = m_db->Elements().Insert(*tempEl);
    ASSERT_TRUE(el.IsValid());

    //‎Verify that both entires are there and can be get on the basis of AuthorityId. TFS 357980
    Utf8String insertedExternalKey;
    //This fails and only latest value is there
    //EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el, auth1Id));
    //EXPECT_STREQ("TestExtKey", insertedExternalKey.c_str());

    Utf8String insertedExternalKey2;
    EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Query(insertedExternalKey2, *el, auth2Id));
    EXPECT_STREQ("TestExtKey2", insertedExternalKey2.c_str());

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ElementAspectTests, ExternalKeyAspect_MultipleElements)
    {
    SetupSeedProject();

    TestElementCPtr el1, el2;
    TestElementPtr tempEl1 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");
    TestElementPtr tempEl2 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement2");

    //Create aspect
    DgnAuthorityId auth1Id = Create("Auth1")->GetAuthorityId();
    static DgnElement::AppData::Key s_appDataKey1;

    DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(auth1Id, "TestExtKey");
    ASSERT_TRUE(extkeyAspect.IsValid());

    //Add aspect to both elements
    tempEl1->AddAppData(s_appDataKey1, extkeyAspect.get());
    tempEl2->AddAppData(s_appDataKey1, extkeyAspect.get());

    //Insert Elements and aspects should be added
    el1 = m_db->Elements().Insert(*tempEl1);
    ASSERT_TRUE(el1.IsValid());
    el2 = m_db->Elements().Insert(*tempEl2);
    ASSERT_TRUE(el2.IsValid());


    //‎Verify that both elements have the aspect
    Utf8String insertedExternalKey;
    EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el1, auth1Id));
    EXPECT_STREQ("TestExtKey", insertedExternalKey.c_str());

    EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el2, auth1Id));
    EXPECT_STREQ("TestExtKey", insertedExternalKey.c_str());

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ElementAspectTests, ExternalKeyAspect_WrongAuthorityId)
    {
    SetupSeedProject();

    TestElementCPtr el;
    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");

    //Create an aspect with Authority that is not in DgnAuthorities
    DgnAuthorityId authId((uint64_t)1000);
    DgnAuthorityCPtr invalidAuth = m_db->Authorities().GetAuthority(authId);
    ASSERT_TRUE(invalidAuth.IsNull());

    //Authority doesn't exist but it lets add it. This shouldn't happen. Reported TFS 358209
    DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(authId, "TestExtKey");
    ASSERT_TRUE(extkeyAspect.IsValid());

    //Add aspect to element and insert it.
    tempEl->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
    el = m_db->Elements().Insert(*tempEl);
    ASSERT_TRUE(el.IsValid());

    //It can be accessed also
    Utf8String insertedExternalKey;
    EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el, authId));
    EXPECT_STREQ("TestExtKey", insertedExternalKey.c_str());

    }
//---------------------------------------------------------------------------------------
// @bsimethod                               Umar.Hayat                            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ElementAspectTests, ExternalKeyAspect_Delete)
    {
    SetupSeedProject();

    TestElementCPtr el1;
    TestElementPtr tempEl1 = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");

    //Create aspect
    DgnAuthorityId auth1Id = Create("Auth1")->GetAuthorityId();
    DgnAuthorityId auth2Id = Create("Auth2")->GetAuthorityId();
    static DgnElement::AppData::Key s_appDataKey1;
    static DgnElement::AppData::Key s_appDataKey2;

    DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(auth1Id, "TestExtKey");
    ASSERT_TRUE(extkeyAspect.IsValid());
    DgnElement::ExternalKeyAspectPtr extkeyAspect2 = DgnElement::ExternalKeyAspect::Create(auth2Id, "TestExtKey2");
    ASSERT_TRUE(extkeyAspect2.IsValid());
    
    //Add aspect 
    tempEl1->AddAppData(s_appDataKey1, extkeyAspect.get());
    tempEl1->AddAppData(s_appDataKey2, extkeyAspect2.get());

    //Insert Elements and aspects should be added
    el1 = m_db->Elements().Insert(*tempEl1);
    ASSERT_TRUE(el1.IsValid());

    //‎Verify that elements has the aspect
    Utf8String insertedExternalKey;
    EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el1, auth1Id));
    EXPECT_STREQ("TestExtKey", insertedExternalKey.c_str());
    EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el1, auth2Id));
    EXPECT_STREQ("TestExtKey2", insertedExternalKey.c_str());

    EXPECT_EQ(DgnDbStatus::Success, DgnElement::ExternalKeyAspect::Delete(*el1, auth1Id));
    // Verify 
    EXPECT_TRUE(DgnDbStatus::Success != DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el1, auth1Id));
    EXPECT_TRUE(DgnDbStatus::Success == DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el1, auth2Id));

    }
//---------------------------------------------------------------------------------------
// @bsimethod                               Umar.Hayat                            03/2016
//---------------------------------------------------------------------------------------
TEST_F(ElementAspectTests, ImportElementsWithAspect)
    {
    // Open Source Db
    SetupSeedProject();
    DgnAuthorityId auth1Id;
    TestElementCPtr el1;
    if (true)
        {
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");

        //Create aspect
        auth1Id = Create("Auth1")->GetAuthorityId();
        static DgnElement::AppData::Key s_appDataKey1;

        DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(auth1Id, "TestExtKey");
        ASSERT_TRUE(extkeyAspect.IsValid());

        //Add aspect 
        tempEl->AddAppData(s_appDataKey1, extkeyAspect.get());
        tempEl->SetCode(DgnCode(auth1Id, "TestCode", ""));
        DgnElement::UniqueAspect::SetAspect(*tempEl, *TestUniqueAspect::Create("Initial Value"));

        //Insert Elements and aspects should be added
        el1 = m_db->Elements().Insert(*tempEl);
        ASSERT_TRUE(el1.IsValid());

        m_db->SaveChanges();
        }
    //  *******************************
    //  Import element having aspect into separate db
    if (true)
        {        
        // Open Destination DB
        BeFileName destinationFile;
        ASSERT_TRUE(DgnDbStatus::Success == DgnDbTestFixture::GetSeedDbCopy(destinationFile, L"DestinationDb.bim"));
        DgnDbPtr db2;
        DgnDbTestFixture::OpenDb(db2, destinationFile, DgnDb::OpenMode::ReadWrite, true);
        ASSERT_TRUE(db2.IsValid());
        auto status = DgnPlatformTestDomain::GetDomain().ImportSchema(*db2);
        ASSERT_TRUE(DgnDbStatus::Success == status);

        DgnImportContext importContext(*m_db, *db2);
        DgnDbStatus stat;
        DgnAuthority::Import(&stat, *m_db->Authorities().GetAuthority(auth1Id), importContext);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_EQ(DbResult::BE_SQLITE_OK , db2->SaveChanges());

        DgnElementCPtr el1Dest = el1->Import(&stat, *db2->Models().GetModel(db2->Models().QueryFirstModelId()), importContext);
        ASSERT_TRUE(el1Dest.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_EQ(DbResult::BE_SQLITE_OK, db2->SaveChanges());

        DgnElementCPtr el = db2->Elements().Get<DgnElement>(el1Dest->GetElementId());
        ASSERT_TRUE(el.IsValid());

        // Verify that Authority was copied over
        DgnAuthorityId authIdb = db2->Authorities().QueryAuthorityId(el->GetCodeAuthority()->GetName().c_str());
        ASSERT_TRUE(authIdb.IsValid());
        
        Utf8String insertedExternalKey;
        EXPECT_TRUE(DgnDbStatus::Success == DgnElement::ExternalKeyAspect::Query(insertedExternalKey, *el, authIdb));
        EXPECT_TRUE(insertedExternalKey.Equals("TestExtKey"));

        ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass(*m_db);
        TestUniqueAspectCP aspect = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, aclass);
        EXPECT_TRUE(nullptr != aspect) << "element should have a peristent aspect in destination db";
        if (aspect)
            EXPECT_TRUE(aspect->GetTestUniqueAspectProperty().Equals("Initial Value"));
        }
    }
