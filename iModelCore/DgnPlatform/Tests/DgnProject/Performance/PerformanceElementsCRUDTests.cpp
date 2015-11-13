/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceElementsCRUDTests.h"

HANDLER_DEFINE_MEMBERS (PerformanceElementHandler)
DOMAIN_DEFINE_MEMBERS (PerformanceElementTestDomain)

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElementTestDomain::PerformanceElementTestDomain () : DgnDomain (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, "Test Schema", 1)
    {
    RegisterHandler (PerformanceElementHandler::GetHandler ());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementTestDomain::RegisterDomainAndImportSchema (DgnDbR db, ECN::ECSchemaR schema)
    {
    DgnDomains::RegisterDomain (PerformanceElementTestDomain::GetDomain ());

    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchema (schema);
    DgnBaseDomain::GetDomain ().ImportSchema (db, schemaContext->GetCache ());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElementPtr PerformanceElement::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id)
    {
    return new PerformanceElement (PhysicalElement::CreateParams (db, modelId, classId, category, Placement3d (), Code (), id, DgnElementId ()));
    }


//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SetUpTestDgnDb(Utf8CP testClassName)
    {
    WString seedFileName;
    seedFileName.Sprintf(L"dgndb_ecsqlvssqlite_%s_seed_%d.dgndb", WString(testClassName, BentleyCharEncoding::Utf8).c_str(),
                         DateTime::GetCurrentTimeUtc().GetDayOfYear());

    BeFileName seedFilePath;
    BeTest::GetHost().GetOutputRoot(seedFilePath);
    seedFilePath.AppendToPath(seedFileName.c_str());

    if (!seedFilePath.DoesPathExist())
        {
        SetupProject(L"3dMetricGeneral.idgndb", seedFileName.c_str(), BeSQLite::Db::OpenMode::ReadWrite);
        ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
        BeFileName searchDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchDir);
        searchDir.AppendToPath(L"ECSchemas").AppendToPath(L"Dgn");
        schemaContext->AddSchemaPath(searchDir.GetName());

        ECN::ECSchemaPtr schema = nullptr;
        ASSERT_EQ (ECN::SchemaReadStatus::Success, ECN::ECSchema::ReadFromXmlString (schema, s_testSchemaXml, *schemaContext));

        PerformanceElementTestDomain::RegisterDomainAndImportSchema(*m_db, *schema);
        ASSERT_TRUE(m_db->IsDbOpen());

        bvector<DgnElementPtr> testElements;
        CreateElements(ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, testClassName, testElements);
        for (DgnElementPtr& element : testElements)
            {
            DgnDbStatus stat = DgnDbStatus::Success;
            element->Insert(&stat);
            ASSERT_EQ(DgnDbStatus::Success, stat);
            }
        m_db->SaveChanges();
        }

    BeFileName testFilePath;
    BeTest::GetHost().GetOutputRoot(testFilePath);

    WString testFileName;
    testFileName.Sprintf(L"dgndb_ecsqlvssqlite_%s.dgndb", WString(testClassName, BentleyCharEncoding::Utf8).c_str());
    testFilePath.AppendToPath(testFileName.c_str());

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(seedFilePath, testFilePath, false));

    DbResult status;
    m_db = DgnDb::OpenDgnDb(&status, testFilePath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_EQ(DbResult::BE_SQLITE_OK, status) << status;
    ASSERT_TRUE(m_db.IsValid());
    }

