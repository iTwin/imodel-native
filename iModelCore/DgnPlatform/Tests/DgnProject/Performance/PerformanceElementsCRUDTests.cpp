/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceElementsCRUDTests.h"

// Uncomment this if you want elapsed time of each test case logged to console in addition to the log file.
// #define PERF_ELEM_CRUD_LOG_TO_CONSOLE 1

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SetUpTestDgnDb(WCharCP destFileName, Utf8CP testClassName, int initialInstanceCount)
    {
    WString seedFileName;
    seedFileName.Sprintf(L"dgndb_ecsqlvssqlite_%d_%ls_seed%d.ibim", initialInstanceCount, WString(testClassName, BentleyCharEncoding::Utf8).c_str(), DateTime::GetCurrentTimeUtc().GetDayOfYear());

    BeFileName seedFilePath;
    BeTest::GetHost().GetOutputRoot(seedFilePath);
    seedFilePath.AppendToPath(BeFileName(BeTest::GetNameOfCurrentTestCase()));
    seedFilePath.AppendToPath(seedFileName.c_str());
    if (!seedFilePath.DoesPathExist())
        {
        SetupSeedProject(seedFileName.c_str());
        ASSERT_TRUE(m_db->IsDbOpen());
        //m_db->Schemas().CreateECClassViewsInDb();
        {
        bvector<DgnElementPtr> testElements;
        CreateElements(initialInstanceCount, testClassName, testElements, "InitialInstances");
        DgnDbStatus stat = DgnDbStatus::Success;
        for (DgnElementPtr& element : testElements)
            {
            element->Insert(&stat);
            ASSERT_EQ(DgnDbStatus::Success, stat);
            ASSERT_TRUE(element.IsValid());
            }
        }

        m_db->CloseDb();
        }

    BeFileName dgndbFilePath;
    BeTest::GetHost().GetOutputRoot(dgndbFilePath);
    dgndbFilePath.AppendToPath(destFileName);

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedFilePath, dgndbFilePath, false));

    DbResult status;
    m_db = DgnDb::OpenDgnDb(&status, dgndbFilePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_EQ(DbResult::BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(m_db.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool appendEllipse3d(GeometryBuilder& builder, double cx, double cy, double cz)
    {
    DEllipse3d ellipseData = DEllipse3d::From(cx, cy, cz,
                                              0, 0, 2,
                                              0, 3, 0,
                                              0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    return builder.Append(*ellipse);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Majd.Uddin            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
static bool appendSolidPrimitive(GeometryBuilder& builder, double dz, double radius)
    {
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    GeometricPrimitivePtr elmGeom3 = GeometricPrimitive::Create(*solidPrimitive);
    EXPECT_TRUE(elmGeom3.IsValid());
    EXPECT_TRUE(GeometricPrimitive::GeometryType::SolidPrimitive == elmGeom3->GetGeometryType());
    ISolidPrimitivePtr getAsSolid = elmGeom3->GetAsISolidPrimitive();
    EXPECT_TRUE(getAsSolid.IsValid());

    return builder.Append(*getAsSolid);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
static bool useEllipse = true;
void PerformanceElementsCRUDTestFixture::AddGeometry(DgnElementPtr element) const
    {
    GeometrySourceP geomElem = element->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*element->GetModel(), geomElem->GetCategoryId(), DPoint3d::From(0.0, 0.0, 0.0));
    if (useEllipse)
        ASSERT_TRUE(appendEllipse3d(*builder, 1, 2, 3));
    else
        ASSERT_TRUE(appendSolidPrimitive(*builder, 3.0, 1.5));
    ASSERT_EQ(SUCCESS, builder->Finish(*geomElem));

    ASSERT_TRUE(geomElem->HasGeometry());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ExtendGeometry(DgnElementPtr element) const
    {
    GeometrySourceP geomElem = element->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create(*element->GetModel(), geomElem->GetCategoryId(), DPoint3d::From(0.0, 0.0, 0.0));
    if (useEllipse)
        ASSERT_TRUE(appendEllipse3d(*builder, 3, 2, 1));
    else
        ASSERT_TRUE(appendSolidPrimitive(*builder, 6.0, 3.0));

    ASSERT_EQ(SUCCESS, builder->Finish(*geomElem));
    ASSERT_TRUE(geomElem->HasGeometry());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementPropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElement - ";
    uint64_t longVal = 10000000LL;
    double doubleVal = -3.1416;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == element->SetPropertyValue("Str", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Long", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Double", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementSub1PropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElementSub1 - ";
    uint64_t longVal = 20000000LL;
    double doubleVal = 2.71828;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == SetPerfElementPropertyValues(element, update)) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub1Str", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub1Long", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub1Double", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementSub2PropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElementSub2 - ";
    uint64_t longVal = 30000000LL;
    double doubleVal = 1.414121;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == SetPerfElementSub1PropertyValues(element, update)) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub2Str", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub2Long", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub2Double", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPerfElementSub3PropertyValues(DgnElementPtr element, bool update) const
    {
    Utf8String stringVal = "PerfElementSub3 - ";
    uint64_t longVal = 40000000LL;
    double doubleVal = 1.61803398874;

    if (update)
        {
        stringVal.append("UpdatedValue");
        longVal = longVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append("InitValue");
        }

    if ((DgnDbStatus::Success == SetPerfElementSub2PropertyValues(element, update)) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub3Str", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub3Long", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("Sub3Double", doubleVal)))
        return DgnDbStatus::Success;

    return DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::SetPropertyValues(Utf8CP className, DgnElementPtr element, bool update) const
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME))
        return SetPerfElementPropertyValues(element, update);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME))
        return SetPerfElementSub1PropertyValues(element, update);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME))
        return SetPerfElementSub2PropertyValues(element, update);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME))
        return SetPerfElementSub3PropertyValues(element, update);

    return DgnDbStatus::BadElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::CreateElements(int numInstances, Utf8CP className, bvector<DgnElementPtr>& elements, Utf8CP modelName) const
    {
    PhysicalModelPtr targetModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, modelName);
    Utf8String categoryName;
    categoryName.Sprintf("%s_Category", modelName);
    DgnCategoryId catId = DgnDbTestUtils::InsertSpatialCategory(*m_db, categoryName.c_str());

    bool addMultiAspect = false;
    bool addExtKey = false;

    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerfElementPtr element = PerfElement::Create(*m_db, targetModel->GetModelId(), catId);
            ASSERT_EQ(DgnDbStatus::Success, SetPropertyValues(className, element));
            ASSERT_TRUE(element != nullptr);

            AddGeometry(element);
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
                {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(CodeSpecId((uint64_t) 1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
                }

            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerfElementSub1Ptr element = PerfElementSub1::Create(*m_db, targetModel->GetModelId(), catId);
            ASSERT_EQ(DgnDbStatus::Success, SetPropertyValues(className, element));
            ASSERT_TRUE(element != nullptr);

            AddGeometry(element);
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
                {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(CodeSpecId((uint64_t) 1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
                }

            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerfElementSub2Ptr element = PerfElementSub2::Create(*m_db, targetModel->GetModelId(), catId);
            ASSERT_EQ(DgnDbStatus::Success, SetPropertyValues(className, element));
            ASSERT_TRUE(element != nullptr);

            AddGeometry(element);
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
                {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(CodeSpecId((uint64_t) 1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
                }

            elements.push_back(element);
            }
        }
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME))
        {
        for (int i = 0; i < numInstances; i++)
            {
            PerfElementSub3Ptr element = PerfElementSub3::Create(*m_db, targetModel->GetModelId(), catId);
            ASSERT_EQ(DgnDbStatus::Success, SetPropertyValues(className, element));
            ASSERT_TRUE(element != nullptr);

            AddGeometry(element);
            if (addMultiAspect)
                DgnElement::MultiAspect::AddAspect(*element, *TestMultiAspect::Create("Initial Value"));
            if (addExtKey)
                {
                DgnElement::ExternalKeyAspectPtr extkeyAspect = DgnElement::ExternalKeyAspect::Create(CodeSpecId((uint64_t) 1), "TestExtKey");
                ASSERT_TRUE(extkeyAspect.IsValid());
                element->AddAppData(DgnElement::ExternalKeyAspect::GetAppDataKey(), extkeyAspect.get());
                }

            elements.push_back(element);
            }
        }
    ASSERT_EQ(numInstances, (int) elements.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSelectParams(DgnElementCR element)
    {
    if (0 != strcmp("PerfElement - InitValue", element.GetPropertyValueString("Str").c_str()) ||
        (10000000 != element.GetPropertyValueUInt64("Long")) ||
        (-3.1416 != element.GetPropertyValueDouble("Double")))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSub1SelectParams(DgnElementCR element)
    {
    if ((DgnDbStatus::Success != VerifyPerfElementSelectParams(element)) ||
        0 != strcmp("PerfElementSub1 - InitValue", element.GetPropertyValueString("Sub1Str").c_str()) ||
        (20000000 != element.GetPropertyValueUInt64("Sub1Long")) ||
        (2.71828 != element.GetPropertyValueDouble("Sub1Double")))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSub2SelectParams(DgnElementCR element)
    {
    if ((DgnDbStatus::Success != VerifyPerfElementSub1SelectParams(element)) ||
        0 != strcmp("PerfElementSub2 - InitValue", element.GetPropertyValueString("Sub2Str").c_str()) ||
        (30000000 != element.GetPropertyValueUInt64("Sub2Long")) ||
        (1.414121 != element.GetPropertyValueDouble("Sub2Double")))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSub3SelectParams(DgnElementCR element)
    {
    if ((DgnDbStatus::Success != VerifyPerfElementSub2SelectParams(element)) ||
        0 != strcmp("PerfElementSub3 - InitValue", element.GetPropertyValueString("Sub3Str").c_str()) ||
        (40000000 != element.GetPropertyValueUInt64("Sub3Long")) ||
        (1.61803398874 != element.GetPropertyValueDouble("Sub3Double")))
        return DgnDbStatus::ReadError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetPropertyValues(DgnElementCR element, Utf8CP className)
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME))
        return VerifyPerfElementSelectParams(element);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME))
        return VerifyPerfElementSub1SelectParams(element);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME))
        return VerifyPerfElementSub2SelectParams(element);
    else if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME))
        return VerifyPerfElementSub3SelectParams(element);

    return DgnDbStatus::BadElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
