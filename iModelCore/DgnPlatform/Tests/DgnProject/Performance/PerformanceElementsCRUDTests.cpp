/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/PerformanceElementsCRUDTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceElementsCRUDTests.h"

HANDLER_DEFINE_MEMBERS (PerformanceElement1Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement2Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement3Handler)
HANDLER_DEFINE_MEMBERS (PerformanceElement4Handler)
//HANDLER_DEFINE_MEMBERS (PerformanceElement4bHandler)
DOMAIN_DEFINE_MEMBERS (PerformanceElementTestDomain)

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElementTestDomain::PerformanceElementTestDomain () : DgnDomain (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, "Test Schema", 1)
    {
    RegisterHandler (PerformanceElement1Handler::GetHandler ());
    RegisterHandler (PerformanceElement2Handler::GetHandler ());
    RegisterHandler (PerformanceElement3Handler::GetHandler ());
    RegisterHandler (PerformanceElement4Handler::GetHandler ());
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
PerformanceElement1Ptr PerformanceElement1::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id)
    {
    PerformanceElement1Ptr ptr = new PerformanceElement1 (PhysicalElement::CreateParams (db, modelId, classId, category, Placement3d (), Code (), id, DgnElementId ()));
    if (!ptr.IsValid ())
        return nullptr;
    return ptr.get ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElement2Ptr PerformanceElement2::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id)
    {
    PerformanceElement2Ptr ptr = new PerformanceElement2 (PhysicalElement::CreateParams (db, modelId, classId, category, Placement3d (), Code (), id, DgnElementId ()));
    if (!ptr.IsValid ())
        return nullptr;
    return ptr.get ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElement3Ptr PerformanceElement3::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id)
    {
    PerformanceElement3Ptr ptr = new PerformanceElement3 (PhysicalElement::CreateParams (db, modelId, classId, category, Placement3d (), Code (), id, DgnElementId ()));
    if (!ptr.IsValid ())
        return nullptr;
    return ptr.get ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
PerformanceElement4Ptr PerformanceElement4::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id)
    {
    PerformanceElement4Ptr ptr = new PerformanceElement4 (PhysicalElement::CreateParams (db, modelId, classId, category, Placement3d (), Code (), id, DgnElementId ()));
    if (!ptr.IsValid ())
        return nullptr;
    return ptr.get ();
    }
//
//PerformanceElement4bPtr PerformanceElement4b::Create (Dgn::DgnDbR db, Dgn::DgnModelId modelId, Dgn::DgnClassId classId, Dgn::DgnCategoryId category, DgnElementId id)
//    {
//    PerformanceElement4bPtr ptr = new PerformanceElement4b (PhysicalElement::CreateParams (db, modelId, classId, category, Placement3d (), Code (), id, DgnElementId ()));
//    if (!ptr.IsValid ())
//        return nullptr;
//    return ptr.get ();
//    }

