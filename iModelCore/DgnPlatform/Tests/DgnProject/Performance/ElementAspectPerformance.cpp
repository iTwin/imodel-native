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
    void LogTiming (double timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const;
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
void ElementAspectTests1::LogTiming (double timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const
{
    Utf8CP noClassIdFilterStr = omitClassIdFilter ? "w/o ECClassId filter " : " ";

    Utf8String totalDescription;
    totalDescription.Sprintf ("%s %s '%s' [Initial count: %d]", description, noClassIdFilterStr, testClassName, initialInstanceCount);
    Utf8String desc;
    desc.Sprintf ("%s", description);
    int pos = desc.find ("API");
    Utf8String opType = desc.substr (pos + 4);
    LOGTODB (TEST_DETAILS, timer, opCount, totalDescription.c_str (), totalDescription.c_str (), opType.ToUpper (), initialInstanceCount);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectPerformance_Insert)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //  ... insert element with an aspect
    int const count = 1001;
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
    LogTiming (timer1, "Insert Element With a Unique Aspect", "TestElement", false, 0, count-1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, SimpleElementPerformance_Insert)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   insert element without aspect
    int const count = 1001;
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
    LogTiming (timer, "Insert Simple Element Without Aspect", "TestElement", false, 0, count-1);
}



/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, SimpleElementPerformance_Update)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   update element without aspect
    int const count = 1001;
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
    LogTiming (timer1, "Update Simple Element Without Aspect", "TestElement", false, count-1, count-1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
 +===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectPerformance_Update)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   update element with aspect
    int const count = 1001;
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
    LogTiming (timer1, "Update Element With a Unique Aspect", "TestElement2", false, count-1, count-1);
}


