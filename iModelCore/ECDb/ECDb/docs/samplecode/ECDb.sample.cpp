/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Overview_ECDb_Include.sampleCode
// this includes the standard ECDb headers and its dependencies
#include <ECDb/ECDbApi.h>
#include <Bentley/DateTime.h>
//__PUBLISH_EXTRACT_END__

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateECDbFile()
    {
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_CreateECDb.sampleCode

    // Initialize ECDb (Call this only once per process!)
    BeFileName tempDir(L"C:\\users\\someuser\\AppData\\Local\\Temp\\");
    BeFileName hostAssetsDir(L"C:\\appdata\\assets\\");
    ECDb::Initialize(tempDir, &hostAssetsDir);

    // Create ECDb file
    ECDb ecdb;
    DbResult r = ecdb.CreateNewDb("C:\\data\\foo.ecdb");
    if (BE_SQLITE_OK != r)
        {
        // do error handling here...
        return ERROR;
        }

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus OpenCloseECDbConnection()
    {
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_OpenCloseECDb.sampleCode

    // Initialize ECDb (Call this only once per process!)
    BeFileName tempDir(L"C:\\users\\someuser\\AppData\\Local\\Temp\\");
    BeFileName hostAssetsDir(L"C:\\appdata\\assets\\");
    ECDb::Initialize(tempDir, &hostAssetsDir);

    // Open ECDb file
    ECDb ecdb;
    DbResult r = ecdb.OpenBeSQLiteDb("C:\\data\\foo.ecdb", ECDb::OpenParams(ECDb::OpenMode::Readonly, DefaultTxn::Yes));
    if (BE_SQLITE_OK != r)
        {
        // do error handling here...
        return ERROR;
        }

    // Work with the ECDb file

    // Close ECDb file
    ecdb.CloseDb();

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbImportSchemaFromECSchemaXml()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ImportSchema.sampleCode
    // 1) Deserialize ECSchema from XML file
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    // Add the ECDb file's schema manager as schema locater so that all schemas already available
    // in the ECDb file can be reused
    schemaContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    // add schema search paths for referenced schemas.
    // This is needed in case the schema references classes from other schemas in other locations.
    schemaContext->AddSchemaPath(L"C:\\schema\\references");

    // Read specified schema. All referenced schemas will also be read. Schema and references
    // are available in the cache of the schemaContext
    ECSchemaPtr schema = nullptr;
    SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlFile(schema, L"C:\\schema\\foo.ecschema.xml", *schemaContext);
    if (SchemaReadStatus::Success != deserializeStat)
        {
        // Schema could not be read into memory. Do error handling here
        return ERROR;
        }

    // 2) Import ECSchema (and its references) into ECDb file. The schema and its references are available in the cache
    //    of the schema context from the XML deserialization step.
    bvector<ECSchemaCP> schemasToImport = schemaContext->GetCache().GetSchemas();
    if (SUCCESS != ecdb.Schemas().ImportSchemas(schemasToImport))
        {
        // Schema(s) could not be imported into the ECDb file. Do error handling here
        ecdb.AbandonChanges();
        return ERROR;
        }

    // commits the changes in the ECDb file (the imported ECSchema) to disk
    ecdb.SaveChanges();
    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaManagerGetSchema()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_SchemaManagerGetSchema.sampleCode

    // Retrieve a schema
    // (Passing false as second parameter indicates that classes of the schema should not be loaded along with the schema, 
    // but be loaded on demand)
    ECSchemaCP fooSchema = ecdb.Schemas().GetSchema("fooschema", false);
    if (fooSchema == nullptr)
        {
        // schema not found. do error handling here...
        return ERROR;
        }

    // do something with the ECSchema
    // ...

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaManagerGetClass()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_SchemaManagerGetClass.sampleCode

    // Retrieve an ECClass
    ECClassCP addressClass = ecdb.Schemas().GetClass("fooschema", "Address");
    if (addressClass == nullptr)
        {
        // class not found. do error handling here...
        return ERROR;
        }

    // do something with the ECClass
    // ...

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

bvector<ECN::ECSchemaCP> ReadSchema(Utf8CP);
BeSQLite::IChangeSet& GetChangeSetInfoFromHub(DateTime& pushDate, Utf8String& createdBy, Utf8CP changeSetHubId);

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   12/17
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbCustomizeChangeCacheFile()
    {
    ECDb primaryECDb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_CustomizeChangeCacheFile.sampleCode

    //*** Step 1: Set-up Change Cache file
    BeFileName changeCacheFilePath = ECDb::GetDefaultChangeCachePath(primaryECDb.GetDbFileName());
    ECDb changeCacheFile;
    primaryECDb.CreateChangeCache(changeCacheFile, changeCacheFilePath);

    //Read a custom schema (the method ReadSchema is just a place holder function to keep the example simple) 
    bvector<ECN::ECSchemaCP> customSchemas = ReadSchema(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
         <ECSchema schemaName="ChangeSets" alias="cset" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="ECDbChange" version="01.00.00" alias="change"/>
          <ECEntityClass typeName="ChangeSet" modifier="Sealed">
              <ECNavigationProperty propertyName="Summary" relationshipName="ChangeSummaryExtractedFromChangeSet" direction="Backward"/>
              <ECProperty propertyName="ChangeSetHubId" typeName="string" />
              <ECProperty propertyName="PushDate" typeName="dateTime" />
              <ECProperty propertyName="CreatedBy" typeName="string" />
          </ECEntityClass>
          <ECRelationshipClass typeName="ChangeSummaryExtractedFromChangeSet" modifier="Sealed" strength="referencing">
                <Source multiplicity="(0..1)" roleLabel="is extracted from" polymorphic="false">
                    <Class class="change:ChangeSummary"/>
                </Source>
                <Target multiplicity="(0..1)" roleLabel="refers to" polymorphic="false">
                    <Class class="ChangeSet"/>
                </Target>
           </ECRelationshipClass>
         </ECSchema>
             )xml");

    changeCacheFile.Schemas().ImportSchemas(customSchemas);
    changeCacheFile.SaveChanges();
    changeCacheFile.CloseDb();

    //***Step 2: Add additional information to extracted change summary
    
    //retrieve changeset and additional information from elsewhere (using a place holder function to keep the example simple) 
    Utf8CP changeSetHubId = "ec1efd72621d42a0642bf135bdca409e94a454ed";
    DateTime pushDate;
    Utf8String createdBy;
    BeSQLite::IChangeSet& changeSet = GetChangeSetInfoFromHub(pushDate, createdBy, changeSetHubId);

    changeCacheFile.OpenBeSQLiteDb(changeCacheFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite));

    //extract the change summary
    ECInstanceKey changeSummaryKey;
    ECDb::ExtractChangeSummary(changeSummaryKey, changeCacheFile, primaryECDb, ChangeSetArg(changeSet));

    //add additional information and relate it to the new change summary
    ECSqlStatement insertChangeSetStmt;
    insertChangeSetStmt.Prepare(changeCacheFile, "INSERT INTO cset.ChangeSet(Summary,ChangeSetHubId,PushDate,CreatedBy) VALUES(?,?,?,?)");
    insertChangeSetStmt.BindNavigationValue(1, changeSummaryKey.GetInstanceId());
    insertChangeSetStmt.BindText(2, changeSetHubId, IECSqlBinder::MakeCopy::No);
    insertChangeSetStmt.BindDateTime(3, pushDate);
    insertChangeSetStmt.BindText(4, createdBy.c_str(), IECSqlBinder::MakeCopy::No);
    insertChangeSetStmt.Step();
    insertChangeSetStmt.ClearBindings();
    insertChangeSetStmt.Reset();

    changeCacheFile.SaveChanges();
    changeCacheFile.CloseDb();

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
