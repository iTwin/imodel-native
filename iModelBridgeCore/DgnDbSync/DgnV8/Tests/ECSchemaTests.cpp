/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/ECSchemaTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <VersionedDgnV8Api/ECObjects/ECObjectsAPI.h>

BEGIN_UNNAMED_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                          08/15
//----------------------------------------------------------------------------------------
struct ECSchemaTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);
    void SetUp();
    void TearDown();

    protected:
        void VerifyElement(DgnV8Api::ElementId&, Utf8CP className, bool isPrimaryInstance, uint8_t refIndex = 1);
        void VerifyElement(DgnDbR db, DgnV8Api::ElementId&, Utf8CP className, bool isPrimaryInstance, uint8_t refIndex = 1);
        ECObjectsV8::ECSchemaPtr ImportSchemaAndAddInstance(BentleyApi::BeFileNameR fileName, DgnV8Api::ElementId&, Utf8CP schemaString);
        ECObjectsV8::ECSchemaPtr ReadSchema(Utf8CP schemaString);
        void ImportSchemaAndAddInstance(BentleyApi::BeFileNameR fileName, DgnV8Api::ElementId&, ECObjectsV8::ECSchemaPtr& schema, WCharCP className = L"ClassA");
        void ImportSchemaAndAddInstance(V8FileEditor& v8editor, DgnV8Api::ElementId& eid, ECObjectsV8::ECSchemaPtr& schema, WCharCP className = L"ClassA");
        void ImportSchema(BentleyApi::BeFileNameR fileName, ECObjectsV8::ECSchemaPtr& schema);
        void ImportSchema(V8FileEditor& v8editor, ECObjectsV8::ECSchemaPtr& schema);
        ECObjectsV8::ECSchemaPtr ImportSchema(BentleyApi::BeFileNameR fileName, Utf8CP schemaString);
        void ECSchemaTests::WriteInstanceOfSimpleClass(V8FileEditor& v8editor, ECObjectsV8::ECSchemaPtr& schema);

        struct ScopedExternalSchemaLocator
            {
            private:
                ECObjectsV8::SearchPathSchemaFileLocaterPtr m_schemaLocater;
                BentleyStatus                               m_status;
            public:
                ScopedExternalSchemaLocator(Bentley::WString searchPath)
                    {
                    bvector<Bentley::WString> searchPaths;
                    searchPaths.push_back(searchPath);
                    m_schemaLocater = ECObjectsV8::SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
                    m_status = DgnV8Api::DgnECManager::GetManager().RegisterExternalSchemaLocater(*m_schemaLocater);
                    }

                ~ScopedExternalSchemaLocator()
                    {
                    DgnV8Api::DgnECManager::GetManager().UnRegisterExternalSchemaLocater(*m_schemaLocater);
                    }

                BentleyStatus Status() const { return m_status; }
            };
    };

Utf8CP TestSchema = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECSchemaReference name =\"Bentley_Standard_CustomAttributes\" version =\"01.04\" prefix =\"bsca\" />"
"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"n\" typeName=\"int\" />"
"    </ECClass>"
"    <ECClass typeName=\"ClassB\" >"
"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
"    </ECClass>"
"</ECSchema>";

Utf8CP TestSchema2 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema2\" nameSpacePrefix=\"test2\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"ClassC\" >"
"        <ECProperty propertyName=\"q\" typeName=\"Int\" />"
"    </ECClass>"
"</ECSchema>";

Utf8CP SchemaVersionA = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"n\" typeName=\"int\" />"
"    </ECClass>"
"    <ECClass typeName=\"ClassB\" >"
"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
"    </ECClass>"
"    <ECRelationshipClass typeName=\"AtoB\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
"       <Source cardinality=\"(0,1)\" polymorphic=\"True\">"
"           <Class class=\"ClassA\" />"
"       </Source>"
"       <Target cardinality=\"(0,N)\" polymorphic=\"True\">"
"           <Class class=\"ClassB\"/>"
"       </Target>"
"   </ECRelationshipClass>"
"</ECSchema>";

Utf8CP SchemaVersionB = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"n\" typeName=\"int\" />"
"        <ECProperty propertyName=\"m\" typeName=\"int\" />"
"    </ECClass>"
"    <ECClass typeName=\"ClassC\" >"
"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
"    </ECClass>"
"</ECSchema>";

Utf8CP SchemaVersionC = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"o\" typeName=\"string\" />"
"    </ECClass>"
"    <ECClass typeName=\"ClassC\" >"
"        <ECProperty propertyName=\"q\" typeName=\"double\" />"
"    </ECClass>"
"</ECSchema>";

Utf8CP SchemaVersionConflict = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"n\" typeName=\"string\" />"
"    </ECClass>"
"    <ECClass typeName=\"ClassB\" >"
"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
"    </ECClass>"
"</ECSchema>";