/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, SimpleElementPerformance_Delete)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //   delete element without aspect
    int const count = 1001;
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
    LogTiming (timer, "Delete Simple Element Without any Aspect", "TestElement", false, count-1, count-1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectPerformance_Delete)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestUniqueAspect::GetECClass (*m_db);

    //  ... delete element with an aspect
    int const count = 1001;
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
    LogTiming (timer1, "Delete Element With a Unique Aspect", "TestElement", false, count-1, count-1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, MultiAspectPerformance_Insert)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestMultiAspect::GetECClass (*m_db);
    int const Aspectcount = 11;
    int const EleCount = 101;
    TestMultiAspectPtr a1[Aspectcount];
    EC::ECInstanceId a1id[Aspectcount];
    TestElementPtr tempEl[EleCount];
    TestElementCPtr el[EleCount];

    //  Insert an element with a multi aspect...
    StopWatch timer1 (true);
    for (int j = 1; j < EleCount; j++)
    {
        tempEl[j] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        for (int i = 1; i < Aspectcount; i++)
        {
            a1[i] = TestMultiAspect::Create ("test");
            DgnElement::MultiAspect::AddAspect (*tempEl[j], *(a1[i]));
        }
        el[j] = m_db->Elements ().Insert (*tempEl[j]);
        ASSERT_TRUE (el[j].IsValid ());
    }
    timer1.Stop ();
    LogTiming (timer1, "Insert an Element With Multi Aspect ", "TestElement", false, 0, EleCount-1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, MultiAspectPerformance_Update)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestMultiAspect::GetECClass (*m_db);
    int const Aspectcount = 11;
    int const EleCount = 101;
    TestMultiAspectPtr a1[EleCount][Aspectcount];
    EC::ECInstanceId a1id[EleCount][Aspectcount];
    TestElementPtr tempEl[EleCount];
    TestElementCPtr el[EleCount];

    //  Insert an element with a multi aspect...
    StopWatch timer1 (true);
    for (int j = 1; j < EleCount; j++)
    {
        tempEl[j] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        for (int i = 1; i < Aspectcount; i++)
        {
            a1[j][i] = TestMultiAspect::Create ("test");
            DgnElement::MultiAspect::AddAspect (*tempEl[j], *(a1[j][i]));
        }
        el[j] = m_db->Elements ().Insert (*tempEl[j]);
        ASSERT_TRUE (el[j].IsValid ());

        for (int i = 1; i < Aspectcount; i++)
        {
            a1id[j][i] = a1[j][i]->GetAspectInstanceId ();
            ASSERT_TRUE (a1id[j][i].IsValid ());
        }
    }

    //  update element with multi aspects 
    TestElementPtr tempE2[EleCount]; 
    TestMultiAspectP aspect[Aspectcount];
    double timeCount = 0;
    for (int j = 1; j < EleCount; j++)
    {
        tempE2[j] = el[j]->MakeCopy<TestElement> ();
        for (int i = 1; i < Aspectcount; i++)
        {
            aspect[i] = DgnElement::MultiAspect::GetP<TestMultiAspect> (*tempE2[j], aclass, a1id[j][i]);
            TestMultiAspectCP aspectPersist = DgnElement::MultiAspect::Get<TestMultiAspect> (*el[j], aclass, a1id[j][i]);
            ASSERT_TRUE (aspectPersist != nullptr);
            ASSERT_EQ (aspectPersist, DgnElement::MultiAspect::Get<TestMultiAspect> (*el[j], aclass, a1id[j][i]));
            ASSERT_EQ (aspect[i], DgnElement::MultiAspect::GetP<TestMultiAspect> (*tempE2[j], aclass, a1id[j][i]));
        }

        StopWatch timer1 (true);
        for (int i = 1; i < Aspectcount; i++)
        {
            aspect[i]->SetTestMultiAspectProperty ("updated");
        }
        ASSERT_TRUE (m_db->Elements ().Update (*tempE2[j]).IsValid ());
        timer1.Stop ();
        timeCount = timeCount + timer1.GetElapsedSeconds ();
    }
    LogTiming (timeCount, "Update an Element With Multiple Aspect", "TestElement", false, EleCount-1, EleCount-1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, MultiAspectPerformance_Delete)
{
    SetupSeedProject ();
    ECN::ECClassCR aclass = *TestMultiAspect::GetECClass (*m_db);
    int const Aspectcount = 11;
    int const EleCount = 101;
    TestMultiAspectPtr a1[Aspectcount];
    EC::ECInstanceId a1id[Aspectcount];
    TestElementPtr tempEl[EleCount];
    TestElementCPtr el[EleCount];

    //  Insert an element with multi aspect...
    for (int j = 1; j < EleCount; j++)
    {
        tempEl[j] = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, "TestElement");
        for (int i = 1; i < Aspectcount; i++)
        {
            a1[i] = TestMultiAspect::Create ("test");
            DgnElement::MultiAspect::AddAspect (*tempEl[j], *(a1[i]));
        }
        el[j] = m_db->Elements ().Insert (*tempEl[j]);
        ASSERT_TRUE (el[j].IsValid ());
    }

    //  delete an element with multi aspects
    StopWatch timer1 (true);
    for (int j = 1; j < EleCount; j++)
    {
        tempEl[j]->Delete ();
    }
    timer1.Stop ();
    LogTiming (timer1, "Delete an Element With Multi Aspects", "TestElement", false, EleCount-1, EleCount-1);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      07/18
 +===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, SimpleElementPerformanceProperties_Insert)
{
    SetupSeedProject ();
    double doubleVal = 1.61803398874;
    int const count = 101;
    DgnElementCPtr persistentEl[count];

    StopWatch timer1 (true);
    for (int i = 1; i < count; i++)
    {
        DgnClassId classId (m_db->Schemas ().GetClassId (DPTEST_SCHEMA_NAME, "TestElement"));
        TestElement::CreateParams params (*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d (), DgnCode ());
        TestElement el (params);

        // Set properties
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("DoubleProperty1", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("DoubleProperty2", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("DoubleProperty3", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("DoubleProperty4", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("PointProperty1", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("PointProperty2", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("PointProperty3", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("PointProperty4", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("b", false));
        EXPECT_EQ (DgnDbStatus::Success, el.SetPropertyValue ("p2d", DPoint2d::From (0, 9)));

        // Insert the element
        persistentEl[i] = el.Insert ();
        ASSERT_TRUE (persistentEl[i].IsValid ());
    }
    timer1.Stop ();
    LogTiming (timer1, "Insert Simple Elemenet With 10 Properties", "TestElement", false, 0, count-1);

    m_db->SaveChanges ();
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      07/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectElementPerfProperties_Insert1)
{
    USING_NAMESPACE_BENTLEY_EC;
    SetupSeedProject ();
    int const count = 101;

    StopWatch timer1 (true);
    for (int i = 1; i < count; i++)
    {
        TestElementPtr element = TestElement::Create (*m_db, m_defaultModelId, m_defaultCategoryId, ("TestElement"));
        ASSERT_TRUE (element.IsValid ());

        ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass (*m_db);
        ASSERT_NE (aspectClassUnique, nullptr);

        RefCountedPtr<DgnElement::UniqueAspect> aspect = DgnElement::UniqueAspect::CreateAspect (*m_db, *aspectClassUnique);
        ASSERT_TRUE (aspect.IsValid ());
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue (DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty, ECValue ("Aspect2")));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue (DPTEST_TEST_UNIQUE_ASPECT_LengthProperty, ECValue (2.0)));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test1", ECValue (2.0)));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test2", ECValue ("Aspect2t2")));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test3", ECValue (2.0)));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test4", ECValue ("Aspect2t4")));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test5", ECValue (2.0)));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test6", ECValue ("Aspect2t6")));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test7", ECValue (2.0)));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test8", ECValue ("Aspect2t8")));

        DgnElement::UniqueAspect::SetAspect (*element, *aspect);
        ASSERT_TRUE (element->Insert ().IsValid ());
    }
    timer1.Stop ();
    LogTiming (timer1, "Insert Unique Aspect Element With Aspect Having 10 Properties", "TestElement", false, 0, count-1);

    m_db->SaveChanges ();
}




