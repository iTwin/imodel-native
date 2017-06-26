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
        //m_db->Schemas().CreateClassViewsInDb();
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

    if ((DgnDbStatus::Success == element->SetPropertyValue("BaseStr", stringVal.c_str())) &&
        (DgnDbStatus::Success == element->SetPropertyValue("BaseLong", ECN::ECValue(longVal))) &&
        (DgnDbStatus::Success == element->SetPropertyValue("BaseDouble", doubleVal)))
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
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME))
        return SetPerfElementPropertyValues(element, update);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME))
        return SetPerfElementSub1PropertyValues(element, update);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME))
        return SetPerfElementSub2PropertyValues(element, update);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME) || 0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME))
        return SetPerfElementSub3PropertyValues(element, update);

    return DgnDbStatus::BadElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Sam.Wilson                       01/17
//+---------------+---------------+---------------+---------------+---------------+------
Dgn::PhysicalElementPtr PerformanceElementsCRUDTestFixture::CreatePerfElement(Utf8CP className, DgnModelR targetModel, DgnCategoryId catId, DgnElementId parent, DgnClassId dgnClassId) const
    {
    if (0 == strcmp(className, PERF_TEST_PERFELEMENT_CLASS_NAME))
        return PerfElement::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB1_CLASS_NAME))
        return PerfElementSub1::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB2_CLASS_NAME))
        return PerfElementSub2::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTSUB3_CLASS_NAME))
        return PerfElementSub3::Create(*m_db, targetModel.GetModelId(), catId, parent, dgnClassId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME))
        return PerfElementCHBase::Create(*m_db, targetModel.GetModelId(), catId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME))
        return PerfElementCHSub1::Create(*m_db, targetModel.GetModelId(), catId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME))
        return PerfElementCHSub2::Create(*m_db, targetModel.GetModelId(), catId);
    if (0 == strcmp(className, PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME))
        return PerfElementCHSub3::Create(*m_db, targetModel.GetModelId(), catId);
    return nullptr;
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

    for (int i = 0; i < numInstances; i++)
        {
        Dgn::PhysicalElementPtr element = CreatePerfElement(className, *targetModel, catId);
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
    ASSERT_EQ(numInstances, (int) elements.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::VerifyPerfElementSelectParams(DgnElementCR element)
    {
    if (0 != strcmp("PerfElement - InitValue", element.GetPropertyValueString("BaseStr").c_str()) ||
        (10000000 != element.GetPropertyValueUInt64("BaseLong")) ||
        (-3.1416 != element.GetPropertyValueDouble("BaseDouble")))
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

    int minElemId = GetfirstElementId(className);
    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(minElemId + i*elementIdIncrement);
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

    int minElemId = GetfirstElementId(className);
    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    //First build dgnelements with modified Geomtry
    bvector<DgnElementPtr> elements;
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(minElemId + i*elementIdIncrement);
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

    int minElemId = GetfirstElementId(className);
    const int elementIdIncrement = DetermineElementIdIncrement(initialInstanceCount, opCount);

    StopWatch timer(true);
    for (uint64_t i = 0; i < opCount; i++)
        {
        const DgnElementId id(minElemId + i*elementIdIncrement);
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
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), opCount, totalDescription.c_str());
#ifdef PERF_ELEM_CRUD_LOG_TO_CONSOLE
    printf("%.8f %s\n", timer.GetElapsedSeconds(), totalDescription.c_str());
#endif
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Majd.Uddin                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
int  PerformanceElementsCRUDTestFixture::GetfirstElementId(Utf8CP className)
{

    uint64_t firstElemId = s_firstElementId;
    const DgnElementId id(firstElemId);
    DgnElementCPtr element = m_db->Elements().GetElement(id);
    if (!element.IsValid())
    {// Get the minimum Id from bis_Element table.
        Statement stat1;
        DgnClassId classId = m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, className);
        
        DbResult result = stat1.Prepare(*m_db, "SELECT min(Id) from bis_Element where ECClassId=?");
        stat1.BindId(1, classId);
        EXPECT_EQ(result, BE_SQLITE_OK);
        EXPECT_TRUE(stat1.IsPrepared());

        EXPECT_EQ(BE_SQLITE_ROW, stat1.Step());
        firstElemId = stat1.GetValueInt(0);
        const DgnElementId id2(firstElemId);
        DgnElementCPtr element2 = m_db->Elements().GetElement(id2);
        EXPECT_TRUE(element2.IsValid());
    }
    return firstElemId;
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
// @bsimethod                                     Sam.Wilson                      01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, InsertApiCH)
    {
    ApiInsertTime(PERF_TEST_PERFELEMENTCHBASE_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTCHSUB1_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTCHSUB2_CLASS_NAME);
    ApiInsertTime(PERF_TEST_PERFELEMENTCHSUB3_CLASS_NAME);
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

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct QueryCategoryIdPerformanceTest : PerformanceElementsCRUDTestFixture
{
    DgnSubCategoryId    m_subCategoryId;

    void Initialize();

    template<typename T> double TimeStatement(T& stmt, uint32_t nIterations, Utf8CP sql)
        {
        StopWatch timer(true);
        for (uint32_t i = 0; i < nIterations; i++)
            {
            stmt.BindId(1, m_subCategoryId);
            stmt.Step();
            stmt.Reset();
            }

        double elapsed = timer.GetCurrentSeconds();
#if defined(QUERY_CATEGORY_ID_OUTPUT_RESULTS)
        printf("%f seconds to execute %u iterations of statement: %s\n", elapsed, nIterations, sql);
#endif
        return elapsed;
        }

    double TimeSQLStatement(Utf8CP sql, uint32_t nIterations) { return TimeStatement(*m_db->GetCachedStatement(sql), nIterations, sql); }
    double TimeECSqlStatement(Utf8CP ecsql, uint32_t nIterations)
        {
        auto stmt = m_db->GetPreparedECSqlStatement(ecsql);
        double elapsed = TimeStatement(*stmt, nIterations, ecsql);

#if defined(QUERY_CATEGORY_ID_EXPLAIN_QUERY)
        printf("Native SQL: %s\n", stmt->GetNativeSql());
        auto exp1 = m_db->ExplainQuery(stmt->GetNativeSql(), false),
             exp2 = m_db->ExplainQuery(stmt->GetNativeSql(), true);

        printf("ExplainQuery(false) =>\n%s\n", exp1.c_str());
        printf("ExplainQuery(true) => \n%s\n", exp2.c_str());
#endif

        return elapsed;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryCategoryIdPerformanceTest::Initialize()
    {
    SetUpTestDgnDb(L"QueryCategoryIdsPerf.ibim", PERF_TEST_PERFELEMENTSUB3_CLASS_NAME, 0);
    m_subCategoryId = DgnCategory::GetDefaultSubCategoryId(m_defaultCategoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryCategoryIdPerformanceTest, TimeStatements)
    {
    Initialize();

    constexpr uint32_t nIterations = 500000;
    TimeSQLStatement("SELECT ParentId from bis_Element WHERE Id=?", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM " BIS_SCHEMA(BIS_CLASS_SubCategory) " WHERE ECInstanceId=?", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM " BIS_SCHEMA(BIS_CLASS_SubCategory) " WHERE ECInstanceId=? ECSqlOptions NoECClassIdFilter", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM bis.Element WHERE ECInstanceId=?", nIterations);
    TimeECSqlStatement("SELECT Parent.Id FROM bis.Element WHERE ECInstanceId=? ECSqlOptions NoECClassIdFilter", nIterations);
    }