Utf8CP const PerformanceElementsCRUDTestFixture::s_testSchemaXml =
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
        "  <ECClass typeName='Element1' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>dgn:PhysicalElement</BaseClass>"
        "    <ECProperty propertyName='Prop1_1' typeName='string' />"
        "    <ECProperty propertyName='Prop1_2' typeName='long' />"
        "    <ECProperty propertyName='Prop1_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element2' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element1</BaseClass>"
        "    <ECProperty propertyName='Prop2_1' typeName='string' />"
        "    <ECProperty propertyName='Prop2_2' typeName='long' />"
        "    <ECProperty propertyName='Prop2_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element3' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element2</BaseClass>"
        "    <ECProperty propertyName='Prop3_1' typeName='string' />"
        "    <ECProperty propertyName='Prop3_2' typeName='long' />"
        "    <ECProperty propertyName='Prop3_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element4' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element3</BaseClass>"
        "    <ECProperty propertyName='Prop4_1' typeName='string' />"
        "    <ECProperty propertyName='Prop4_2' typeName='long' />"
        "    <ECProperty propertyName='Prop4_3' typeName='double' />"
        "  </ECClass>"
        "  <ECClass typeName='Element4b' >"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>Element3</BaseClass>"
        "    <ECProperty propertyName='Prop4b_1' typeName='string' />"
        "    <ECProperty propertyName='Prop4b_2' typeName='long' />"
        "    <ECProperty propertyName='Prop4b_3' typeName='double' />"
        "    <ECProperty propertyName='Prop4b_4' typeName='point3d' />"
        "  </ECClass>"
        "  <ECClass typeName='SimpleElement'>"
        "    <ECCustomAttributes>"
        "       <ClassHasHandler xmlns=\"dgn.02.00\" />"
        "    </ECCustomAttributes>"
        "    <BaseClass>dgn:Element</BaseClass>"
        "  </ECClass>"
        "</ECSchema>";

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::CreateElements(Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements)
    {
    DgnModelPtr model = CreatePhysicalModel();
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId(*m_db);
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(schemaName, className));

    for (int i = 0; i < s_initialInstanceCount; i++)
        {
        DgnElementId id = DgnElementId((uint64_t) s_firstElementId + i);
        PerformanceElementPtr element = PerformanceElement::Create(*m_db, model->GetModelId(), mclassId, catid, id);
        ASSERT_TRUE(element != nullptr);
        elements.push_back(element);
        }
    ASSERT_EQ(s_initialInstanceCount, (int) elements.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2015
//---------------+---------------+---------------+---------------+---------------+-------
PhysicalModelPtr PerformanceElementsCRUDTestFixture::CreatePhysicalModel() const
    {
    DgnClassId mclassId = DgnClassId(m_db->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel(PhysicalModel::CreateParams(*m_db, mclassId, DgnModel::CreateModelCode("Instances")));
    EXPECT_EQ(DgnDbStatus::Success, targetModel->Insert());       /* Insert the new model into the DgnDb */
    return targetModel;
    }
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element1 - ";
    int64_t intVal = 10000000LL;
    double doubleVal = -3.1416;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop1_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop1_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop1_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element2 - ";
    int64_t intVal = 20000000LL;
    double doubleVal = 2.71828;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop2_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop2_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop2_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element3 - ";
    int64_t intVal = 30000000LL;
    double doubleVal = 1.414121;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop3_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop3_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop3_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams (BeSQLite::Statement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element4 - ";
    int64_t intVal = 40000000LL;
    double doubleVal = 1.61803398874;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt, updateParams)) ||
        (DgnDbStatus::Success != BindElement3PropertyParams (stmt, updateParams)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop4_1"), stringVal.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop4_2"), intVal)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop4_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::BindParams (DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className)
    {
    bool updateParams = false;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":Id"), element->GetElementId ()));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":ModelId"), element->GetModelId ()));
    DgnAuthority::Code elementCode = DgnAuthority::CreateDefaultCode ();
    if (elementCode.IsEmpty ())
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindNull (stmt.GetParameterIndex (":Code")));
        }
    else
        {
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (stmt.GetParameterIndex (":Code"), elementCode.GetValue ().c_str (), BeSQLite::Statement::MakeCopy::No));
        }
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":CodeAuthorityId"), elementCode.GetAuthority ()));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindText (stmt.GetParameterIndex (":CodeNameSpace"), elementCode.GetNameSpace ().c_str (), BeSQLite::Statement::MakeCopy::No));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":ParentId"), element->GetParentId ()));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":CategoryId"), element->ToGeometrySource ()->GetCategoryId ()));

    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::BindUpdateParams (BeSQLite::Statement& stmt, Utf8CP className)
    {
    bool updateParams = true;
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//BindParams Overloads for ECSql
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams (ECSqlStatement& statement, bool updateParams)
    {
    Utf8String stringVal = "Element1 - ";
    int64_t intVal = 10000000LL;
    double doubleVal = -3.1416;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }

    if ((ECSqlStatus::Success != statement.BindText (statement.GetParameterIndex ("Prop1_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != statement.BindInt64 (statement.GetParameterIndex ("Prop1_2"), intVal)) ||
        (ECSqlStatus::Success != statement.BindDouble (statement.GetParameterIndex ("Prop1_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams (ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element2 - ";
    int64_t intVal = 20000000LL;
    double doubleVal = 2.71828;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop2_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop2_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop2_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams (ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element3 - ";
    int64_t intVal = 30000000LL;
    double doubleVal = 1.414121;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop3_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop3_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop3_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams (ECSqlStatement& stmt, bool updateParams)
    {
    Utf8String stringVal = "Element4 - ";
    int64_t intVal = 40000000LL;
    double doubleVal = 1.61803398874;
    if (updateParams)
        {
        stringVal.append ("UpdatedValue");
        intVal = intVal * 2;
        doubleVal = doubleVal * 2;
        }
    else
        {
        stringVal.append ("InitValue");
        }
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt, updateParams)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt, updateParams)) ||
        (DgnDbStatus::Success != BindElement3PropertyParams (stmt, updateParams)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop4_1"), stringVal.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop4_2"), intVal)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop4_3"), doubleVal)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::BindParams (DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className)
    {
    bool updateParams = false;
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ECInstanceId"), element->GetElementId ()));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ModelId"), element->GetModelId ()));
    DgnAuthority::Code elementCode = DgnAuthority::CreateDefaultCode ();
    if (elementCode.IsEmpty ())
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindNull (stmt.GetParameterIndex ("Code")));
        }
    else
        {
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (stmt.GetParameterIndex ("Code"), elementCode.GetValue ().c_str (), IECSqlBinder::MakeCopy::No));
        }
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("CodeAuthorityId"), elementCode.GetAuthority ()));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindText (stmt.GetParameterIndex ("CodeNameSpace"), elementCode.GetNameSpace ().c_str (), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ParentId"), element->GetParentId ()));
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("CategoryId"), element->ToGeometrySource ()->GetCategoryId ()));

    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::BindUpdateParams (ECSqlStatement& stmt, Utf8CP className)
    {
    bool updateParams = true;
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt, updateParams));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt, updateParams));
        }
    }