//---------------------------------------------------------------------------------------
// @bsiClass                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PerformanceElementsCRUDTestFixture : public PerformanceElementTestFixture
    {
    private:
        DgnModelPtr CreateElements (int numInstances, Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements)
            {
            DgnModelPtr modelPtr;
            CreateElements (numInstances, schemaName, className, elements, modelPtr);
            return modelPtr;
            }

        void CreateElements (int numInstances, Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements, DgnModelPtr& modelPtr);
        void InitializeProject (WCharCP dbName);

    protected:

        Utf8String m_insertStr;

        void GetSqlStatements (Utf8CP className);
        DgnDbStatus BindElement1PropertyParams (BeSQLite::Statement& stmt);
        DgnDbStatus BindElement2PropertyParams (BeSQLite::Statement& stmt);
        DgnDbStatus BindElement3PropertyParams (BeSQLite::Statement& stmt);
        DgnDbStatus BindElement4PropertyParams (BeSQLite::Statement& stmt);
        //DgnDbStatus BindElement4bPropertyParams (BeSQLite::Statement& stmt);
        void BindInsertParams (DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className);

        void GetECSqlStatements (Utf8CP className);
        DgnDbStatus BindElement1PropertyParams (ECSqlStatement& stmt);
        DgnDbStatus BindElement2PropertyParams (ECSqlStatement& stmt);
        DgnDbStatus BindElement3PropertyParams (ECSqlStatement& stmt);
        DgnDbStatus BindElement4PropertyParams (ECSqlStatement& stmt);
        void BindInsertParams (DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className);

        void ECSqlTimeInsertion (int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName);

        void SqlTimeInsertion (int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName);
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::InitializeProject (WCharCP dbName)
    {
    SetupProject (L"3dMetricGeneral.idgndb", dbName, BeSQLite::Db::OpenMode::ReadWrite);
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext ();
    BeFileName searchDir;
    BeTest::GetHost ().GetDgnPlatformAssetsDirectory (searchDir);
    searchDir.AppendToPath (L"ECSchemas").AppendToPath (L"Dgn");
    schemaContext->AddSchemaPath (searchDir.GetName ());

    ECN::ECSchemaPtr schema = nullptr;
    if (ECN::SCHEMA_READ_STATUS_Success != ECN::ECSchema::ReadFromXmlString (schema, s_testSchemaXml, *schemaContext))
        return;

    PerformanceElementTestDomain::RegisterDomainAndImportSchema (*m_db, *schema);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::CreateElements (int numInstances, Utf8CP schemaName, Utf8CP className, bvector<DgnElementPtr>& elements, DgnModelPtr& model)
    {
    model = CreatePhysicalModel ();
    DgnCategoryId catid = DgnCategory::QueryHighestCategoryId (*m_db);
    DgnClassId mclassId = DgnClassId (m_db->Schemas ().GetECClassId (schemaName, className));
    int j = 5;
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)j);
            j = j + 1;
            PerformanceElement1Ptr element = PerformanceElement1::Create (*m_db, model->GetModelId (), mclassId, catid, id);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)j);
            j = j + 1;
            PerformanceElement2Ptr element = PerformanceElement2::Create (*m_db, model->GetModelId (), mclassId, catid, id);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)j);
            j = j + 1;
            PerformanceElement3Ptr element = PerformanceElement3::Create (*m_db, model->GetModelId (), mclassId, catid, id);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        for (int i = 0; i < numInstances; i++)
            {
            DgnElementId id = DgnElementId ((uint64_t)j);
            j = j + 1;
            PerformanceElement4Ptr element = PerformanceElement4::Create (*m_db, model->GetModelId (), mclassId, catid, id);
            ASSERT_TRUE (element != nullptr);
            elements.push_back (element);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams (BeSQLite::Statement& stmt)
    {
    //ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);
    //for (auto prop: ecClass->GetProperties())
    //    {
    //    }
    int random = rand ();
    Utf8PrintfString str ("Element1 - %d", random);
    if ((DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop1_1"), str.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop1_2"), 10000000)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop1_3"), -3.1416)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams (BeSQLite::Statement& stmt)
    {
    int random = rand ();
    Utf8PrintfString str ("Element2 - %d", random);
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop2_1"), str.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop2_2"), 20000000)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop2_3"), 2.71828)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams (BeSQLite::Statement& stmt)
    {
    int random = rand ();
    Utf8PrintfString str ("Element3 - %d", random);
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop3_1"), str.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop3_2"), 30000000)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop3_3"), 1.414121)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams (BeSQLite::Statement& stmt)
    {
    int random = rand ();
    Utf8PrintfString str ("Element4 - %d", random);
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt)) ||
        (DgnDbStatus::Success != BindElement3PropertyParams (stmt)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop4_1"), str.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop4_2"), 40000000)) ||
        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop4_3"), 1.61803398874)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4bPropertyParams (BeSQLite::Statement& stmt)