Utf8CP SchemaVersionConflictRelationship = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"n\" typeName=\"string\" />"
"    </ECClass>"
"    <ECClass typeName=\"ClassB\" >"
"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
"    </ECClass>"
"    <ECRelationshipClass typeName=\"AtoB\" isDomainClass=\"True\" strength=\"embedding\" strengthDirection=\"forward\">"
"       <Source cardinality=\"(0,1)\" polymorphic=\"True\">"
"           <Class class=\"ClassA\" />"
"       </Source>"
"       <Target cardinality=\"(1,1)\" polymorphic=\"False\">"
"           <Class class=\"ClassB\"/>"
"       </Target>"
"   </ECRelationshipClass>"
"</ECSchema>";

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTests::SetUp()
    {
    T_Super::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTests::TearDown()
    {
    m_wantCleanUp = false;
    T_Super::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsV8::ECSchemaPtr ECSchemaTests::ReadSchema(Utf8CP schemaString)
    {
    ECObjectsV8::ECSchemaPtr schema;
    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    if (SUCCESS != ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaString, *schemaContext))
        return nullptr;
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsV8::ECSchemaPtr ECSchemaTests::ImportSchemaAndAddInstance(BentleyApi::BeFileNameR fileName, DgnV8Api::ElementId& eid, Utf8CP schemaString)
    {
    V8FileEditor v8editor;
    v8editor.Open(fileName);
    ECObjectsV8::ECSchemaPtr schema = ReadSchema(schemaString);
    EXPECT_TRUE(schema.IsValid());

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, schema->GetName().c_str(), L"ClassA"));
    v8editor.Save();
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTests::ImportSchemaAndAddInstance(V8FileEditor& v8editor, DgnV8Api::ElementId& eid, ECObjectsV8::ECSchemaPtr& schema, WCharCP className)
    {
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, schema->GetName().c_str(), className));
    v8editor.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTests::ImportSchemaAndAddInstance(BentleyApi::BeFileNameR fileName, DgnV8Api::ElementId& eid, ECObjectsV8::ECSchemaPtr& schema, WCharCP className)
    {
    V8FileEditor v8editor;
    v8editor.Open(fileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, schema->GetName().c_str(), className));
    v8editor.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTests::ImportSchema(BentleyApi::BeFileNameR fileName, ECObjectsV8::ECSchemaPtr& schema)
    {
    V8FileEditor v8editor;
    v8editor.Open(fileName);
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTests::ImportSchema(V8FileEditor& v8editor, ECObjectsV8::ECSchemaPtr& schema)
    {
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsV8::ECSchemaPtr ECSchemaTests::ImportSchema(BentleyApi::BeFileNameR fileName, Utf8CP schemaString)
    {
    V8FileEditor v8editor;
    v8editor.Open(fileName);
    ECObjectsV8::ECSchemaPtr schema = ReadSchema(schemaString);
    EXPECT_TRUE(schema.IsValid());

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaTests::VerifyElement(DgnV8Api::ElementId& eid, Utf8CP className, bool isPrimaryInstance, uint8_t refIndex)
    {
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    VerifyElement(*db, eid, className, isPrimaryInstance, refIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                03/2016
//---------------+---------------+---------------+---------------+---------------+-------
void ECSchemaTests::VerifyElement(DgnDbR db, DgnV8Api::ElementId& eid, Utf8CP className, bool isPrimaryInstance, uint8_t refIndex)
    {
    DgnElementCPtr elem1 = FindV8ElementInDgnDb(db, eid, refIndex);
    ASSERT_TRUE(elem1.IsValid());

    if (isPrimaryInstance)
        {
        EXPECT_TRUE(elem1->GetElementClass()->GetName().Equals(className));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, Schema_UnUsed)
    {
    LineUpFiles(L"Schema.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, TestSchema, *schemaContext));

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL == ecSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, Schema_Used)
    {
    LineUpFiles(L"Schema.ibim", L"Test3d.dgn", false);

    DgnV8Api::ElementId eid;
    ImportSchemaAndAddInstance(m_v8FileName, eid, TestSchema);

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL != ecSchema);
    VerifyElement(*db, eid, "ClassA", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, SchemasWithDifferentVersion)
    {
    LineUpFiles(L"SchemaVersionMismatch.ibim", L"Test3d.dgn", false);
    DgnV8Api::ElementId eid, eid2;
    {
    BentleyApi::BeFileName refV8File1;
    CreateAndAddV8Attachment(refV8File1, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    ECObjectsV8::ECSchemaPtr schema = ReadSchema(TestSchema);
    EXPECT_TRUE(schema.IsValid());
    ImportSchemaAndAddInstance(refV8File1, eid, schema);

    //schema->SetVersionMajor(2);
    //schema->SetVersionMinor(2);
    ECObjectsV8::ECClassP ecClass;
    EXPECT_TRUE(ECObjectsV8::ECOBJECTS_STATUS_Success == schema->CreateClass(ecClass, L"NewClass"));
    ECObjectsV8::PrimitiveECPropertyP ecProperty;
    EXPECT_TRUE(ECObjectsV8::ECOBJECTS_STATUS_Success == ecClass->CreatePrimitiveProperty(ecProperty, L"NewIntProperty", ECObjectsV8::PrimitiveType::PRIMITIVETYPE_Integer));
    BentleyApi::BeFileName refV8File2;
    CreateAndAddV8Attachment(refV8File2, 2);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    ImportSchemaAndAddInstance(refV8File2, eid2, schema, L"NewClass");
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL != ecSchema);
    /*EXPECT_EQ(2, ecSchema->GetVersionMajor());
    EXPECT_EQ(2, ecSchema->GetVersionMinor());*/
    VerifyElement(*db, eid, "ClassA", true, 2);
    VerifyElement(*db, eid2, "NewClass", true, 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, DuplicateSchemas_AddNewProperty)
    {
    LineUpFiles(L"SchemaMismatch.ibim", L"Test3d.dgn", false);
    DgnV8Api::ElementId eid, eid2;
    {
    BentleyApi::BeFileName refV8File1;
    CreateAndAddV8Attachment(refV8File1, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    ECObjectsV8::ECSchemaPtr schema = ReadSchema(TestSchema);
    EXPECT_TRUE(schema.IsValid());
    EXPECT_TRUE(ECObjectsV8::ECOBJECTS_STATUS_Success == ECObjectsV8::StandardCustomAttributeHelper::SetIsDynamicSchema(*schema, true));
    ImportSchemaAndAddInstance(refV8File1, eid, schema);

    ECObjectsV8::ECClassP testClass = schema->GetClassP(L"ClassA");
    ECObjectsV8::PrimitiveECPropertyP ecProperty;
    EXPECT_TRUE(ECObjectsV8::ECOBJECTS_STATUS_Success == testClass->CreatePrimitiveProperty(ecProperty, L"NewIntProperty", ECObjectsV8::PrimitiveType::PRIMITIVETYPE_Integer));
    BentleyApi::BeFileName refV8File2;
    CreateAndAddV8Attachment(refV8File2, 2);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    ImportSchemaAndAddInstance(refV8File2, eid2, schema);
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL != ecSchema);
    VerifyElement(*db, eid, "ClassA", true, 2);
    VerifyElement(*db, eid2, "ClassA", true, 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, DuplicateSchemas_AddNewClass)
    {
    LineUpFiles(L"SchemaMismatch_AddNewClass.ibim", L"Test3d.dgn", false);
    DgnV8Api::ElementId eid, eid2;
    {
    BentleyApi::BeFileName refV8File1;
    CreateAndAddV8Attachment(refV8File1, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    ECObjectsV8::ECSchemaPtr schema = ReadSchema(TestSchema);
    EXPECT_TRUE(schema.IsValid());
    EXPECT_TRUE(ECObjectsV8::ECOBJECTS_STATUS_Success == ECObjectsV8::StandardCustomAttributeHelper::SetIsDynamicSchema(*schema, true));
    ImportSchemaAndAddInstance(refV8File1, eid, schema);

    ECObjectsV8::ECClassP ecClass;
    EXPECT_TRUE(ECObjectsV8::ECOBJECTS_STATUS_Success == schema->CreateClass(ecClass, L"NewClass"));
    BentleyApi::BeFileName refV8File2;
    CreateAndAddV8Attachment(refV8File2, 2);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    ImportSchemaAndAddInstance(refV8File2, eid2, schema, L"NewClass");
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL != ecSchema);
    VerifyElement(*db, eid, "ClassA", true, 2);
    VerifyElement(*db, eid2, "NewClass", true, 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, DuplicateSchemas_DeleteClass)
    {
    LineUpFiles(L"SchemaMismatch_DeleteClass.ibim", L"Test3d.dgn", false);
    DgnV8Api::ElementId eid, eid2;
    {
    ECObjectsV8::ECSchemaPtr schema;
    BentleyApi::BeFileName refV8File1;
    CreateAndAddV8Attachment(refV8File1, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    schema = ImportSchemaAndAddInstance(refV8File1, eid, TestSchema);

    ECObjectsV8::ECClassP testClass = schema->GetClassP(L"ClassB");
    EXPECT_TRUE(ECObjectsV8::ECOBJECTS_STATUS_Success == schema->DeleteClass(*testClass));
    BentleyApi::BeFileName refV8File2;
    CreateAndAddV8Attachment(refV8File2, 2);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.
    ImportSchemaAndAddInstance(refV8File2, eid2, schema);
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL != ecSchema);
    VerifyElement(*db, eid, "ClassA", true, 2);
    VerifyElement(*db, eid2, "ClassA", true, 3);

    BentleyApi::ECN::ECClassCP classB = ecSchema->GetClassCP("ClassB");
    ASSERT_TRUE(NULL != classB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, ReferenceSchema)
    {
    LineUpFiles(L"SchemaMismatch.ibim", L"Test3d.dgn", false);

    DgnV8Api::ElementId eid;
    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    ECObjectsV8::ECSchemaPtr refSchema = ReadSchema(TestSchema);
    EXPECT_TRUE(refSchema.IsValid());
    ImportSchema(v8editor, refSchema);

    ECObjectsV8::ECSchemaPtr schema = ReadSchema(TestSchema2);
    EXPECT_TRUE(schema.IsValid());
    schema->AddReferencedSchema(*refSchema);

    ImportSchemaAndAddInstance(v8editor, eid, schema, L"ClassC");
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema2");
    ASSERT_TRUE(NULL != ecSchema);
    VerifyElement(*db, eid, "ClassC", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsV8::ECSchemaPtr RegisteExternalSchema(WString ecSchemaXmlFile, Bentley::SchemaInfoR schemaInfo)
    {
    ECObjectsV8::ECSchemaPtr ecSchema = NULL;
    //ECObjectsV8::ECSchemaCachePtr schemaOwner = ECObjectsV8::ECSchemaCache::Create();
    ECObjectsV8::ECSchemaReadContextPtr schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::SchemaReadStatus schemaDesrializationStatus = ECObjectsV8::ECSchema::ReadFromXmlFile(ecSchema, ecSchemaXmlFile.c_str(), *schemaContext);
    if (ECObjectsV8::SCHEMA_READ_STATUS_Success != schemaDesrializationStatus)
        EXPECT_TRUE(false) << "Deserialization Failed";
    ecSchema = DgnV8Api::DgnECManager::GetManager().LocateExternalSchema(schemaInfo, ECObjectsV8::SchemaMatchType::SCHEMAMATCHTYPE_Exact);
    return ecSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, ExternalSchema)
    {
    LineUpFiles(L"ExternalSchema.ibim", L"Test3d.dgn", false);

    ScopedExternalSchemaLocator locator(GetOutputDir().c_str());
    ASSERT_TRUE(SUCCESS == locator.Status());

    Utf8CP refSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"RefSchema\" nameSpacePrefix=\"ref\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"Base\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"FederationGuid\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Status\" typeName=\"string\" />"
        "    </ECClass>"
        "</ECSchema>";

    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECSchemaReference name =\"RefSchema\" version =\"01.01\" prefix =\"ref\" />"
        "    <ECClass typeName=\"Foo\" isDomainClass=\"True\">"
        "        <BaseClass>ref:Base</BaseClass>"
        "        <ECProperty propertyName=\"Code\" typeName=\"string\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Goo\" >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName=\"code\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"status\" typeName=\"int\" />"
        "    </ECClass>"
        "</ECSchema>";


    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema, refSchema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *schemaContext));
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));

    DgnV8Api::ElementId eid;
    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    BentleyApi::BeFileName schemaFileName = GetOutputFileName(L"TestSchema.01.01.ecschema.xml   ");
    BentleyApi::BeFileName refSchemaFileName = GetOutputFileName(L"RefSchema.01.01.ecschema.xml   ");

    ASSERT_TRUE(ECObjectsV8::SCHEMA_WRITE_STATUS_Success == schema->WriteToXmlFile(schemaFileName.c_str()));
    ASSERT_TRUE(ECObjectsV8::SCHEMA_WRITE_STATUS_Success == refSchema->WriteToXmlFile(refSchemaFileName.c_str()));
    EXPECT_TRUE(DgnV8Api::SCHEMAIMPORT_Success == DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *v8editor.m_file, true));

    v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    v8editor.CreateInstanceOnElement(createdDgnECInstance, eh, v8editor.m_defaultModel, schema->GetName().c_str(), L"Foo");
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL != ecSchema);
    VerifyElement(*db, eid, "Foo", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Muhammad Hassan                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSchemaTests, ConvertSchemaWithCaseSensitiveIssues)
    {
    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"foo\" isDomainClass=\"True\">"
        "        <ECArrayProperty propertyName=\"StringArray\" typeName=\"string\"  minOccurs=\"0\" maxOccurs=\"10\" />"
        "        <ECProperty propertyName=\"String\" typeName=\"string\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Foo\" >"
        "        <ECProperty propertyName=\"String2\" typeName=\"string\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Goo\" >"
        "        <ECProperty propertyName=\"GooProp\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"gooprop\" typeName=\"string\" />"
        "    </ECClass>"
        "</ECSchema>";

    LineUpFiles(L"schemawithcasesensitiveissues.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaPtr schema=ImportSchema(m_v8FileName, schemaXml);
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::ElementId eid;
        v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
        DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        v8editor.CreateInstanceOnElement(createdDgnECInstance, eh, v8editor.m_defaultModel, schema->GetName().c_str(), L"Goo");
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    BentleyApi::ECN::ECSchemaCP testSchema = dgnProj->Schemas().GetSchema("TestSchema", true);
    ASSERT_TRUE(NULL != testSchema);

    // classes with same name but differet case should also be handled and converted classes count for TestSchema should be 3
    ASSERT_EQ(testSchema->GetClassCount(), 3);

    BentleyApi::ECN::ECClassCP ecClass = testSchema->GetClassCP("Goo");
    ASSERT_TRUE(nullptr != ecClass);

    // Properties with same name but different case should also be handled 
    // Goo should contain 2 properties in the converted file
    ASSERT_EQ(ecClass->GetPropertyCount(false), 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECSchemaTests, RemapSerializedInstance)
    {
    Utf8CP refSchemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"RefSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"Base\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"FederationGuid\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Status\" typeName=\"string\" />"
        "    </ECClass>"
        "</ECSchema>";

    Utf8CP schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECSchemaReference name =\"RefSchema\" version =\"01.01\" prefix =\"ref\" />"
        "    <ECClass typeName=\"Foo\" isDomainClass=\"True\">"
        "        <BaseClass>ref:Base</BaseClass>"
        "        <ECProperty propertyName=\"Code\" typeName=\"string\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Goo\" >"
        "        <BaseClass>Foo</BaseClass>"      
        "        <ECProperty propertyName=\"code\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"status\" typeName=\"int\" />"
        "    </ECClass>"
        "</ECSchema>";

    LineUpFiles(L"RemapSerializedInstance.ibim", L"Test3d.dgn", false);
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema, schema2;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, refSchemaXml, *schemaContext));
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema2, schemaXml, *schemaContext));
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema2, *(v8editor.m_file)));

    DgnV8Api::ElementId eid, eid2;
    v8editor.AddLine(&eid);
    v8editor.AddLine(&eid2);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::ElementHandle eh2(eid2, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"Goo"));
    Bentley::ECN::ECValue v;
    v.SetInteger(1);
    createdDgnECInstance->SetValue(L"code", v);

    v.SetInteger(5);
    createdDgnECInstance->SetValue(L"status", v);

    v.SetUtf8CP("MyGuid");
    createdDgnECInstance->SetValue(L"FederationGuid", v);

    createdDgnECInstance->WriteChanges();

    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance2, *((DgnV8Api::ElementHandle*)&eh2), v8editor.m_defaultModel, L"TestSchema", L"Goo"));
    v.SetInteger(3);
    createdDgnECInstance2->SetValue(L"code", v);

    v.SetUtf8CP("MyGuid2");
    createdDgnECInstance2->SetValue(L"FederationGuid", v);

    createdDgnECInstance2->WriteChanges();
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    if (true)
        {
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);
        SyncInfo::V8FileSyncInfoId editV8FileSyncInfoId;
        syncInfo.MustFindFileByName(editV8FileSyncInfoId, m_v8FileName);
        SyncInfo::V8ModelSyncInfoId editV8ModelSyncInfoId;
        syncInfo.MustFindModelByV8ModelId(editV8ModelSyncInfoId, editV8FileSyncInfoId, v8editor.m_defaultModel->GetModelId());
        DgnElementId dgnDbElementId;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementId, editV8ModelSyncInfoId, eid);
        DgnElementId dgnDbElementId2;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementId2, editV8ModelSyncInfoId, eid2);

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        //db->Schemas().CreateClassViewsInDb();
        //db->SaveChanges();
        auto dgnDbElement = db->Elements().GetElement(dgnDbElementId);
        ASSERT_TRUE(dgnDbElement.IsValid());

        Utf8String selEcSql;
        selEcSql.append("SELECT test_code_, test_FederationGuid_, test_status_ FROM ").append(dgnDbElement->GetElementClass()->GetECSqlName().c_str()).append("WHERE ECInstanceId=?");
        EC::ECSqlStatement stmt;
        stmt.Prepare(*db, selEcSql.c_str());
        stmt.BindId(1, dgnDbElementId);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_TRUE(1 == stmt.GetValueInt(0));
        ASSERT_TRUE(0 == strcmp("MyGuid", stmt.GetValueText(1)));
        ASSERT_TRUE(5 == stmt.GetValueInt(2));

        Utf8String selEcSql2;
        selEcSql2.append("SELECT test_code_, test_FederationGuid_ FROM ").append(dgnDbElement->GetElementClass()->GetECSqlName().c_str()).append("WHERE ECInstanceId=?");
        EC::ECSqlStatement stmt2;
        stmt2.Prepare(*db, selEcSql.c_str());
        stmt2.BindId(1, dgnDbElementId2);
        ASSERT_EQ(BE_SQLITE_ROW, stmt2.Step());
        ASSERT_TRUE(3 == stmt2.GetValueInt(0));
        ASSERT_TRUE(0 == strcmp("MyGuid2", stmt2.GetValueText(1)));
        }

    // Verify that an update with a remapped property will remap correctly
    DgnV8Api::ElementId eid3;
        {
        v8editor.AddLine(&eid3);
        DgnV8Api::ElementHandle eh(eid3, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance3;
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance3, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"Goo"));
        v.SetInteger(9);
        createdDgnECInstance3->SetValue(L"code", v);

        v.SetUtf8CP("9Guid");
        createdDgnECInstance3->SetValue(L"FederationGuid", v);
        createdDgnECInstance3->WriteChanges();
        v8editor.Save();
        }

    DoUpdate(m_dgnDbFileName, m_v8FileName, false);
    if (true)
        {
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);
        SyncInfo::V8FileSyncInfoId editV8FileSyncInfoId;
        syncInfo.MustFindFileByName(editV8FileSyncInfoId, m_v8FileName);
        SyncInfo::V8ModelSyncInfoId editV8ModelSyncInfoId;
        syncInfo.MustFindModelByV8ModelId(editV8ModelSyncInfoId, editV8FileSyncInfoId, v8editor.m_defaultModel->GetModelId());
        DgnElementId dgnDbElementId3;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementId3, editV8ModelSyncInfoId, eid3);

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        auto dgnDbElement3 = db->Elements().GetElement(dgnDbElementId3);
        ASSERT_TRUE(dgnDbElement3.IsValid());

        Utf8String selEcSql3;
        selEcSql3.append("SELECT test_code_, test_FederationGuid_ FROM ").append(dgnDbElement3->GetElementClass()->GetECSqlName().c_str()).append("WHERE ECInstanceId=?");
        EC::ECSqlStatement stmt3;
        stmt3.Prepare(*db, selEcSql3.c_str());
        stmt3.BindId(1, dgnDbElementId3);
        ASSERT_EQ(BE_SQLITE_ROW, stmt3.Step());
        ASSERT_TRUE(9 == stmt3.GetValueInt(0));
        ASSERT_TRUE(0 == strcmp("9Guid", stmt3.GetValueText(1)));
        }

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECSchemaTests, ExternalSchemaRemoved)
    {
    LineUpFiles(L"ExternalSchemaMissing.ibim", L"Test3d.dgn", false);

    ScopedExternalSchemaLocator locator(GetOutputDir().c_str());
    ASSERT_TRUE(SUCCESS == locator.Status());

    DgnV8Api::ElementId eid;
    {
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    ECObjectsV8::ECSchemaPtr schema = ReadSchema(TestSchema);
    BentleyApi::BeFileName schemaFileName = GetOutputFileName(L"TestSchema.01.01.ecschema.xml   ");

    ASSERT_TRUE(ECObjectsV8::SCHEMA_WRITE_STATUS_Success == schema->WriteToXmlFile(schemaFileName.c_str()));
    DgnV8Api::SchemaInfo info(schema->GetSchemaKey(), *v8editor.m_file);
    ECObjectsV8::ECSchemaPtr externalSchema = RegisteExternalSchema(schemaFileName.c_str(), info);
    EXPECT_TRUE(externalSchema.IsValid());
    EXPECT_TRUE(DgnV8Api::SCHEMAIMPORT_Success == DgnV8Api::DgnECManager::GetManager().ImportSchema(*externalSchema, *v8editor.m_file, true));

    v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    v8editor.CreateInstanceOnElement(createdDgnECInstance, eh, v8editor.m_defaultModel, externalSchema->GetName().c_str(), L"ClassA");
    BentleyApi::BeFileName::BeDeleteFile(schemaFileName);
    }

    DoConvert(m_dgnDbFileName, m_v8FileName);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    BentleyApi::ECN::ECSchemaCP ecSchema = db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(NULL == ecSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan              01/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECSchemaTests, IdSpecificationCASkipIndex)
    {
    Utf8CP SchemaXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECSchemaReference name =\"Bentley_Standard_CustomAttributes\" version =\"01.04\" prefix =\"bsca\" />"
        "    <ECClass typeName=\"ClassWithSyncId\" isDomainClass=\"True\">"
        "        <ECCustomAttributes>"
        "            <SyncIDSpecification xmlns=\"Bentley_Standard_CustomAttributes.01.04\">"
        "               <Property>a</Property>"
        "            </SyncIDSpecification>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName=\"a\" typeName=\"int\" />"
        "    </ECClass>"
        "</ECSchema>";

    LineUpFiles(L"unsupportedcaonproperty.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaPtr schema = ImportSchema(m_v8FileName, SchemaXML);
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::ElementId eid;
        v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
        DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        v8editor.CreateInstanceOnElement(createdDgnECInstance, eh, v8editor.m_defaultModel, schema->GetName().c_str(), L"ClassWithSyncId");
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);

    BentleyApi::Dgn::DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    ASSERT_EQ(1, dgnProj->Schemas().GetSchema("TestSchema", true)->GetClassCP("ClassWithSyncId")->GetPropertyCount(false));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*dgnProj, "SELECT NULL FROM sqlite_master WHERE name='ix_test_ClassWithSyncId_SyncIDSpecification_a' AND type='index'"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Index shouldn't have been created";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan              01/2017
//---------------+---------------+---------------+---------------+---------------+-------
// Strength, StrengthDirection, SourceClassMultiplicity, TargetClassMultiplicity of the converted relationship must match the BIS base relationship.
TEST_F(ECSchemaTests, DerivedRelationshipAttributes)
    {
    LineUpFiles(L"unsupportedcaonproperty.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaPtr schema = ImportSchema(m_v8FileName, SchemaVersionA);
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::ElementId eid;
        v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
        DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        v8editor.CreateInstanceOnElement(createdDgnECInstance, eh, v8editor.m_defaultModel, schema->GetName().c_str(), L"ClassA");
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);

    BentleyApi::Dgn::DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    BentleyApi::ECN::ECSchemaCP ecSchema = dgnProj->Schemas().GetSchema("TestSchema", true);
    BentleyApi::ECN::ECRelationshipClassCP derivedRelationship = ecSchema->GetClassCP("AtoB")->GetRelationshipClassCP();
    BentleyApi::ECN::ECBaseClassesList baseClassesList = derivedRelationship->GetBaseClasses();
    BentleyApi::ECN::ECClassP ecClass = baseClassesList.front();

    BentleyApi::ECN::ECRelationshipClassCP baseRelationship = ecClass->GetRelationshipClassCP();
    ASSERT_EQ(derivedRelationship->GetStrength(), baseRelationship->GetStrength());
    ASSERT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(derivedRelationship->GetSource().GetMultiplicity(), baseRelationship->GetSource().GetMultiplicity()));
    ASSERT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(derivedRelationship->GetTarget().GetMultiplicity(), baseRelationship->GetTarget().GetMultiplicity()));
    ASSERT_EQ(derivedRelationship->GetStrengthDirection(), baseRelationship->GetStrengthDirection());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan              02/2017
//---------------+---------------+---------------+---------------+---------------+-------
// According to implementation source Multiplicity will be set after the base relationship is determined
// Target class Multiplicity is always (0..*)
TEST_F(ECSchemaTests, RelsWithAnyClassConstraint)
    {
    Utf8CP xmlRelsWithAnyClass = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"n\" typeName=\"string\" />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName=\"Rel11\" isDomainClass=\"True\" strength=\"embedding\" strengthDirection=\"forward\">"
        "       <Source cardinality=\"(0,1)\" polymorphic=\"True\">"
        "           <Class class=\"ClassA\" />"
        "       </Source>"
        "       <Target cardinality=\"(0,1)\" polymorphic=\"False\">"
        "           <Class class=\"AnyClass\"/>"
        "       </Target>"
        "   </ECRelationshipClass>"
        "    <ECRelationshipClass typeName=\"Rel1N\" isDomainClass=\"True\" strength=\"embedding\" strengthDirection=\"forward\">"
        "       <Source cardinality=\"(1,1)\" polymorphic=\"True\">"
        "           <Class class=\"AnyClass\" />"
        "       </Source>"
        "       <Target cardinality=\"(0,N)\" polymorphic=\"False\">"
        "           <Class class=\"ClassA\"/>"
        "       </Target>"
        "   </ECRelationshipClass>"
        "    <ECRelationshipClass typeName=\"RelNN\" isDomainClass=\"True\" strength=\"embedding\" strengthDirection=\"backward\">"
        "       <Source cardinality=\"(0,N)\" polymorphic=\"True\">"
        "           <Class class=\"AnyClass\" />"
        "       </Source>"
        "       <Target cardinality=\"(0,N)\" polymorphic=\"False\">"
        "           <Class class=\"AnyClass\"/>"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    LineUpFiles(L"relswithanyclassasconstraint.ibim", L"Test3d.dgn", false);
    m_wantCleanUp = false;

    ECObjectsV8::ECSchemaPtr schema = ImportSchema(m_v8FileName, xmlRelsWithAnyClass);
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::ElementId eid;
        v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
        DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        v8editor.CreateInstanceOnElement(createdDgnECInstance, eh, v8editor.m_defaultModel, schema->GetName().c_str(), L"ClassA");
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);

    BentleyApi::Dgn::DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
    ASSERT_TRUE(dgnProj->IsDbOpen());

    BentleyApi::ECN::ECSchemaCP ecSchema = dgnProj->Schemas().GetSchema("TestSchema", true);

    BentleyApi::ECN::ECRelationshipClassCP relationshipClass = ecSchema->GetClassCP("Rel11")->GetRelationshipClassCP();
    ASSERT_TRUE(relationshipClass != nullptr);
    // AnyClass on Target side of (0-1, 0-1) relationship should be replaced with default BIS class, resulting multiplicity should remain the same
    EXPECT_STREQ("ClassA", relationshipClass->GetSource().GetConstraintClasses().front()->GetName().c_str());
    EXPECT_STREQ("Element", relationshipClass->GetTarget().GetConstraintClasses().front()->GetName().c_str());
    EXPECT_EQ(BentleyApi::ECN::ECRelatedInstanceDirection::Forward, relationshipClass->GetStrengthDirection());
    EXPECT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(BentleyApi::ECN::RelationshipMultiplicity::ZeroMany(), relationshipClass->GetSource().GetMultiplicity()));
    EXPECT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(BentleyApi::ECN::RelationshipMultiplicity::ZeroMany(), relationshipClass->GetTarget().GetMultiplicity()));

    relationshipClass = ecSchema->GetClassCP("Rel1N")->GetRelationshipClassCP();
    ASSERT_TRUE(relationshipClass != nullptr);
    // AnyClass on Source side of (1-1, 0-N) relationship should be replaced with default BIS class, resulting multiplicity should remain the same
    EXPECT_STREQ("Element", relationshipClass->GetSource().GetConstraintClasses().front()->GetName().c_str());
    EXPECT_STREQ("ClassA", relationshipClass->GetTarget().GetConstraintClasses().front()->GetName().c_str());
    EXPECT_EQ(BentleyApi::ECN::ECRelatedInstanceDirection::Forward, relationshipClass->GetStrengthDirection());
    EXPECT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(BentleyApi::ECN::RelationshipMultiplicity::ZeroMany(), relationshipClass->GetSource().GetMultiplicity()));
    EXPECT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(BentleyApi::ECN::RelationshipMultiplicity::ZeroMany(), relationshipClass->GetTarget().GetMultiplicity()));

    relationshipClass = ecSchema->GetClassCP("RelNN")->GetRelationshipClassCP();
    ASSERT_TRUE(relationshipClass != nullptr);
    // AnyClass on both ends of (0-N, 0-N) relationship should be replaced with default BIS class, resulting multiplicity should remain the same
    EXPECT_STREQ("Element", relationshipClass->GetSource().GetConstraintClasses().front()->GetName().c_str());
    EXPECT_STREQ("Element", relationshipClass->GetTarget().GetConstraintClasses().front()->GetName().c_str());
    EXPECT_EQ(BentleyApi::ECN::ECRelatedInstanceDirection::Forward, relationshipClass->GetStrengthDirection());
    EXPECT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(BentleyApi::ECN::RelationshipMultiplicity::ZeroMany(), relationshipClass->GetSource().GetMultiplicity()));
    EXPECT_TRUE(0 == BentleyApi::ECN::RelationshipMultiplicity::Compare(BentleyApi::ECN::RelationshipMultiplicity::ZeroMany(), relationshipClass->GetTarget().GetMultiplicity()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Simi Hartstein                   04/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECSchemaTests, Mixinify)
	{
	Utf8CP schemaMixinTest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<ECSchema schemaName=\"TestMixinifySchema\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
		"    <ECClass typeName=\"BaseInterface\" description=\"Parent class for all interfaces\" displayLabel=\"Interface\" isDomainClass=\"False\" />"
		"    <ECClass typeName=\"BaseObject\" isDomainClass=\"False\">"
		"        <ECProperty propertyName=\"Oid\" typeName=\"string\" />"
		"    </ECClass>"
		"    <ECClass typeName=\"Interface1\" isDomainClass=\"False\">"
		"        <BaseClass>BaseInterface</BaseClass>"
		"    </ECClass>"
		"    <ECClass typeName=\"Interface2\" isDomainClass=\"False\">"
		"        <BaseClass>BaseInterface</BaseClass>"
		"    </ECClass>"
		"    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
		"        <BaseClass>BaseObject</BaseClass>"
		"        <ECProperty propertyName=\"TestProperty1\" typeName=\"string\" />"
		"    </ECClass>"
		"    <ECClass typeName=\"ClassB\" isDomainClass=\"True\">"
		"        <BaseClass>BaseObject</BaseClass>"
		"        <BaseClass>Interface1</BaseClass>"
		"        <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
		"    </ECClass>"
		"    <ECClass typeName=\"ClassC\" isDomainClass=\"True\">"
		"        <BaseClass>BaseObject</BaseClass>"
		"        <BaseClass>Interface1</BaseClass>"
		"        <BaseClass>Interface2</BaseClass>"
		"        <ECProperty propertyName=\"TestProperty2\" typeName=\"string\" />"
		"    </ECClass>"
		"</ECSchema>";

	LineUpFiles(L"mixinify.ibim", L"Test3d.dgn", false);

    ECObjectsV8::ECSchemaPtr schema = ImportSchema(m_v8FileName, schemaMixinTest);
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        DgnV8Api::ElementId eid;
        v8editor.AddLine(&eid, nullptr, DPoint3d::FromOne());
        DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        v8editor.CreateInstanceOnElement(createdDgnECInstance, eh, v8editor.m_defaultModel, schema->GetName().c_str(), L"ClassA");
        }
    DoConvert(m_dgnDbFileName, m_v8FileName);
	
	BentleyApi::Dgn::DgnDbPtr dgnProj = OpenExistingDgnDb(m_dgnDbFileName, Db::OpenMode::Readonly);
	ASSERT_TRUE(dgnProj->IsDbOpen());

	// assert schema exists (schema may not exist if schema conversion failed)
	BentleyApi::ECN::ECSchemaCP ecSchema = dgnProj->Schemas().GetSchema("TestMixinifySchema", true);
	ASSERT_TRUE(ecSchema != nullptr);

	// assert BaseObject, ClassA, ClassB, and ClassC are not mixins
	EXPECT_FALSE(ecSchema->GetClassCP("BaseObject")->GetEntityClassCP()->IsMixin());
	EXPECT_FALSE(ecSchema->GetClassCP("ClassA")->GetEntityClassCP()->IsMixin());
	EXPECT_FALSE(ecSchema->GetClassCP("ClassB")->GetEntityClassCP()->IsMixin());
	EXPECT_FALSE(ecSchema->GetClassCP("ClassC")->GetEntityClassCP()->IsMixin());

	// assert BaseInterface, Interface1, and Interface2 are mixins
	EXPECT_TRUE(ecSchema->GetClassCP("BaseInterface")->GetEntityClassCP()->IsMixin());
	EXPECT_TRUE(ecSchema->GetClassCP("Interface1")->GetEntityClassCP()->IsMixin());
	EXPECT_TRUE(ecSchema->GetClassCP("Interface2")->GetEntityClassCP()->IsMixin());

	// assert correct applies to choice
	EXPECT_STREQ("BaseObject", ecSchema->GetClassCP("Interface1")->GetEntityClassCP()->GetAppliesToClass()->GetName().c_str());
	EXPECT_STREQ("ClassC", ecSchema->GetClassCP("Interface2")->GetEntityClassCP()->GetAppliesToClass()->GetName().c_str());
	EXPECT_STREQ("BaseObject", ecSchema->GetClassCP("BaseInterface")->GetEntityClassCP()->GetAppliesToClass()->GetName().c_str());
	}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECSchemaTests, RemapReservedPropertyNames) // TFS#670031
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" nameSpacePrefix="test" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Foo" isDomainClass="True">
                <ECProperty propertyName="ECInstanceId" typeName="string" />
                <ECProperty propertyName="Id" typeName="string" />
                <ECProperty propertyName="ECClassId" typeName="string" />
                <ECProperty propertyName="SourceECInstanceId" typeName="string" />
                <ECProperty propertyName="SourceId" typeName="string" />
                <ECProperty propertyName="SourceECClassId" typeName="string" />
                <ECProperty propertyName="TargetECInstanceId" typeName="string" />
                <ECProperty propertyName="TargetId" typeName="string" />
                <ECProperty propertyName="TargetECClassId" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

    LineUpFiles(L"RemapReservedPropertyNames.ibim", L"Test3d.dgn", false);
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
    ECObjectsV8::ECSchemaPtr schema;
    EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestSchema", L"Foo"));
    Bentley::ECN::ECValue v;
    v.SetUtf8CP("MyECInstanceId");
    createdDgnECInstance->SetValue(L"ECInstanceId", v);

    v.SetUtf8CP("MyId");
    createdDgnECInstance->SetValue(L"Id", v);

    v.SetUtf8CP("MyECClassId");
    createdDgnECInstance->SetValue(L"ECClassId", v);

    v.SetUtf8CP("MySourceECInstanceId");
    createdDgnECInstance->SetValue(L"SourceECInstanceId", v);

    v.SetUtf8CP("MySourceId");
    createdDgnECInstance->SetValue(L"SourceId", v);

    v.SetUtf8CP("MySourceECClassId");
    createdDgnECInstance->SetValue(L"SourceECClassId", v);

    v.SetUtf8CP("MyTargetECInstanceId");
    createdDgnECInstance->SetValue(L"TargetECInstanceId", v);

    v.SetUtf8CP("MyTargetId");
    createdDgnECInstance->SetValue(L"TargetId", v);

    v.SetUtf8CP("MyTargetECClassId");
    createdDgnECInstance->SetValue(L"TargetECClassId", v);

    createdDgnECInstance->WriteChanges();
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);

    if (true)
        {
        SyncInfoReader syncInfo;
        syncInfo.AttachToDgnDb(m_dgnDbFileName);
        SyncInfo::V8FileSyncInfoId editV8FileSyncInfoId;
        syncInfo.MustFindFileByName(editV8FileSyncInfoId, m_v8FileName);
        SyncInfo::V8ModelSyncInfoId editV8ModelSyncInfoId;
        syncInfo.MustFindModelByV8ModelId(editV8ModelSyncInfoId, editV8FileSyncInfoId, v8editor.m_defaultModel->GetModelId());
        DgnElementId dgnDbElementId;
        syncInfo.MustFindElementByV8ElementId(dgnDbElementId, editV8ModelSyncInfoId, eid);

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        //db->Schemas().CreateClassViewsInDb();
        //db->SaveChanges();
        auto dgnDbElement = db->Elements().GetElement(dgnDbElementId);
        ASSERT_TRUE(dgnDbElement.IsValid());

        Utf8String selEcSql;
        selEcSql.append("SELECT test_ECInstanceId_, test_Id_, test_ECClassId_, test_SourceECInstanceId_, test_SourceId_, test_SourceECClassId_, test_TargetECInstanceId_, test_TargetId_, test_TargetECClassId_ FROM ").append(dgnDbElement->GetElementClass()->GetECSqlName().c_str()).append("WHERE ECInstanceId=?");
        EC::ECSqlStatement stmt;
        stmt.Prepare(*db, selEcSql.c_str());
        stmt.BindId(1, dgnDbElementId);
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_TRUE(0 == strcmp("MyECInstanceId", stmt.GetValueText(0)));
        ASSERT_TRUE(0 == strcmp("MyId", stmt.GetValueText(1)));
        ASSERT_TRUE(0 == strcmp("MyECClassId", stmt.GetValueText(2)));
        ASSERT_TRUE(0 == strcmp("MySourceECInstanceId", stmt.GetValueText(3)));
        ASSERT_TRUE(0 == strcmp("MySourceId", stmt.GetValueText(4)));
        ASSERT_TRUE(0 == strcmp("MySourceECClassId", stmt.GetValueText(5)));
        ASSERT_TRUE(0 == strcmp("MyTargetECInstanceId", stmt.GetValueText(6)));
        ASSERT_TRUE(0 == strcmp("MyTargetId", stmt.GetValueText(7)));
        ASSERT_TRUE(0 == strcmp("MyTargetECClassId", stmt.GetValueText(8)));
         
        BentleyApi::ECN::ECClassCP fooClass = db->Schemas().GetSchema("TestSchema")->GetClassCP("Foo");
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("ECInstanceId"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("Id"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("ECClassId"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("SourceECInstanceId"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("SourceId"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("SourceECClassId"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("TargetECInstanceId"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("TargetId"));
        ASSERT_TRUE(nullptr == fooClass->GetPropertyP("TargetECClassId"));
        }

    }


    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Carole.MacDonald            06/2018
    //---------------+---------------+---------------+---------------+---------------+-------
    TEST_F(ECSchemaTests, SchemaWithMultiInheritance)
        {
        LineUpFiles(L"SkipSchemaWithVerifier.ibim", L"Test3d.dgn", false);
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);

        // Schema names that start with "ECXA_" are automatically flattened
        Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="ECXA_Multi" nameSpacePrefix="testa" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        <ECClass typeName="Ichi" isDomainClass="True">
            <ECProperty propertyName="Alpha" typeName="string" />
        </ECClass>
        <ECClass typeName="Ni" isDomainClass="True">
            <ECProperty propertyName="Beta" typeName="string" />
        </ECClass>
        <ECClass typeName="San" isDomainClass="True">
            <BaseClass>Ichi</BaseClass>
            <BaseClass>Ni</BaseClass>
            <ECProperty propertyName="Gamma" typeName="string" />
        </ECClass>
        <ECClass typeName="Shi" isDomainClass="True">
            <ECProperty propertyName="Delta" typeName="string" />
        </ECClass>
        <ECClass typeName="Go" isDomainClass="True">
            <BaseClass>Ichi</BaseClass>
            <BaseClass>Shi</BaseClass>
            <ECProperty propertyName="Epsilon" typeName="string" />
        </ECClass>
        <ECClass typeName="Roku" isDomainClass="True">
            <BaseClass>Shi</BaseClass>
            <ECProperty propertyName="Zeta" typeName="string" />
        </ECClass>
        <ECRelationshipClass typeName="IchiToShi" isDomainClass="True" strength="referencing" strengthDirection="forward">"
           <Source cardinality="(0,1)" polymorphic="True">"
               <Class class="Ichi" />
           </Source>
           <Target cardinality="(0,N)" polymorphic="True">
               <Class class="Shi"/>
           </Target>
       </ECRelationshipClass>

        <ECRelationshipClass typeName="NiToRoku" isDomainClass="True" strength="referencing" strengthDirection="forward">"
           <Source cardinality="(0,1)" polymorphic="True">"
               <Class class="Ni" />
           </Source>
           <Target cardinality="(0,N)" polymorphic="True">
               <Class class="Roku"/>
           </Target>
       </ECRelationshipClass>

    </ECSchema>)xml";

        ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
        ECObjectsV8::ECSchemaPtr schema;
        EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
        EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));

        DgnV8Api::ElementId eid;
        v8editor.AddLine(&eid);
        DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
        DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
        EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"ECXA_Multi", L"Ichi"));

        v8editor.Save();
        DoConvert(m_dgnDbFileName, m_v8FileName);

        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

        BentleyApi::ECN::ECSchemaCP imported = db->Schemas().GetSchema("ECXA_Multi");
        ASSERT_TRUE(nullptr != imported);

        ASSERT_EQ(8, imported->GetClassCount());

        for (BentleyApi::ECN::ECClassCP ecClass : imported->GetClasses())
            {
            ASSERT_TRUE(ecClass->GetBaseClasses().size() < 2);
            }

        BentleyApi::ECN::ECClassCP san = imported->GetClassCP("San");
        BentleyApi::ECN::ECClassCP shi = imported->GetClassCP("Shi");
        BentleyApi::ECN::ECClassCP go = imported->GetClassCP("Go");
        BentleyApi::ECN::ECClassCP roku = imported->GetClassCP("Roku");
        ASSERT_TRUE(nullptr != san);
        ASSERT_TRUE(nullptr != san->GetPropertyP("Alpha", false));
        ASSERT_TRUE(nullptr != san->GetPropertyP("Beta", false));
        ASSERT_TRUE(nullptr != san->GetPropertyP("Gamma", false));

        BentleyApi::ECN::ECRelationshipClassCP ichiToShi = imported->GetClassCP("IchiToShi")->GetRelationshipClassCP();
        ASSERT_TRUE(ichiToShi->GetTarget().SupportsClass(*roku));
        ASSERT_TRUE(roku->Is(shi));

        ASSERT_TRUE(ichiToShi->GetTarget().SupportsClass(*go));
        ASSERT_FALSE(go->Is(shi));

        }