/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      07/18
 +===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectElementPerfProperties_Insert2)
{
    USING_NAMESPACE_BENTLEY_EC;
    SetupSeedProject ();
    int const count = 101;

    StopWatch timer1 (true);
    for (int i = 1; i < count; i++)
    {
        DgnClassId classId (m_db->Schemas ().GetClassId (DPTEST_SCHEMA_NAME, "TestElement"));
        TestElement::CreateParams params (*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d (), DgnCode ());
        TestElement element (params);

        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("PointProperty2", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("PointProperty3", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("PointProperty4", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("b", false));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("p2d", DPoint2d::From (0, 9)));

        ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass (*m_db);
        ASSERT_NE (aspectClassUnique, nullptr);

        RefCountedPtr<DgnElement::UniqueAspect> aspect = DgnElement::UniqueAspect::CreateAspect (*m_db, *aspectClassUnique);
        ASSERT_TRUE (aspect.IsValid ());
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue (DPTEST_TEST_UNIQUE_ASPECT_TestUniqueAspectProperty, ECValue ("Aspect2")));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue (DPTEST_TEST_UNIQUE_ASPECT_LengthProperty, ECValue (2.0)));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test1", ECValue (2.0)));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test2", ECValue ("Aspect2t2")));
        ASSERT_EQ (DgnDbStatus::Success, aspect->SetPropertyValue ("test3", ECValue (2.0)));

        DgnElement::UniqueAspect::SetAspect (element, *aspect);
        ASSERT_TRUE (element.Insert ().IsValid ());
    }
    timer1.Stop ();
    LogTiming (timer1, "Insert Unique Aspect Element With Aspect Having 5 Properties and Element Having 5 Properties", "TestElement", false, 0, count-1);

    m_db->SaveChanges ();
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      07/18
 +===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectElementPerfProperties_Insert3)
{
    USING_NAMESPACE_BENTLEY_EC;
    SetupSeedProject ();
    double doubleVal = 1.61803398874;
    int const count = 101;

    StopWatch timer1 (true);
    for (int i = 1; i < count; i++)
    {
        DgnClassId classId (m_db->Schemas ().GetClassId (DPTEST_SCHEMA_NAME, "TestElement"));
        TestElement::CreateParams params (*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d (), DgnCode ());
        TestElement element (params);

        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("DoubleProperty1", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("DoubleProperty2", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("DoubleProperty3", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("DoubleProperty4", doubleVal));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("PointProperty1", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("PointProperty2", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("PointProperty3", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("PointProperty4", DPoint3d::From (0, 9, 9)));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("b", false));
        EXPECT_EQ (DgnDbStatus::Success, element.SetPropertyValue ("p2d", DPoint2d::From (0, 9)));

        ECClassCP aspectClassUnique = TestUniqueAspect::GetECClass (*m_db);
        ASSERT_NE (aspectClassUnique, nullptr);

        RefCountedPtr<DgnElement::UniqueAspect> aspect = DgnElement::UniqueAspect::CreateAspect (*m_db, *aspectClassUnique);
        ASSERT_TRUE (aspect.IsValid ());

        DgnElement::UniqueAspect::SetAspect (element, *aspect);
        ASSERT_TRUE (element.Insert ().IsValid ());
    }
    timer1.Stop ();
    LogTiming (timer1, "Insert Unique Aspect Element With Aspect Having 0 Properties and Element Having 10 Properties", "TestElement", false, 0, count-1);

    m_db->SaveChanges ();
}
