/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ElementAspect_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    CodeSpecPtr Create(Utf8CP name, bool insert = true)
    {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_db, name);
        if (insert)
        {
            EXPECT_EQ(DgnDbStatus::Success, codeSpec->Insert());
            auto codeSpecId = codeSpec->GetCodeSpecId();
            EXPECT_TRUE(codeSpecId.IsValid());
        }

        return codeSpec;
    }

    void SetUniqueAspectPropertyValue(DgnElementR, ECN::ECClassCR, Utf8CP propertyName, Utf8CP propertyValue);
    void SetMultiAspectPropertyValue(DgnElementR, ECN::ECClassCR, ECInstanceId, Utf8CP propertyName, Utf8CP propertyValue);

    void AssertUniqueAspectPropertyValue(Utf8CP expectedValue, DgnElementCR, ECN::ECClassCR, Utf8CP propertyName);
    void AssertMultiAspectPropertyValue(Utf8CP expectedValue, DgnElementCR, ECN::ECClassCR, ECInstanceId, Utf8CP propertyName);
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

        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspect WHERE (Element.Id=?)");
        stmt->BindId(1, el->GetElementId());
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
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspect WHERE (Element.Id=?)");
        stmt->BindId(1, el->GetElementId());
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

    BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspect WHERE (Element.Id=?)");
    stmt->BindId(1, el->GetElementId());
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
            "SELECT aspect.ECInstanceId,aspect.TestMultiAspectProperty FROM DgnPlatformTest.TestMultiAspect aspect WHERE aspect.Element.Id=?");
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
        TestMultiAspectCP aspectPersist = DgnElement::MultiAspect::Get<TestMultiAspect>(*el, aclass, a2id); // make sure that getting a read-only copy of the aspect from the original does not interfere with the update
        ASSERT_TRUE(aspectPersist != nullptr);
        ASSERT_EQ( aspectPersist , DgnElement::MultiAspect::Get<TestMultiAspect>(*el, aclass, a2id) ) << "Get should return the same instance each time we call it";
        ASSERT_EQ( aspectPersist , DgnElement::MultiAspect::Get<TestMultiAspect>(*el, aclass, a2id) ) << "Get should return the same instance each time we call it";
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
        TestMultiAspectCP a1 = DgnElement::MultiAspect::Get<TestMultiAspect>(*el, aclass, a2id);
        ASSERT_TRUE(a1 != nullptr);
        a1->GetTestMultiAspectProperty().Equals("1");
        stmt->Reset();
        stmt->ClearBindings();
        stmt->BindId(1, a2id);
        ASSERT_EQ(BE_SQLITE_ROW , stmt->Step() );
        ASSERT_STREQ( "2 is Changed" , stmt->GetValueText(0) );
        TestMultiAspectCP a2 = DgnElement::MultiAspect::Get<TestMultiAspect>(*el, aclass, a2id);
        ASSERT_TRUE(a2 != nullptr);
        a2->GetTestMultiAspectProperty().Equals("2 is Changed");
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
// @bsimethod                               Umar.Hayat                            03/2016
//---------------------------------------------------------------------------------------
TEST_F(ElementAspectTests, ImportElementsWithAspect)
    {
    // Open Source Db
    SetupSeedProject();
    CodeSpecId codeSpec1Id;
    TestElementCPtr el1;
    if (true)
        {
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");

        //Create aspect
        CodeSpecPtr codeSpec1 = Create("CodeSpec1");
        ASSERT_TRUE(codeSpec1.IsValid());
        codeSpec1Id = codeSpec1->GetCodeSpecId();

        //Add aspect 
        tempEl->SetCode(codeSpec1->CreateCode("TestCode"));
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
        ASSERT_TRUE(SchemaStatus::Success == status);

        DgnImportContext importContext(*m_db, *db2);
        DgnDbStatus stat;
        CodeSpec::Import(&stat, *m_db->CodeSpecs().GetCodeSpec(codeSpec1Id), importContext);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_EQ(DbResult::BE_SQLITE_OK , db2->SaveChanges());

        DgnElementCPtr el1Dest = el1->Import(&stat, *db2->Models().GetModel(DgnDbTestUtils::QueryFirstGeometricModelId(*db2)), importContext);
        ASSERT_TRUE(el1Dest.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_EQ(DbResult::BE_SQLITE_OK, db2->SaveChanges());

        DgnElementCPtr el = db2->Elements().Get<DgnElement>(el1Dest->GetElementId());
        ASSERT_TRUE(el.IsValid());

        // Verify that CodeSpec was copied over
        CodeSpecId codeSpecIdb = db2->CodeSpecs().QueryCodeSpecId(el->GetCodeSpec()->GetName().c_str());
        ASSERT_TRUE(codeSpecIdb.IsValid());
        
        ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass(*m_db);
        TestUniqueAspectCP aspect = DgnElement::UniqueAspect::Get<TestUniqueAspect>(*el, aclass);
        EXPECT_TRUE(nullptr != aspect) << "element should have a peristent aspect in destination db";
        if (aspect)
            EXPECT_TRUE(aspect->GetTestUniqueAspectProperty().Equals("Initial Value"));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAspectTests, GenericAspect_CRUD)
    {
    // Open Source Db
    SetupSeedProject();

    auto maspectclassNoHandler = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, "TestMultiAspectNoHandler");
    ASSERT_TRUE(nullptr != maspectclassNoHandler);

    auto maspectclassWithHandler = TestMultiAspect::GetECClass(*m_db);
    ASSERT_TRUE(nullptr != maspectclassWithHandler);

    DgnElementCPtr persistEl;
    if (true)
        {
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");

        auto maspect1 = maspectclassNoHandler->GetDefaultStandaloneEnabler()->CreateInstance();
        maspect1->SetValue("TestMultiAspectProperty", ECN::ECValue("foo"));

        ASSERT_EQ(DgnDbStatus::Success, DgnElement::GenericMultiAspect::AddAspect(*tempEl, *maspect1));

        // Check that we can find the aspect that we just added to the non-persistent element
        auto foundProps = DgnElement::GenericMultiAspect::GetAspectP(*tempEl, *maspectclassNoHandler, BeSQLite::EC::ECInstanceId());
        ASSERT_TRUE(foundProps != nullptr);
        ECN::ECValue value;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, foundProps->GetValue(value, "TestMultiAspectProperty"));
        ASSERT_STREQ("foo", value.ToString().c_str());

        // Check that we can find the same aspect again. Aspects are cached in memory, so the second request may be handled differently from the first.
        foundProps = DgnElement::GenericMultiAspect::GetAspectP(*tempEl, *maspectclassNoHandler, BeSQLite::EC::ECInstanceId());
        ASSERT_TRUE(foundProps != nullptr);
        ASSERT_EQ(ECN::ECObjectsStatus::Success, foundProps->GetValue(value, "TestMultiAspectProperty"));
        ASSERT_STREQ("foo", value.ToString().c_str());

        persistEl = tempEl->Insert();
        }

    ASSERT_TRUE(persistEl.IsValid());

    if (true)
        {
        // Check that we can find the same aspect from the persistent element
        auto editEl = persistEl->CopyForEdit();

        auto foundProps = DgnElement::GenericMultiAspect::GetAspectP(*editEl, *maspectclassNoHandler, BeSQLite::EC::ECInstanceId());
        ASSERT_TRUE(foundProps != nullptr);
        ECN::ECValue value;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, foundProps->GetValue(value, "TestMultiAspectProperty"));
        ASSERT_STREQ("foo", value.ToString().c_str());

        // Check that we can find the same aspect again. Aspects are cached in memory, so the second request may be handled differently from the first.
        foundProps = DgnElement::GenericMultiAspect::GetAspectP(*editEl, *maspectclassNoHandler, BeSQLite::EC::ECInstanceId());
        ASSERT_TRUE(foundProps != nullptr);
        ASSERT_EQ(ECN::ECObjectsStatus::Success, foundProps->GetValue(value, "TestMultiAspectProperty"));
        ASSERT_STREQ("foo", value.ToString().c_str());

        // Add another instance of the same multi aspect -- should now have 2
        auto maspect1 = maspectclassNoHandler->GetDefaultStandaloneEnabler()->CreateInstance();
        maspect1->SetValue("TestMultiAspectProperty", ECN::ECValue("bar"));
        ASSERT_EQ(DgnDbStatus::Success, DgnElement::GenericMultiAspect::AddAspect(*editEl, *maspect1));
        ASSERT_TRUE(editEl->Update().IsValid());

        ECSqlStatement findMa;
        findMa.Prepare(*m_db, "SELECT ECInstanceId, TestMultiAspectProperty FROM DgnPlatformTest.TestMultiAspectNoHandler WHERE Element.Id=?");    
        findMa.BindId(1, editEl->GetElementId());
        bvector<Utf8String> values;
        BeSQLite::EC::ECInstanceId id;
        while (BE_SQLITE_ROW == findMa.Step())
            {
            auto value = findMa.GetValueText(1);
            auto aspectid = findMa.GetValueId<BeSQLite::EC::ECInstanceId>(0);
            values.push_back(value);
            if (0==strcmp("bar", value))
                id = aspectid;
            }
            
        ASSERT_EQ(2, values.size());
        ASSERT_TRUE(id.IsValid());
        ASSERT_TRUE(values.end() != std::find(values.begin(), values.end(), "foo"));
        ASSERT_TRUE(values.end() != std::find(values.begin(), values.end(), "bar"));

        //  Modify a property on one of the aspects
        maspect1->SetValue("TestMultiAspectProperty", ECN::ECValue("bar-changed"));
        ASSERT_EQ(DgnDbStatus::Success, DgnElement::GenericMultiAspect::SetAspect(*editEl, *maspect1, id));
        ASSERT_TRUE(editEl->Update().IsValid());

        //      ... we should still only have 2, but one should have a modified value
        findMa.Reset();
        values.clear();
        while (BE_SQLITE_ROW == findMa.Step())
            {
            auto value = findMa.GetValueText(1);
            auto aspectid = findMa.GetValueId<BeSQLite::EC::ECInstanceId>(0);
            values.push_back(value);
            if (0==strcmp("bar-changed", value))
                ASSERT_EQ(id, aspectid);
            else
                ASSERT_NE(id, aspectid);
            }
            
        ASSERT_EQ(2, values.size());
        ASSERT_TRUE(id.IsValid());
        ASSERT_TRUE(values.end() != std::find(values.begin(), values.end(), "foo"));
        ASSERT_TRUE(values.end() != std::find(values.begin(), values.end(), "bar-changed"));

        //  Add an instance of a different multiaspect class
        TestMultiAspect::AddAspect(*editEl, *TestMultiAspect::Create("has handler"));
        ASSERT_TRUE(editEl->Update().IsValid());

        //      ... we should still only have 2 instances of the first MA, 
        findMa.Reset();
        values.clear();
        while (BE_SQLITE_ROW == findMa.Step())
            {
            auto value = findMa.GetValueText(1);
            auto aspectid = findMa.GetValueId<BeSQLite::EC::ECInstanceId>(0);
            values.push_back(value);
            if (0==strcmp("bar-changed", value))
                ASSERT_EQ(id, aspectid);
            else
                ASSERT_NE(id, aspectid);
            }
            
        ASSERT_EQ(2, values.size());
        ASSERT_TRUE(id.IsValid());
        ASSERT_TRUE(values.end() != std::find(values.begin(), values.end(), "foo"));
        ASSERT_TRUE(values.end() != std::find(values.begin(), values.end(), "bar-changed"));

        //      ... plus one instance of the second MA
        ECSqlStatement selectTmas;
        selectTmas.Prepare(*m_db, "SELECT ECInstanceId, TestMultiAspectProperty TestMultiAspectProperty FROM DgnPlatformTest.TestMultiAspect WHERE Element.Id=?");
        selectTmas.BindId(1, editEl->GetElementId());
        int countTmas = 0;
        while (BE_SQLITE_ROW == selectTmas.Step())
            {
            auto aspectid = selectTmas.GetValueId<BeSQLite::EC::ECInstanceId>(0);
            auto value = selectTmas.GetValueText(1);

            ASSERT_EQ(0, countTmas);
            ++countTmas;
            ASSERT_STREQ("has handler", value);
            }

        ASSERT_EQ(1, countTmas);
        }

    if (true)
        {
        // If an aspect is supposed to have a handler, then we are not allowed to write such aspects using the generic handler
        auto editEl = persistEl->CopyForEdit();
        ASSERT_TRUE(nullptr != maspectclassWithHandler);
        auto maspectWithHandler1 = maspectclassWithHandler->GetDefaultStandaloneEnabler()->CreateInstance();
        maspectWithHandler1->SetValue("TestMultiAspectProperty", ECN::ECValue("foo"));  
        ASSERT_NE(DgnDbStatus::Success, DgnElement::GenericMultiAspect::AddAspect(*editEl, *maspectWithHandler1));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAspectTests, PolymorphicUniqueAspects)
    {
    SetupSeedProject();
    m_db->Schemas().CreateClassViewsInDb();
    ECN::ECClassCP baseClass = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, "TestUniqueAspectNoHandler");
    ECN::ECClassCP d1Class = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, "TestUniqueAspectNoHandlerD1");
    ECN::ECClassCP d2Class = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, "TestUniqueAspectNoHandlerD2");
    ASSERT_TRUE(nullptr != baseClass);
    ASSERT_TRUE(nullptr != d1Class);
    ASSERT_TRUE(nullptr != d2Class);

    TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");
    tempEl = tempEl->Insert()->MakeCopy<TestElement>();

    if (true)
        {
        // Put an instance of TestUniqueAspectNoHandlerD1 on the element.
        auto d1 = d1Class->GetDefaultStandaloneEnabler()->CreateInstance();
        const Utf8CP d1PropertyValue = "D1 property value 1";
        d1->SetValue("D1", ECN::ECValue(d1PropertyValue));

        ASSERT_EQ(nullptr, DgnElement::GenericUniqueAspect::GetAspect(*tempEl, *baseClass));
        ASSERT_EQ(DgnDbStatus::Success, DgnElement::GenericUniqueAspect::SetAspect(*tempEl, *d1, baseClass));

        auto d1Found = DgnElement::GenericUniqueAspect::GetAspect(*tempEl, *baseClass); // get it while it's cached in memory
        ASSERT_TRUE(d1Found != nullptr);
        ASSERT_STREQ(d1Found->GetClass().GetFullName(), d1->GetClass().GetFullName());
        ECN::ECValue value;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, d1Found->GetValue(value, "D1"));
        ASSERT_STREQ(value.GetUtf8CP(), d1PropertyValue);

        tempEl = tempEl->Update()->MakeCopy<TestElement>();  // clears the aspect cache
        d1Found = DgnElement::GenericUniqueAspect::GetAspect(*tempEl, *baseClass); // get the aspect from the db
        ASSERT_TRUE(d1Found != nullptr);
        ASSERT_STREQ(d1Found->GetClass().GetFullName(), d1->GetClass().GetFullName());
        ASSERT_EQ(ECN::ECObjectsStatus::Success, d1Found->GetValue(value, "D1"));
        ASSERT_STREQ(value.GetUtf8CP(), d1PropertyValue);
        }

    if (true)
        {
        // Replace the d1 aspect with a d2 aspect
        auto d2 = d2Class->GetDefaultStandaloneEnabler()->CreateInstance();
        const Utf8CP d2PropertyValue = "D2 property value 2";
        d2->SetValue("D2", ECN::ECValue(d2PropertyValue));

        ASSERT_EQ(DgnDbStatus::Success, DgnElement::GenericUniqueAspect::SetAspect(*tempEl, *d2, baseClass));
        auto d2Found = DgnElement::GenericUniqueAspect::GetAspect(*tempEl, *baseClass); // get it while it's cached in memory
        ASSERT_TRUE(d2Found != nullptr);
        ASSERT_STREQ(d2Found->GetClass().GetFullName(), d2->GetClass().GetFullName());
        ECN::ECValue value;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, d2Found->GetValue(value, "D2")); 
        ASSERT_STREQ(value.GetUtf8CP(), d2PropertyValue);
        
        tempEl = tempEl->Update()->MakeCopy<TestElement>();  // clears the aspect cache
        d2Found = DgnElement::GenericUniqueAspect::GetAspect(*tempEl, *baseClass); // get the aspect from the db
        ASSERT_TRUE(d2Found != nullptr);
        ASSERT_STREQ(d2Found->GetClass().GetFullName(), d2->GetClass().GetFullName());
        ASSERT_EQ(ECN::ECObjectsStatus::Success, d2Found->GetValue(value, "D2")); 
        ASSERT_STREQ(value.GetUtf8CP(), d2PropertyValue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ridha.Malik      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAspectTests, GenericUniqueAspect_CRUD)
    {
    // Open Source Db
    SetupSeedProject();
    m_db->Schemas().CreateClassViewsInDb();
    ECN::ECClassCP uaspectclassNoHandler = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, "TestUniqueAspectNoHandler");
    ASSERT_TRUE(nullptr != uaspectclassNoHandler);

    auto uaspectclassWithHandler = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_TRUE(nullptr != uaspectclassWithHandler);
    BeFileName fileName = m_db->GetFileName();
    DgnElementId elementId;
    if (true)
    {
        TestElementPtr tempEl = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement1");

        auto uaspect = uaspectclassNoHandler->GetDefaultStandaloneEnabler()->CreateInstance();
        uaspect->SetValue("TestUniqueAspectProperty", ECN::ECValue("foo"));

        ASSERT_EQ(nullptr, DgnElement::GenericUniqueAspect::GetAspect(*tempEl, *uaspectclassNoHandler));

        ASSERT_EQ(DgnDbStatus::Success, DgnElement::GenericUniqueAspect::SetAspect(*tempEl, *uaspect));
        // Check that we can find the aspect that we just added to the non-persistent element
        auto foundProps = DgnElement::GenericUniqueAspect::GetAspectP(*tempEl, *uaspectclassNoHandler);
        ASSERT_TRUE(foundProps != nullptr);
        ECN::ECValue value;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, foundProps->GetValue(value, "TestUniqueAspectProperty"));
        ASSERT_STREQ("foo", value.ToString().c_str());

        auto el = tempEl->Insert();
        ASSERT_TRUE(el.IsValid());
        elementId = el->GetElementId();
        m_db->SaveChanges();
    }

    ASSERT_TRUE(elementId.IsValid());

    if (true)
    {
        // Check that we can find the same aspect from the persistent element 
        auto editEl = m_db->Elements().GetForEdit<DgnElement>(elementId);
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspectNoHandler WHERE Element.Id=?");
        stmt->BindId(1, editEl->GetElementId());
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        ASSERT_STREQ("foo", stmt->GetValueText(0));

        //update
        auto foundProps = DgnElement::GenericUniqueAspect::GetAspectP(*editEl, *uaspectclassNoHandler);
        ASSERT_TRUE(foundProps != nullptr);
        //  Modify a property on one of the aspects
        ASSERT_EQ(ECN::ECObjectsStatus::Success, foundProps->SetValue("TestUniqueAspectProperty", ECN::ECValue("foo-changed")));
        ASSERT_EQ(DgnDbStatus::Success, DgnElement::GenericUniqueAspect::SetAspect(*editEl, *foundProps));
        foundProps = DgnElement::GenericUniqueAspect::GetAspectP(*editEl, *uaspectclassNoHandler);
        ASSERT_TRUE(foundProps != nullptr);
        ECN::ECValue value;
        ASSERT_EQ(ECN::ECObjectsStatus::Success, foundProps->GetValue(value, "TestUniqueAspectProperty"));
        ASSERT_STREQ("foo-changed", value.ToString().c_str());
        ASSERT_TRUE(editEl->Update().IsValid());
        m_db->SaveChanges();
    }

    m_db->CloseDb();

    if (true)
    {
        m_db = nullptr;
        OpenDb(m_db,fileName, Db::OpenMode::ReadWrite, true);
        auto el = m_db->Elements().GetForEdit<DgnElement>(elementId);
        BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db->GetPreparedECSqlStatement("SELECT TestUniqueAspectProperty FROM DgnPlatformTest.TestUniqueAspectNoHandler WHERE Element.Id=?");
        stmt->BindId(1, el->GetElementId());
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
        ASSERT_STREQ("foo-changed", stmt->GetValueText(0));
    }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAspectTests::SetUniqueAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, Utf8CP propertyName, Utf8CP propertyValue)
    {
    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(element, aspectClass);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAspectTests::SetMultiAspectPropertyValue(DgnElementR element, ECN::ECClassCR aspectClass, ECInstanceId aspectInstanceId, Utf8CP propertyName, Utf8CP propertyValue)
    {
    DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(element, aspectClass, aspectInstanceId);
    ASSERT_NE(aspect, nullptr);
    ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(propertyName, ECN::ECValue(propertyValue)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAspectTests::AssertUniqueAspectPropertyValue(Utf8CP expectedValue, DgnElementCR element, ECN::ECClassCR aspectClass, Utf8CP propertyName)
    {
    DgnElement::UniqueAspect const* aspect = DgnElement::UniqueAspect::GetAspect(element, aspectClass);
    ASSERT_NE(aspect, nullptr);
    ECN::ECValue actualValue;
    ASSERT_EQ(DgnDbStatus::Success, aspect->GetPropertyValue(actualValue, propertyName));
    ASSERT_STREQ(expectedValue, actualValue.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAspectTests::AssertMultiAspectPropertyValue(Utf8CP expectedValue, DgnElementCR element, ECN::ECClassCR aspectClass, ECInstanceId aspectInstanceId, Utf8CP propertyName)
    {
    ASSERT_TRUE(aspectInstanceId.IsValid());
    DgnElement::MultiAspect const* aspect = DgnElement::MultiAspect::GetAspect(element, aspectClass, aspectInstanceId);
    ASSERT_NE(aspect, nullptr);
    ECN::ECValue actualValue;
    ASSERT_EQ(DgnDbStatus::Success, aspect->GetPropertyValue(actualValue, propertyName));
    ASSERT_STREQ(expectedValue, actualValue.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementAspectTests, PresentationRuleScenarios)
    {
    USING_NAMESPACE_BENTLEY_EC;
    SetupSeedProject();
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateClassViewsInDb());
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "MySpatialCategory");
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "MyPhysicalModel");

    TestElementPtr element = TestElement::Create(*m_db, physicalModel->GetModelId(), categoryId);
    ASSERT_TRUE(element.IsValid());

    ECClassCP aspectClassUniqueNoHandler = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_CLASS_TestUniqueAspectNoHandler);
    ASSERT_NE(aspectClassUniqueNoHandler, nullptr);

    ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassUnique, nullptr);

    ECClassCP aspectClassMultiNoHandler = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_CLASS_TestMultiAspectNoHandler);
    ASSERT_NE(aspectClassMultiNoHandler, nullptr);

    ECClassCP aspectClassMulti = TestMultiAspect::GetECClass(*m_db);
    ASSERT_NE(aspectClassMulti, nullptr);

    // add instance of DgnPlatformTest.TestUniqueAspectNoHandler
        {
        RefCountedPtr<DgnElement::UniqueAspect> aspect = DgnElement::UniqueAspect::CreateAspect(*m_db, *aspectClassUniqueNoHandler);
        ASSERT_TRUE(aspect.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty, ECValue("Aspect1")));
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_UNIQUE_ASPECT_LengthProperty, ECValue(1.0)));
        DgnElement::UniqueAspect::SetAspect(*element, *aspect);
        }

    // add instance of DgnPlatformTest.TestUniqueAspect
        {
        RefCountedPtr<DgnElement::UniqueAspect> aspect = DgnElement::UniqueAspect::CreateAspect(*m_db, *aspectClassUnique);
        ASSERT_TRUE(aspect.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty, ECValue("Aspect2")));
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_UNIQUE_ASPECT_LengthProperty, ECValue(2.0)));
        DgnElement::UniqueAspect::SetAspect(*element, *aspect);
        }

    // add first instance of DgnPlatformTest.TestMultiAspectNoHandler
        {
        RefCountedPtr<DgnElement::MultiAspect> aspect = DgnElement::MultiAspect::CreateAspect(*m_db, *aspectClassMultiNoHandler);
        ASSERT_TRUE(aspect.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, ECValue("Aspect3")));
        DgnElement::MultiAspect::AddAspect(*element, *aspect);
        }

    // add second instance of DgnPlatformTest.TestMultiAspectNoHandler
        {
        RefCountedPtr<DgnElement::MultiAspect> aspect = DgnElement::MultiAspect::CreateAspect(*m_db, *aspectClassMultiNoHandler);
        ASSERT_TRUE(aspect.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, ECValue("Aspect4")));
        DgnElement::MultiAspect::AddAspect(*element, *aspect);
        }

    // add first instance of DgnPlatformTest.TestMultiAspect
        {
        RefCountedPtr<DgnElement::MultiAspect> aspect = DgnElement::MultiAspect::CreateAspect(*m_db, *aspectClassMulti);
        ASSERT_TRUE(aspect.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, ECValue("Aspect5")));
        DgnElement::MultiAspect::AddAspect(*element, *aspect);
        }

    // add second instance of DgnPlatformTest.TestMultiAspect
        {
        RefCountedPtr<DgnElement::MultiAspect> aspect = DgnElement::MultiAspect::CreateAspect(*m_db, *aspectClassMulti);
        ASSERT_TRUE(aspect.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, aspect->SetPropertyValue(DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, ECValue("Aspect6")));
        DgnElement::MultiAspect::AddAspect(*element, *aspect);
        }

    // insert element (and aspects) and verify counts
    ASSERT_TRUE(element->Insert().IsValid());
    ASSERT_EQ(1, DgnDbTestUtils::SelectCountFromECClass(*m_db, DPTEST_SCHEMA(DPTEST_CLASS_TestUniqueAspectNoHandler)));
    ASSERT_EQ(1, DgnDbTestUtils::SelectCountFromECClass(*m_db, DPTEST_SCHEMA(DPTEST_TEST_UNIQUE_ASPECT_CLASS_NAME)));
    ASSERT_EQ(2, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect)));
    ASSERT_EQ(2, DgnDbTestUtils::SelectCountFromECClass(*m_db, DPTEST_SCHEMA(DPTEST_CLASS_TestMultiAspectNoHandler)));
    ASSERT_EQ(2, DgnDbTestUtils::SelectCountFromECClass(*m_db, DPTEST_SCHEMA(DPTEST_TEST_MULTI_ASPECT_CLASS_NAME)));
    ASSERT_EQ(4, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)));
    ASSERT_EQ(6, DgnDbTestUtils::SelectCountFromECClass(*m_db, BIS_SCHEMA(BIS_CLASS_ElementAspect)));

    // cause element to be reloaded
    DgnElementId elementId = element->GetElementId();
    element = nullptr;
    m_db->Elements().ClearCache();
    element = m_db->Elements().GetForEdit<TestElement>(elementId);
    ASSERT_TRUE(element.IsValid());

    ECInstanceId aspectInstanceId3, aspectInstanceId4, aspectInstanceId5, aspectInstanceId6;

    // Query for ECInstanceIds of DgnPlatformTest.TestMultiAspectNoHandler aspects (only set after Insert)
        {
        CachedECSqlStatementPtr statement = m_db->GetPreparedECSqlStatement("SELECT ECInstanceId FROM " DPTEST_SCHEMA(DPTEST_CLASS_TestMultiAspectNoHandler) " WHERE " DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "=?");
        statement->BindText(1, "Aspect3", IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, statement->Step());
        aspectInstanceId3 = statement->GetValueId<ECInstanceId>(0);
        ASSERT_TRUE(aspectInstanceId3.IsValid());

        statement->Reset();
        statement->BindText(1, "Aspect4", IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, statement->Step());
        aspectInstanceId4 = statement->GetValueId<ECInstanceId>(0);
        ASSERT_TRUE(aspectInstanceId4.IsValid());
        }

    // Query for ECInstanceIds of DgnPlatformTest.TestMultiAspect aspects (only set after Insert)
        {
        CachedECSqlStatementPtr statement = m_db->GetPreparedECSqlStatement("SELECT ECInstanceId FROM " DPTEST_SCHEMA(DPTEST_TEST_MULTI_ASPECT_CLASS_NAME) " WHERE " DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty "=?");
        statement->BindText(1, "Aspect5", IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, statement->Step());
        aspectInstanceId5 = statement->GetValueId<ECInstanceId>(0);
        ASSERT_TRUE(aspectInstanceId5.IsValid());

        statement->Reset();
        statement->BindText(1, "Aspect6", IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_ROW, statement->Step());
        aspectInstanceId6 = statement->GetValueId<ECInstanceId>(0);
        ASSERT_TRUE(aspectInstanceId6.IsValid());
        }

    // verify inserted instances
    AssertUniqueAspectPropertyValue("Aspect1", *element, *aspectClassUniqueNoHandler, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty);
    AssertUniqueAspectPropertyValue("Aspect2", *element, *aspectClassUnique, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty);
    AssertMultiAspectPropertyValue("Aspect3", *element, *aspectClassMultiNoHandler, aspectInstanceId3, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);
    AssertMultiAspectPropertyValue("Aspect4", *element, *aspectClassMultiNoHandler, aspectInstanceId4, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);
    AssertMultiAspectPropertyValue("Aspect5", *element, *aspectClassMulti, aspectInstanceId5, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);
    AssertMultiAspectPropertyValue("Aspect6", *element, *aspectClassMulti, aspectInstanceId6, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);

    // update property values
    SetUniqueAspectPropertyValue(*element, *aspectClassUniqueNoHandler, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty, "Aspect1-Updated");
    SetUniqueAspectPropertyValue(*element, *aspectClassUnique, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty, "Aspect2-Updated");
    SetMultiAspectPropertyValue(*element, *aspectClassMultiNoHandler, aspectInstanceId3, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, "Aspect3-Updated");
    SetMultiAspectPropertyValue(*element, *aspectClassMultiNoHandler, aspectInstanceId4, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, "Aspect4-Updated");
    SetMultiAspectPropertyValue(*element, *aspectClassMulti, aspectInstanceId5, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, "Aspect5-Updated");
    SetMultiAspectPropertyValue(*element, *aspectClassMulti, aspectInstanceId6, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty, "Aspect6-Updated");

    // cause aspects to be updated
    ASSERT_TRUE(element->Update().IsValid());

    // cause element to be reloaded
    element = nullptr;
    m_db->Elements().ClearCache();
    element = m_db->Elements().GetForEdit<TestElement>(elementId);
    ASSERT_TRUE(element.IsValid());

    // verify updated instances
    AssertUniqueAspectPropertyValue("Aspect1-Updated", *element, *aspectClassUniqueNoHandler, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty);
    AssertUniqueAspectPropertyValue("Aspect2-Updated", *element, *aspectClassUnique, DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty);
    AssertMultiAspectPropertyValue("Aspect3-Updated", *element, *aspectClassMultiNoHandler, aspectInstanceId3, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);
    AssertMultiAspectPropertyValue("Aspect4-Updated", *element, *aspectClassMultiNoHandler, aspectInstanceId4, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);
    AssertMultiAspectPropertyValue("Aspect5-Updated", *element, *aspectClassMulti, aspectInstanceId5, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);
    AssertMultiAspectPropertyValue("Aspect6-Updated", *element, *aspectClassMulti, aspectInstanceId6, DPTEST_TEST_MULTI_ASPECT_TestMultiAspectProperty);

    // Test iteration of DgnPlatformTest.TestUniqueAspectNoHandler
        {
        int aspectCount = 0;
        ElementAspectIterator aspectIterator = m_db->Elements().MakeAspectIterator(DPTEST_SCHEMA(DPTEST_CLASS_TestUniqueAspectNoHandler));
        for (ElementAspectIteratorEntryCR aspectEntry : aspectIterator)
            {
            ASSERT_EQ(aspectEntry.GetClassId(), aspectClassUniqueNoHandler->GetId());
            ASSERT_EQ(aspectEntry.GetElementId(), element->GetElementId());
            ++aspectCount;
            }
        ASSERT_EQ(1, aspectCount);
        }

    // Test iteration of DgnPlatformTest.TestUniqueAspect
        {
        int aspectCount = 0;
        ElementAspectIterator aspectIterator = m_db->Elements().MakeAspectIterator(DPTEST_SCHEMA(DPTEST_TEST_UNIQUE_ASPECT_CLASS_NAME));
        for (ElementAspectIteratorEntryCR aspectEntry : aspectIterator)
            {
            ASSERT_EQ(aspectEntry.GetClassId(), aspectClassUnique->GetId());
            ASSERT_EQ(aspectEntry.GetElementId(), element->GetElementId());
            ++aspectCount;
            }
        ASSERT_EQ(1, aspectCount);
        }

    // Test iteration of DgnPlatformTest.TestMultiAspectNoHandler
        {
        int aspectCount = 0;
        ElementAspectIterator aspectIterator = m_db->Elements().MakeAspectIterator(DPTEST_SCHEMA(DPTEST_CLASS_TestMultiAspectNoHandler));
        for (ElementAspectIteratorEntryCR aspectEntry : aspectIterator)
            {
            ECInstanceId aspectInstanceId = aspectEntry.GetECInstanceId();
            ASSERT_TRUE((aspectInstanceId == aspectInstanceId3) || (aspectInstanceId == aspectInstanceId4));
            ASSERT_EQ(aspectEntry.GetClassId(), aspectClassMultiNoHandler->GetId());
            ASSERT_EQ(aspectEntry.GetElementId(), element->GetElementId());
            ++aspectCount;
            }
        ASSERT_EQ(2, aspectCount);
        }

    // Test iteration of DgnPlatformTest.TestMultiAspect
        {
        int aspectCount = 0;
        ElementAspectIterator aspectIterator = m_db->Elements().MakeAspectIterator(DPTEST_SCHEMA(DPTEST_TEST_MULTI_ASPECT_CLASS_NAME));
        for (ElementAspectIteratorEntryCR aspectEntry : aspectIterator)
            {
            ECInstanceId aspectInstanceId = aspectEntry.GetECInstanceId();
            ASSERT_TRUE((aspectInstanceId == aspectInstanceId5) || (aspectInstanceId == aspectInstanceId6));
            ASSERT_EQ(aspectEntry.GetClassId(), aspectClassMulti->GetId());
            ASSERT_EQ(aspectEntry.GetElementId(), element->GetElementId());
            ++aspectCount;
            }
        ASSERT_EQ(2, aspectCount);
        }

    // Test iteration of all aspects owned by element
        {
        int aspectCount = 0;
        ElementAspectIterator aspectIterator = element->MakeAspectIterator();
        for (ElementAspectIteratorEntryCR aspectEntry : aspectIterator)
            {
            ECInstanceId aspectInstanceId = aspectEntry.GetECInstanceId();
            DgnClassId classId = aspectEntry.GetClassId();

            if (aspectClassMultiNoHandler->GetId() == classId)
                {
                ASSERT_TRUE((aspectInstanceId == aspectInstanceId3) || (aspectInstanceId == aspectInstanceId4));
                }
            else if (aspectClassMultiNoHandler->GetId() == classId)
                {
                ASSERT_TRUE((aspectInstanceId == aspectInstanceId5) || (aspectInstanceId == aspectInstanceId6));
                }

            ASSERT_EQ(aspectEntry.GetElementId(), element->GetElementId());
            ++aspectCount;
            }
        ASSERT_EQ(6, aspectCount);
        }
    }