//---------------------------------------------------------------------------------------
// @bsiclass                                    Carole.MacDonald            01/2018
//---------------+---------------+---------------+---------------+---------------+-------
struct SkipSchemaImportTests : public ConverterTestBaseFixture
    {
    DEFINE_T_SUPER(ConverterTestBaseFixture);

    bool _ShouldImportSchema(BentleyApi::Utf8StringCR fullSchemaName, DgnV8Api::DgnModel& v8Model) override
        {
        if (fullSchemaName.Equals("TestA.01.01"))
            return true;
        return false;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SkipSchemaImportTests, WithInstances)
    {
    LineUpFiles(L"SkipSchema.ibim", L"Test3d.dgn", false);
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    if (true)
        {
        Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestA" nameSpacePrefix="testa" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Foo" isDomainClass="True">
                <ECProperty propertyName="Bar" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

        ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
        ECObjectsV8::ECSchemaPtr schema;
        EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
        EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
        }

    if (true)
        {
        Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestB" nameSpacePrefix="testa" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Goo" isDomainClass="True">
                <ECProperty propertyName="hoo" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

        ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
        ECObjectsV8::ECSchemaPtr schema;
        EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
        EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
        }

    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestA", L"Foo"));

    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid2);
    DgnV8Api::ElementHandle eh2(eid2, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance2, *((DgnV8Api::ElementHandle*)&eh2), v8editor.m_defaultModel, L"TestB", L"Goo"));

    v8editor.Save();
    DoConvert(m_dgnDbFileName, m_v8FileName);

    }

