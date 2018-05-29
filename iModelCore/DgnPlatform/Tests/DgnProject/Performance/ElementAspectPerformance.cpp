/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/ElementAspectPerformance.cpp $
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
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
struct ElementAspectTests1 : public DgnDbTestFixture
{
    CodeSpecPtr Create (Utf8CP name, bool insert = true)
    {
        CodeSpecPtr codeSpec = CodeSpec::Create (*m_db, name);
        if (insert)
        {
            EXPECT_EQ (DgnDbStatus::Success, codeSpec->Insert ());
            auto codeSpecId = codeSpec->GetCodeSpecId ();
            EXPECT_TRUE (codeSpecId.IsValid ());
        }

        return codeSpec;
    }

public:
    void LogTiming (StopWatch& timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const;
};

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
void ElementAspectTests1::LogTiming (StopWatch& timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const
{
    Utf8CP noClassIdFilterStr = omitClassIdFilter ? "w/o ECClassId filter " : " ";

    Utf8String totalDescription;
    totalDescription.Sprintf ("%s %s '%s' [Initial count: %d]", description, noClassIdFilterStr, testClassName, initialInstanceCount);
    Utf8String desc;
    desc.Sprintf ("%s", description);
    int pos = desc.find ("API");
    Utf8String opType = desc.substr (pos + 4);
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), opCount, totalDescription.c_str (), totalDescription.c_str (), opType.ToUpper (), initialInstanceCount);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectPerformance_Insert)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //  ... insert element with an aspect
    int const count = 1000;
    TestElementCPtr e2[count];
    TestElementPtr tempE2[count];
    StopWatch timer1 (true);
    for (int i = 1; i < count; i++) {
        tempE2[i] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, ("TestElement2" + i));
        DgnElement::UniqueAspect::SetAspect (*tempE2[i], *TestUniqueAspect::Create ("Initial Value1"));
        ASSERT_NE (nullptr, DgnElement::UniqueAspect::GetAspect (*(tempE2[i]), aclass)) << "element should have a scheduled aspect";
        e2[i] = m_db->Elements ().Insert (*(tempE2[i]));
        ASSERT_TRUE ((e2[i]).IsValid ());
    }
    timer1.Stop ();
    LogTiming (timer1, "Insert Element With a Unique Aspect", "TestElement", false, 0, count);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, SimpleElementPerformance_Insert)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   insert element without aspect
    int const count = 1000;
    TestElementPtr tempEl[count];
    TestElementCPtr el[count];
    StopWatch timer (true);
    for (int i = 1; i < count; i++) {
        tempEl[i] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, ("TestElement" + i));
        ASSERT_EQ (nullptr, DgnElement::UniqueAspect::GetAspect (*(tempEl[i]), aclass)) << "element should have no aspect";
        el[i] = m_db->Elements ().Insert (*(tempEl[i]));
        ASSERT_TRUE ((el[i]).IsValid ());
    }
    timer.Stop ();
    LogTiming (timer, "Insert Simple Element Without Aspect", "TestElement", false, 0, count);
}



/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, SimpleElementPerformance_Update)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   update element without aspect
    int const count = 1000;
    TestElementPtr tempEl[count];
    TestElementCPtr el[count];
    for (int i = 1; i < count; i++) {
        tempEl[i] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, ("TestElement"));
        ASSERT_EQ (nullptr, DgnElement::UniqueAspect::GetAspect (*(tempEl[i]), aclass)) << "element should have no aspect";
        tempEl[i]->SetUserLabel ("value");
        el[i] = m_db->Elements ().Insert (*(tempEl[i]));
        ASSERT_TRUE ((el[i]).IsValid ());
    }

    DgnDbStatus status;
    StopWatch timer1 (true);
    for (int i = 1; i < count; i++) {
        tempEl[i]->SetUserLabel ("updated");
        tempEl[i]->Update (&status);
    }
    timer1.Stop ();
    LogTiming (timer1, "Update Simple Element Without Aspect", "TestElement", false, count, count);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
 +===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectPerformance_Update)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   update element with aspect
    int const count = 1000;
    TestElementPtr tempEl[count];
    TestElementCPtr el[count];
    for (int i = 1; i < count; i++) {
        tempEl[i] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement2");
        DgnElement::UniqueAspect::SetAspect (*tempEl[i], *TestUniqueAspect::Create ("InitialValue"));
        ASSERT_NE (nullptr, DgnElement::UniqueAspect::GetAspect (*tempEl[i], aclass)) << "element should have a scheduled aspect";
        el[i] = m_db->Elements ().Insert (*(tempEl[i]));
        ASSERT_TRUE ((el[i]).IsValid ());
    }

    TestElementPtr tempE3[count];
    TestUniqueAspectP aspect[count];
    for (int i = 1; i < count; i++) {
        tempE3[i] = el[i]->MakeCopy<TestElement> ();
        aspect[i] = DgnElement::UniqueAspect::GetP<TestUniqueAspect> (*tempE3[i], aclass);
        ASSERT_EQ (aspect[i], DgnElement::UniqueAspect::GetP<TestUniqueAspect> (*tempE3[i], aclass)) << "GetP should return the same instance each time we call it";
    }
    StopWatch timer1 (true);
    for (int i = 1; i < count; i++) {
        aspect[i]->SetTestUniqueAspectProperty ("Changed Value");
        ASSERT_TRUE (m_db->Elements ().Update (*tempE3[i]).IsValid ());
    }
    timer1.Stop ();
    LogTiming (timer1, "Update Element With a Unique Aspect", "TestElement2", false, count, count);
}