//    {
//    int random = rand ();
//    Utf8PrintfString str ("Element4b - %d", random);
//    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt)) ||
//        (DgnDbStatus::Success != BindElement2PropertyParams (stmt)) ||
//        (DgnDbStatus::Success != BindElement3PropertyParams (stmt)) ||
//        (DbResult::BE_SQLITE_OK != stmt.BindText (stmt.GetParameterIndex (":Prop4b_1"), str.c_str (), BeSQLite::Statement::MakeCopy::No)) ||
//        (DbResult::BE_SQLITE_OK != stmt.BindInt64 (stmt.GetParameterIndex (":Prop4b_2"), 45000000)) ||
//        (DbResult::BE_SQLITE_OK != stmt.BindDouble (stmt.GetParameterIndex (":Prop4b_3"), 6.022140857)))
//        return DgnDbStatus::BadArg;
//    return DgnDbStatus::Success;
//    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::BindInsertParams (DgnElementPtr& element, BeSQLite::Statement& stmt, Utf8CP className)
    {
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt));
        }
    //else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS))
    //    {
    //    BindElement4bPropertyParams (stmt);
    //    }
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
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindId (stmt.GetParameterIndex (":CategoryId"), element->ToGeometricElement ()->GetCategoryId ()));
    }

//BindParams Overloads for ECSql
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement1PropertyParams (ECSqlStatement& statement)
    {
    int random = rand ();
    Utf8PrintfString str ("Element1 - %d", random);
    if ((ECSqlStatus::Success != statement.BindText (statement.GetParameterIndex ("Prop1_1"), str.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != statement.BindInt64 (statement.GetParameterIndex ("Prop1_2"), 10000000)) ||
        (ECSqlStatus::Success != statement.BindDouble (statement.GetParameterIndex ("Prop1_3"), -3.1416)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement2PropertyParams (ECSqlStatement& stmt)
    {
    int random = rand ();
    Utf8PrintfString str ("Element2 - %d", random);
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop2_1"), str.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop2_2"), 20000000)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop2_3"), 2.71828)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement3PropertyParams (ECSqlStatement& stmt)
    {
    int random = rand ();
    Utf8PrintfString str ("Element3 - %d", random);
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop3_1"), str.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop3_2"), 30000000)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop3_3"), 1.414121)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus PerformanceElementsCRUDTestFixture::BindElement4PropertyParams (ECSqlStatement& stmt)
    {
    int random = rand ();
    Utf8PrintfString str ("Element4 - %d", random);
    if ((DgnDbStatus::Success != BindElement1PropertyParams (stmt)) ||
        (DgnDbStatus::Success != BindElement2PropertyParams (stmt)) ||
        (DgnDbStatus::Success != BindElement3PropertyParams (stmt)) ||
        (ECSqlStatus::Success != stmt.BindText (stmt.GetParameterIndex ("Prop4_1"), str.c_str (), IECSqlBinder::MakeCopy::No)) ||
        (ECSqlStatus::Success != stmt.BindInt64 (stmt.GetParameterIndex ("Prop4_2"), 40000000)) ||
        (ECSqlStatus::Success != stmt.BindDouble (stmt.GetParameterIndex ("Prop4_3"), 1.61803398874)))
        return DgnDbStatus::BadArg;
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::BindInsertParams (DgnElementPtr& element, ECSqlStatement& stmt, Utf8CP className)
    {
    if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT1_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement1PropertyParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT2_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement2PropertyParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT3_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement3PropertyParams (stmt));
        }
    else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4_CLASS))
        {
        ASSERT_EQ (DgnDbStatus::Success, BindElement4PropertyParams (stmt));
        }
    //else if (0 == strcmp (className, ELEMENT_PERFORMANCE_ELEMENT4b_CLASS))
    //    {
    //    BindElement4bPropertyParams (stmt);
    //    }
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
    ASSERT_EQ (ECSqlStatus::Success, stmt.BindId (stmt.GetParameterIndex ("CategoryId"), element->ToGeometricElement ()->GetCategoryId ()));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetECSqlStatements (Utf8CP className)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);

    Utf8String ecClassName = ECSqlBuilder::ToECSqlSnippet (*ecClass);

    m_insertStr = Utf8String ("INSERT INTO ");
    m_insertStr.append (ecClassName).append (" (ECInstanceId, ");
    Utf8String insertValuesSql (") VALUES (:[ECInstanceId], ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("LastMod", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            m_insertStr.append (", ");
            insertValuesSql.append (", ");
            }

        m_insertStr.append ("[").append (prop->GetName ()).append ("]");
        insertValuesSql.append (":[").append (prop->GetName ()).append ("]");

        isFirstItem = false;
        }

    m_insertStr.append (insertValuesSql).append (")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::GetSqlStatements (Utf8CP className)
    {
    ECN::ECClassCP ecClass = m_db->Schemas ().GetECClass (ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, className);

    m_insertStr = Utf8String ("INSERT INTO dgn_Element ([Id], ");
    Utf8String insertValuesSql (") VALUES (:Id, ");
    bool isFirstItem = true;
    for (auto prop : ecClass->GetProperties (true))
        {
        if (0 == strcmp ("LastMod", prop->GetName ().c_str ()) || 0 == strcmp ("Prop4b_4", prop->GetName ().c_str ()))
            continue;
        if (!isFirstItem)
            {
            m_insertStr.append (", ");
            insertValuesSql.append (", ");
            }

        m_insertStr.append ("[").append (prop->GetName ()).append ("]");
        insertValuesSql.append (":").append (prop->GetName ());

        isFirstItem = false;
        }

    m_insertStr.append (", ECClassId");
    insertValuesSql.append (", 215");
    m_insertStr.append (insertValuesSql).append (")");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::ECSqlTimeInsertion (int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName)
    {
    WString wClassName;
    wClassName.AssignUtf8 (className);
    WPrintfString dbName (L"ECSqlPerformanceElement\\Insert%ls_%d.dgndb", wClassName.c_str (), numInstances);
    InitializeProject (dbName.c_str ());
    bvector<DgnElementPtr> testElements;
    CreateElements (numInstances, schemaName, className, testElements);

    ECSqlStatement stmt;
    /*DgnElementPtr tmpElement = testElements.front ();
    int classId = (int)tmpElement->GetElementClassId ().GetValue ();*/
    GetECSqlStatements (className);
    StopWatch timer (true);

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, m_insertStr.c_str ()));
    for (DgnElementPtr& element : testElements)
        {
        ASSERT_TRUE (element != nullptr);
        BindInsertParams (element, stmt, className);

        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (testcaseName, testName, timer.GetElapsedSeconds (), Utf8PrintfString ("Inserting %d %s elements using ECSql", numInstances, className).c_str (), numInstances);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
void PerformanceElementsCRUDTestFixture::SqlTimeInsertion (int numInstances, Utf8CP schemaName, Utf8CP className, Utf8String testcaseName, Utf8String testName)
    {
    WString wClassName;
    wClassName.AssignUtf8 (className);
    WPrintfString dbName (L"SqlPerformanceElement\\Insert%ls_%d.dgndb", wClassName.c_str (), numInstances);
    InitializeProject (dbName.c_str ());
    bvector<DgnElementPtr> testElements;
    CreateElements (numInstances, schemaName, className, testElements);

    BeSQLite::Statement stmt;
    /*DgnElementPtr tmpElement = testElements.front ();
    int classId = (int)tmpElement->GetElementClassId ().GetValue ();*/
    GetSqlStatements (className);

    StopWatch timer (true);
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (*m_db, m_insertStr.c_str ()));
    for (DgnElementPtr& element : testElements)
        {
        ASSERT_TRUE (element != nullptr);
        BindInsertParams (element, stmt, className);

        ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
        stmt.Reset ();
        stmt.ClearBindings ();
        }
    timer.Stop ();
    LOGTODB (testcaseName, testName, timer.GetElapsedSeconds (), Utf8PrintfString ("Inserting %d %s elements using Sql", numInstances, className).c_str (), numInstances);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (PerformanceElementsCRUDTestFixture, ElementInsertPerformance)
    {
    SqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    ECSqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT1_CLASS, TEST_DETAILS);
    SqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT2_CLASS, TEST_DETAILS);
    ECSqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT2_CLASS, TEST_DETAILS);
    SqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT3_CLASS, TEST_DETAILS);
    ECSqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT3_CLASS, TEST_DETAILS);
    SqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4_CLASS, TEST_DETAILS);
    ECSqlTimeInsertion (1000000, ELEMENT_PERFORMANCE_TEST_SCHEMA_NAME, ELEMENT_PERFORMANCE_ELEMENT4_CLASS, TEST_DETAILS);
    }