DgnElementId PerformanceElementsCRUDTestFixture::generateTimeBasedId(int counter)
    {
    uint64_t part1 = BeTimeUtilities::QueryMillisecondsCounter() << 12;
    uint64_t part2 = counter & 0xFFF;
    return DgnElementId(part1 + part2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    09/2016
//---------------------------------------------------------------------------------------
DgnElementId PerformanceElementsCRUDTestFixture::generateAlternatingBriefcaseId(int counter)
    {
    BeBriefcaseId briefcaseId((counter / 100) % 10 + 2);
    return DgnElementId(BeBriefcaseBasedId(briefcaseId, counter).GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiInsertTime(Utf8CP className, int initialInstanceCount, int opCount, bool setFederationGuid, int idStrategy)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"ElementApiInsert%ls_%d.ibim", wClassName.c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    bvector<DgnElementPtr> testElements;
    CreateElements(opCount, className, testElements, "ElementApiInstances");
    ASSERT_EQ(opCount, (int) testElements.size());

    int i = 0;
    StopWatch timer(true);
    for (DgnElementPtr& element : testElements)
        {
        // optionally allow FederationGuid to be set as part of the performance test
        if (setFederationGuid)
            element->SetFederationGuid(BeGuid(true));

        // optionally allow a different ID allocation strategy for performance comparison purposes
        if (1 == idStrategy)
            element->ForceElementIdForInsert(generateTimeBasedId(++i));
        else if (2 == idStrategy)
            element->ForceElementIdForInsert(generateAlternatingBriefcaseId(++i));

        DgnDbStatus stat = DgnDbStatus::Success;
        element->Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }

    timer.Stop();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Insert", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiSelectTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WString wClassName;
    wClassName.AssignUtf8(className);
    WPrintfString dbName(L"ElementApiSelect%ls_%d.ibim", wClassName.c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);
    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        DgnElementCPtr element = m_db->Elements().GetElement(id);
        ASSERT_TRUE(element != nullptr);
        ASSERT_EQ(DgnDbStatus::Success, GetPropertyValues(*element, className));
        }

    timer.Stop();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Read", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiUpdateTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ElementApiUpdate%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    //First build dgnelements with modified Geomtry
    bvector<DgnElementPtr> elements;
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(id);
        ASSERT_TRUE(element != nullptr);

        ASSERT_EQ(DgnDbStatus::Success, SetPropertyValues(className, element, true));

        ExtendGeometry(element);
        elements.push_back(element);
        }

    //Now update and record time
    StopWatch timer(true);
    for (DgnElementPtr& element : elements)
        {
        DgnDbStatus stat = DgnDbStatus::Success;
        element->DgnElement::Update(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }

    timer.Stop();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Update", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ApiDeleteTime(Utf8CP className, int initialInstanceCount, int opCount)
    {
    WPrintfString dbName(L"ElementApiDelete%ls_%d.ibim", WString(className, BentleyCharEncoding::Utf8).c_str(), opCount);
    SetUpTestDgnDb(dbName.c_str(), className, initialInstanceCount);

    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(s_firstElementId + i*elementIdIncrement);
        STATEMENT_DIAGNOSTICS_LOGCOMMENT("Elements::Delete - START");
        const DgnDbStatus stat = m_db->Elements().Delete(id);
        STATEMENT_DIAGNOSTICS_LOGCOMMENT("Elements::Delete - END");
        ASSERT_EQ(DgnDbStatus::Success, stat);
        }

    timer.Stop();
    m_db->SaveChanges();
    LogTiming(timer, "Element API Delete", className, false, initialInstanceCount, opCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::LogTiming(StopWatch& timer, Utf8CP description, Utf8CP testClassName, bool omitClassIdFilter, int initialInstanceCount, int opCount) const
    {
    Utf8CP noClassIdFilterStr = omitClassIdFilter ? "w/o ECClassId filter " : " ";

    Utf8String totalDescription;
    totalDescription.Sprintf("%s %s '%s' [Initial count: %d]", description, noClassIdFilterStr, testClassName, initialInstanceCount);
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), totalDescription.c_str(), opCount);
#ifdef PERF_ELEM_CRUD_LOG_TO_CONSOLE
    printf("%.8f %s\n", timer.GetElapsedSeconds(), totalDescription.c_str());
#endif
    }

/*******************************************************Class Hierarchy For Performance Tests***********************************************************************************

---------------------------------------------------------------PerfElement(Str, Long, Double)
--------------------------------------------------------------------|
-------------------------------------------------------------PerfElementSub1(Sub1Str, Sub1Long, Sub1Double)
--------------------------------------------------------------------|
-------------------------------------------------------------PerfElementSub2(Sub2Str, Sub2Long, Sub2Double)
--------------------------------------------------------------------|
-------------------------------------------------------------PerfElementSub3(Sub3Str, Sub3Long, Sub3Double)

********************************************************************************************************************************************************************************/

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, InsertApi)
    {
    ApiInsertTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, SelectApi)
    {
    ApiSelectTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiSelectTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiSelectTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiSelectTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, UpdateApi)
    {
    ApiUpdateTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiUpdateTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiUpdateTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiUpdateTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, DeleteApi)
    {
    ApiDeleteTime(PERF_TEST_PERFELEMENT_CLASS_NAME);
    ApiDeleteTime(PERF_TEST_PERFELEMENTSUB1_CLASS_NAME);
    ApiDeleteTime(PERF_TEST_PERFELEMENTSUB2_CLASS_NAME);
    ApiDeleteTime(PERF_TEST_PERFELEMENTSUB3_CLASS_NAME);
    }

// Uncomment this to profile ElementLocksPerformanceTest
// #define PROFILE_ELEMENT_LOCKS_TEST 1
// Uncomment this to output timings of ElementLocksPerformanceTest runs
// #define PRINT_ELEMENT_LOCKS_TEST 1

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   04/16
//=======================================================================================
struct ElementLocksPerformanceTest : PerformanceElementsCRUDTestFixture
    {
    void TestInsert(bool asBriefcase, Utf8CP className, int numElems)
        {
        auto dbName = asBriefcase ? L"LocksBriefcase.ibim" : L"LocksRepository.ibim";
        SetUpTestDgnDb(dbName, className, 0);
        if (asBriefcase)
            TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);

        bvector<DgnElementPtr> elems;
        elems.reserve(numElems);
        CreateElements(numElems, className, elems, "MyModel");

#ifdef PROFILE_ELEMENT_LOCKS_TEST
        printf("Attach profiler...\n");
        getchar();
#endif
        StopWatch timer(true);
        for (auto& elem : elems)
            {
            DgnDbStatus stat;
            elem->Insert(&stat);
            EXPECT_EQ(DgnDbStatus::Success, stat);
            }

        timer.Stop();
#ifdef PRINT_ELEMENT_LOCKS_TEST
        printf("%ls (%d): %f\n", dbName, m_db->GetBriefcaseId().GetValue(), timer.GetElapsedSeconds());
#endif

        m_db->SaveChanges();
        }

    void TestInsert(bool asBriefcase, int nElems) { TestInsert(asBriefcase, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, nElems); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert100) { TestInsert(false, 100); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert1000) { TestInsert(false, 1000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Master_Insert10000) { TestInsert(false, 10000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert100) { TestInsert(true, 100); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert1000) { TestInsert(true, 1000); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementLocksPerformanceTest, Briefcase_Insert10000) { TestInsert(true, 10000); }