/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, SimpleElementPerformance_Delete)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   delete element without aspect
    int const count = 1000;
    TestElementPtr tempEl[count];
    TestElementCPtr el[count];
    for (int i = 1; i < count; i++) {
        tempEl[i] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, ("TestElement"));
        ASSERT_EQ (nullptr, DgnElement::UniqueAspect::GetAspect (*(tempEl[i]), aclass)) << "element should have no aspect";
        tempEl[i]->SetUserLabel ("value");
        el[i] = m_db->Elements ().Insert (*(tempEl[i]));
        ASSERT_TRUE ((el[i]).IsValid ());
    }

    StopWatch timer (true);
    for (int i = 1; i < count; i++) {
        tempEl[i]->Delete ();
    }
    timer.Stop ();
    LogTiming (timer, "Delete Simple Element Without any Aspect", "TestElement", false, count, count);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectPerformance_Delete)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //  ... delete element with an aspect
    int const count = 1000;
    TestElementPtr tempEl[count];
    TestElementCPtr el[count];
    for (int i = 1; i < count; i++) {
        tempEl[i] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement2");
        DgnElement::UniqueAspect::SetAspect (*tempEl[i], *TestUniqueAspect::Create ("InitialValue"));
        ASSERT_NE (nullptr, DgnElement::UniqueAspect::GetAspect (*tempEl[i], aclass)) << "element should have a scheduled aspect";
        el[i] = m_db->Elements ().Insert (*(tempEl[i]));
        ASSERT_TRUE ((el[i]).IsValid ());
    }

    StopWatch timer1 (true);
    for (int i = 1; i < count; i++) {
        tempEl[i]->Delete ();
    }
    timer1.Stop ();
    LogTiming (timer1, "Delete Element With a Unique Aspect", "TestElement", false, count, count);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, MultiAspectPerformance_Insert)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestMultiAspect::GetECClass (*m_db);
    TestElementCPtr el;
    int const count = 1000;
    TestMultiAspectPtr a1[count];
    EC::ECInstanceId a1id[count];

    //  Insert an element with a multi aspect...
    StopWatch timer1 (true);
    TestElementPtr tempEl = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    for (int i = 1; i < count; i++)
    {
        a1[i] = TestMultiAspect::Create ("test");
        DgnElement::MultiAspect::AddAspect (*tempEl, *(a1[i]));
    }
    el = m_db->Elements ().Insert (*tempEl);
    ASSERT_TRUE (el.IsValid ());
    timer1.Stop ();
    LogTiming (timer1, "Insert an Element With Multi Aspect " , "TestElement", false, 0, 1);

    for (int i = 1; i < count; i++)
    {
        a1id[i] = a1[i]->GetAspectInstanceId ();
        ASSERT_TRUE (a1id[i].IsValid ());
    }
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, MultiAspectPerformance_Update)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestMultiAspect::GetECClass (*m_db);
    TestElementCPtr el;
    int const count = 1000;
    TestMultiAspectPtr a1[count];
    EC::ECInstanceId a1id[count];

    TestElementPtr tempEl = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    for (int i = 1; i < count; i++)
    {
        a1[i] = TestMultiAspect::Create ("test");
        DgnElement::MultiAspect::AddAspect (*tempEl, *(a1[i]));
    }
    el = m_db->Elements ().Insert (*tempEl);
    ASSERT_TRUE (el.IsValid ());
    for (int i = 1; i < count; i++)
    {
        a1id[i] = a1[i]->GetAspectInstanceId ();
        ASSERT_TRUE (a1id[i].IsValid ());
    }

    TestElementPtr tempE2 = el->MakeCopy<TestElement> ();
    TestMultiAspectP aspect[count];
    for (int i = 1; i < count; i++)
    {
         aspect[i] = DgnElement::MultiAspect::GetP<TestMultiAspect> (*tempE2, aclass, a1id[i]);
         TestMultiAspectCP aspectPersist = DgnElement::MultiAspect::Get<TestMultiAspect> (*el, aclass, a1id[i]);
         ASSERT_TRUE (aspectPersist != nullptr);
         ASSERT_EQ (aspectPersist, DgnElement::MultiAspect::Get<TestMultiAspect> (*el, aclass, a1id[i]));
         ASSERT_EQ (aspect[i], DgnElement::MultiAspect::GetP<TestMultiAspect> (*tempE2, aclass, a1id[i]));
        
    }

    StopWatch timer1 (true);
    for (int i = 1; i < count; i++)
    {
        aspect[i]->SetTestMultiAspectProperty ("updated");
    }
    ASSERT_TRUE (m_db->Elements ().Update (*tempE2).IsValid ());
    timer1.Stop ();
    LogTiming (timer1, "Update an Element With Multiple Aspect", "TestElement", false, 1, 1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, MultiAspectPerformance_Delete)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestMultiAspect::GetECClass (*m_db);
    TestElementCPtr el;
    int const count = 1000;
    TestMultiAspectPtr a1[count];
    EC::ECInstanceId a1id[count];

    TestElementPtr tempEl = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
    for (int i = 1; i < count; i++)
    {
        a1[i] = TestMultiAspect::Create ("test");
        DgnElement::MultiAspect::AddAspect (*tempEl, *(a1[i]));
    }
    el = m_db->Elements ().Insert (*tempEl);
    ASSERT_TRUE (el.IsValid ());
    for (int i = 1; i < count; i++)
    {
        a1id[i] = a1[i]->GetAspectInstanceId ();
        ASSERT_TRUE (a1id[i].IsValid ());
    }

    StopWatch timer1 (true);
    tempEl->Delete ();
    timer1.Stop ();
    LogTiming (timer1, "Delete an Element With Multi Aspects", "TestElement", false, 1, 1);
}