struct TestVerifier : ISChemaImportVerifier
    {
    protected: 
    bool _ShouldImportSchema(BentleyApi::Utf8StringCR fullSchemaName, DgnV8ModelR v8Model) override
        {
        if (fullSchemaName.Equals("TestA.01.01"))
            return true;
        return false;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECSchemaTests, SkipSchemaUsingVerifier)
    {
    LineUpFiles(L"SkipSchemaWithVerifier.ibim", L"Test3d.dgn", false);
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);

    if (true)
        {
        Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestA" nameSpacePrefix="testa" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Foo" isDomainClass="True">
                <ECProperty propertyName="Bar" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

        ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
        ECObjectsV8::ECSchemaPtr schema;
        EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
        EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
        }

    if (true)
        {
        Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestB" nameSpacePrefix="testa" version="01.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="Goo" isDomainClass="True">
                <ECProperty propertyName="hoo" typeName="string" />
            </ECClass>
        </ECSchema>)xml";

        ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
        ECObjectsV8::ECSchemaPtr schema;
        EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
        EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*schema, *(v8editor.m_file)));
        }

    DgnV8Api::ElementId eid;
    v8editor.AddLine(&eid);
    DgnV8Api::ElementHandle eh(eid, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance, *((DgnV8Api::ElementHandle*)&eh), v8editor.m_defaultModel, L"TestA", L"Foo"));

    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid2);
    DgnV8Api::ElementHandle eh2(eid2, v8editor.m_defaultModel);
    DgnV8Api::DgnElementECInstancePtr createdDgnECInstance2;
    EXPECT_EQ(Bentley::BentleyStatus::SUCCESS, v8editor.CreateInstanceOnElement(createdDgnECInstance2, *((DgnV8Api::ElementHandle*)&eh2), v8editor.m_defaultModel, L"TestB", L"Goo"));

    v8editor.Save();
    m_verifier = new TestVerifier();
    DoConvert(m_dgnDbFileName, m_v8FileName);
    delete m_verifier;
    }