//Methods to verify Business Property Values returned by Sql Statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement1Params (BeSQLite::Statement& stmt)
    {
    if ((0 != strcmp ("Element1 - InitValue", stmt.GetValueText (7))) ||
        (stmt.GetValueInt64 (8) != 10000000) ||
        (stmt.GetValueDouble (9) != -3.1416))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement2Params (BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != GetElement1Params (stmt)) ||
        (0 != strcmp ("Element2 - InitValue", stmt.GetValueText (10))) ||
        (stmt.GetValueInt64 (11) != 20000000) ||
        (stmt.GetValueDouble (12) != 2.71828))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement3Params (BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != GetElement1Params (stmt)) ||
        (DgnDbStatus::Success != GetElement2Params (stmt)) ||
        (0 != strcmp ("Element3 - InitValue", stmt.GetValueText (13))) ||
        (stmt.GetValueInt64 (14) != 30000000) ||
        (stmt.GetValueDouble (15) != 1.414121))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement4Params (BeSQLite::Statement& stmt)
    {
    if ((DgnDbStatus::Success != GetElement1Params (stmt)) ||
        (DgnDbStatus::Success != GetElement2Params (stmt)) ||
        (DgnDbStatus::Success != GetElement3Params (stmt)) ||
        (0 != strcmp ("Element4 - InitValue", stmt.GetValueText (16))) ||
        (stmt.GetValueInt64 (17) != 40000000) ||
        (stmt.GetValueDouble (18) != 1.61803398874))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetPropertyValues (BeSQLite::Statement& stmt, Utf8CP className)
    {
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement1Params (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement2Params (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement3Params (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement4Params (stmt));
        }
    }

//OverLoaded Methods to Verify Business property Values returned by ECSql Statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement1Params (ECSqlStatement& stmt)
    {
    if ((0 != strcmp ("Element1 - InitValue", stmt.GetValueText (7))) ||
        (stmt.GetValueInt64 (8) != 10000000) ||
        (stmt.GetValueDouble (9) != -3.1416))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement2Params (ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != GetElement1Params (stmt)) ||
        (0 != strcmp ("Element2 - InitValue", stmt.GetValueText (10))) ||
        (stmt.GetValueInt64 (11) != 20000000) ||
        (stmt.GetValueDouble (12) != 2.71828))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement3Params (ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != GetElement1Params (stmt)) ||
        (DgnDbStatus::Success != GetElement2Params (stmt)) ||
        (0 != strcmp ("Element3 - InitValue", stmt.GetValueText (13))) ||
        (stmt.GetValueInt64 (14) != 30000000) ||
        (stmt.GetValueDouble (15) != 1.414121))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::GetElement4Params (ECSqlStatement& stmt)
    {
    if ((DgnDbStatus::Success != GetElement1Params (stmt)) ||
        (DgnDbStatus::Success != GetElement2Params (stmt)) ||
        (DgnDbStatus::Success != GetElement3Params (stmt)) ||
        (0 != strcmp ("Element4 - InitValue", stmt.GetValueText (16))) ||
        (stmt.GetValueInt64 (17) != 40000000) ||
        (stmt.GetValueDouble (18) != 1.61803398874))
        return DgnDbStatus::ReadError;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetPropertyValues (ECSqlStatement& stmt, Utf8CP className)
    {
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement1Params (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement2Params (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement3Params (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, GetElement4Params (stmt));
        }
    }

//Methods to Generate Sql CRUD statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetInsertSql (Utf8CP className, Utf8StringR insertSql, DgnClassId classId)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);

    insertSql = Utf8String ("INSERT INTO dgn_Element ([Id], ");
    Utf8String insertValuesSql (") VALUES (:Id, ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            insertSql.append (", ");
            insertValuesSql.append (", ");
            }

        insertSql.append ("[").append (prop->GetName ()).append ("]");
        insertValuesSql.append (":").append (prop->GetName ());

        isFirstItem = false;
        }

    insertSql.append (", ECClassId");
    insertValuesSql.append (", ");
    Utf8String insertValues;
    insertValues.Sprintf ("%s%d", insertValuesSql.c_str (), (int)classId.GetValue ());
    insertSql.append (insertValues).append (")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetSelectSql (Utf8CP className, Utf8StringR selectSql)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    selectSql = "SELECT ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (!isFirstItem)
            {
            selectSql.append (", ");
            }
        selectSql.append (prop->GetName ());
        isFirstItem = false;
        }
    selectSql.append (" FROM dgn_Element WHERE Id = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetUpdateSql (Utf8CP className, Utf8StringR updateSql)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    updateSql = "UPDATE dgn_Element SET ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("ModelId", prop->GetName ().c_str ()) || 0 == strcmp ("Code", prop->GetName ().c_str ()) || 0 == strcmp ("CodeAuthorityId", prop->GetName ().c_str ()) || 0 == strcmp ("CodeNameSpace", prop->GetName ().c_str ()) || 0 == strcmp ("ParentId", prop->GetName ().c_str ()) || 0 == strcmp ("CategoryId", prop->GetName ().c_str ()) || 0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            updateSql.append (", ");
            }
        updateSql.append (prop->GetName ()).append (" = :").append (prop->GetName ());
        isFirstItem = false;
        }
    updateSql.append (" WHERE Id = :Id");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetDeleteSql (Utf8StringR deleteSql)
    {
    deleteSql = "DELETE FROM dgn_Element WHERE Id = ?";
    }

//Overloads to Generate ECSql statements. 
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetInsertECSql (Utf8CP className, Utf8StringR insertECSql)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);

    insertECSql = Utf8String ("INSERT INTO ");
    insertECSql.append (ecClassName).append (" ([ECInstanceId], ");
    Utf8String insertValuesSql (") VALUES (:[ECInstanceId], ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            insertECSql.append (", ");
            insertValuesSql.append (", ");
            }

        insertECSql.append ("[").append (prop->GetName ()).append ("]");
        insertValuesSql.append (":[").append (prop->GetName ()).append ("]");

        isFirstItem = false;
        }

    insertECSql.append (insertValuesSql).append (")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetSelectECSql (Utf8CP className, Utf8StringR selectECSql)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);

    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);
    selectECSql = "SELECT ";
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (!isFirstItem)
            {
            selectECSql.append (", ");
            }
        selectECSql.append (prop->GetName ());
        isFirstItem = false;
        }

    selectECSql.append (" FROM ").append (ecClassName).append (" WHERE ECInstanceId = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetUpdateECSql (Utf8CP className, Utf8StringR updateECSql)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);
    updateECSql = "UPDATE ";
    updateECSql.append (ecClassName).append (" SET ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("ModelId", prop->GetName ().c_str ()) || 0 == strcmp ("Code", prop->GetName ().c_str ()) || 0 == strcmp ("CodeAuthorityId", prop->GetName ().c_str ()) || 0 == strcmp ("CodeNameSpace", prop->GetName ().c_str ()) || 0 == strcmp ("ParentId", prop->GetName ().c_str ()) || 0 == strcmp ("CategoryId", prop->GetName ().c_str ()) || 0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            updateECSql.append (", ");
            }
        updateECSql.append (prop->GetName ()).append (" = :").append (prop->GetName ());
        isFirstItem = false;
        }
    updateECSql.append (" WHERE ECInstanceId = :ecInstanceId");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetDeleteECSql (Utf8CP className, Utf8StringR deleteECSql)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);
    deleteECSql = "DELETE FROM ONLY ";
    deleteECSql.append (ecClassName).append (" WHERE ").append ("ECInstanceId = ?");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlInsertTime (Utf8CP className)
    {
    SetUpTestDgnDb(className);

    //Create elements to Insert
    DgnClassId mclassId = DgnClassId (m_db->Schemas ().GetECClassId (DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel (PhysicalModel::CreateParams (*m_db, mclassId, DgnModel::CreateModelCode ("CustomInstances")));
    EXPECT_EQ (DgnDbStatus::Success, targetModel->Insert ());       /* Insert the new model into the DgnDb */
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId (*m_db);
    DgnClassId classId = DgnClassId (m_db->Schemas ().GetECClassId (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className));

    bvector<DgnElementPtr> testElements;
    for (int i = 0; i < s_opCount; i++)
        {
        DgnElementId id = DgnElementId ((uint64_t)(2000000 + i));
        PerformanceElementPtr element = PerformanceElement::Create (*m_db, targetModel->GetModelId (), classId, catid, id);
        ASSERT_TRUE (element != nullptr);
        testElements.push_back (element);
        }
    ASSERT_EQ (s_opCount, (int)testElements.size ());

    ECSqlStatement stmt;
    Utf8String insertECSql;
    GetInsertECSql (className, insertECSql);
    //printf ("\n Insert ECSql %s : %s \n", className, insertECSql.c_str ());

    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, insertECSql.c_str ()));
    for (DgnElementPtr& element : testElements)
        {
        BindParams (element, stmt, className);
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("ECSQL Insert '%s' [Initial count: %d]", className, s_initialInstanceCount).c_str (), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlSelectTime(Utf8CP className)
    {
    SetUpTestDgnDb(className);

    ECSqlStatement stmt;
    Utf8String selectECSql;
    GetSelectECSql(className, selectECSql);
    //printf ("\n Select ECSql %s : %s \n", className, selectECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer(true);
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, selectECSql.c_str()));
    //printf ("\n Native Sql %s : %s \n", className, stmt.GetNativeSql());

    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, id));
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
        GetPropertyValues(stmt, className);
        stmt.Reset();
        stmt.ClearBindings();
        }

    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), Utf8PrintfString("ECSQL SELECT %s [Initial count: %d]", className, s_initialInstanceCount).c_str(), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlUpdateTime (Utf8CP className)
    {
    SetUpTestDgnDb(className);

    ECSqlStatement stmt;
    Utf8String updateECSql;
    GetUpdateECSql (className, updateECSql);
    //printf ("\n Update ECSql %s : %s \n", className, updateECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, updateECSql.c_str ()));
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("ecInstanceId"), id));
        BindUpdateParams (stmt, className);
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("ECSQL UPDATE %s [Initial count: %d]", className, s_initialInstanceCount).c_str (), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlDeleteTime (Utf8CP className)
    {
    SetUpTestDgnDb(className);

    ECSqlStatement stmt;
    Utf8String deleteECSql;
    GetDeleteECSql (className, deleteECSql);
    //printf ("\n Delete ECSql %s : %s \n", className, deleteECSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, deleteECSql.c_str ()));
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (1, id));
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("ECSQL DELETE %s [Initial count: %d]", className, s_initialInstanceCount).c_str (), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlInsertTime (Utf8CP className)
    {
    SetUpTestDgnDb(className);

    //Create elements to Insert
    DgnClassId mclassId = DgnClassId (m_db->Schemas ().GetECClassId (DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalModel));
    PhysicalModelPtr targetModel = new PhysicalModel (PhysicalModel::CreateParams (*m_db, mclassId, DgnModel::CreateModelCode ("CustomInstances")));
    EXPECT_EQ (DgnDbStatus::Success, targetModel->Insert ());       /* Insert the new model into the DgnDb */
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId (*m_db);
    DgnClassId classId = DgnClassId (m_db->Schemas ().GetECClassId (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className));

    bvector<DgnElementPtr> testElements;
    for (int i = 0; i < s_opCount; i++)
        {
        DgnElementId id = DgnElementId ((uint64_t)(2000000 + i));
        PerformanceElementPtr element = PerformanceElement::Create (*m_db, targetModel->GetModelId (), classId, catid, id);
        ASSERT_TRUE (element != nullptr);
        testElements.push_back (element);
        }
    ASSERT_EQ (s_opCount, (int)testElements.size ());

    BeSQLite::Statement stmt;
    Utf8String insertSql;
    DgnElementPtr firstElement = testElements.front ();
    GetInsertSql (className, insertSql, firstElement->GetElementClassId ());
    //printf ("\n Insert Sql %s : %s \n", className, insertSql.c_str ());

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, insertSql.c_str ()));
    for (DgnElementPtr& element : testElements)
        {
        BindParams (element, stmt, className);
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("SQL INSERT %s [Initial count: %d]", className, s_initialInstanceCount).c_str (), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlSelectTime (Utf8CP className)
    {
    SetUpTestDgnDb(className);

    BeSQLite::Statement stmt;
    Utf8String selectSql;
    GetSelectSql (className, selectSql);
    //printf ("\n Select Sql %s : %s \n", className, selectSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, selectSql.c_str ()));
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (1, id));
        ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ());
        GetPropertyValues (stmt, className);
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("Sql Read Time %s_%d", className, s_initialInstanceCount).c_str (), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlUpdateTime (Utf8CP className)
    {
    SetUpTestDgnDb(className);

    BeSQLite::Statement stmt;
    Utf8String updateSql;
    GetUpdateSql (className, updateSql);
    //printf ("\n Update Sql %s : %s \n", className, updateSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, updateSql.c_str ()));
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":Id"), id));
        BindUpdateParams (stmt, className);
        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (TEST_DETAILS, timer.GetElapsedSeconds (), Utf8PrintfString ("SQL UPDATE %s [Initial count: %d]", className, s_initialInstanceCount).c_str (), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlDeleteTime(Utf8CP className)
    {
    SetUpTestDgnDb(className);

    BeSQLite::Statement stmt;
    Utf8String deleteSql;
    GetDeleteSql(deleteSql);
    //printf ("\n Delete Sql %s : %s \n", className, deleteSql.c_str ());

    const int elementIdIncrement = DetermineElementIdIncrement();

    StopWatch timer(true);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(*m_db, deleteSql.c_str()));
    for (int i = 0; i < s_opCount; i++)
        {
        ECInstanceId id(s_firstElementId + i*elementIdIncrement);
        ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindId(1, id));
        ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
        stmt.Reset();
        stmt.ClearBindings();
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), Utf8PrintfString("SQL DELETE %s [Initial count: %d]", className, s_initialInstanceCount).c_str(), s_opCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsInsert)
    {
 /*   SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    SqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);

    /*ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    ECSqlInsertTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, ElementsRead)
    {
/*    SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlSelectTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    SqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT4_CLASS);

/*    ECSqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    ECSqlSelectTime (ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsUpdate)
    {
/*    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    SqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);

/*    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    ECSqlUpdateTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PerformanceElementsCRUDTestFixture, ElementsDelete)
    {
/*    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    SqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);

/*    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT1_CLASS);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT2_CLASS);
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT3_CLASS);*/
    ECSqlDeleteTime(ELEMENT_PERFORMANCE_ELEMENT4_CLASS);
    }