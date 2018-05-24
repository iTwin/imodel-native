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
TEST_F (ElementAspectTests1, UniqueAspectPerformance_AspectInsert)
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
    LogTiming (timer1, "Insert Element With Aspect", "TestElement", false, 0, count);
}

/*=================================================================================**//**
* @bsiclass                                                     Taslim.Murad      05/18
+===============+===============+===============+===============+===============+======*/
TEST_F (ElementAspectTests1, UniqueAspectPerformance_SimpleInsert)
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
    LogTiming (timer, "Insert Element Without Aspect", "TestElement", false, 0, count);
}

