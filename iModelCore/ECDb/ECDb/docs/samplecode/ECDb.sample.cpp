/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/docs/samplecode/ECDb.sample.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_EXTRACT_START__ Overview_ECDb_Include.sampleCode
// this includes the standard ECDb headers and its dependencies
#include <ECDb/ECDbApi.h>
//__PUBLISH_EXTRACT_END__

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateECDbFile ()
    {
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_CreateECDb.sampleCode

    // Initialize ECDb (Call this only once per process!)
    BeFileName tempDir (L"C:\\users\\someuser\\AppData\\Local\\Temp\\");
    BeFileName hostAssetsDir (L"C:\\appdata\\assets\\");
    ECDb::Initialize (tempDir, &hostAssetsDir);

    // Create ECDb file
    ECDb ecdb;
    DbResult r = ecdb.CreateNewDb ("C:\\data\\foo.ecdb");
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
BentleyStatus OpenCloseECDbConnection ()
    {
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_OpenCloseECDb.sampleCode

    // Initialize ECDb (Call this only once per process!)
    BeFileName tempDir (L"C:\\users\\someuser\\AppData\\Local\\Temp\\");
    BeFileName hostAssetsDir (L"C:\\appdata\\assets\\");
    ECDb::Initialize (tempDir, &hostAssetsDir);

    // Open ECDb file
    ECDb ecdb;
    DbResult r = ecdb.OpenBeSQLiteDb ("C:\\data\\foo.ecdb", ECDb::OpenParams (ECDb::OpenMode::Readonly, DefaultTxn::Yes));
    if (BE_SQLITE_OK != r)
        {
        // do error handling here...
        return ERROR;
        }

    // Work with the ECDb file

    // Close ECDb file
    ecdb.CloseDb ();

    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbImportSchemaFromECSchemaXml ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_ImportSchema.sampleCode
    // 1) Deserialize ECSchema from XML file
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    // Add the ECDb file's schema manager as schema locater so that all schemas already available
    // in the ECDb file can be reused
    schemaContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
    // add schema search paths for referenced schemas.
    // This is needed in case the schema references classes from other schemas in other locations.
    schemaContext->AddSchemaPath (L"C:\\schema\\references");

    // Read specified schema. All referenced schemas will also be read. Schema and references
    // are available in the cache of the schemaContext
    ECSchemaPtr schema = nullptr;
    SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlFile (schema, L"C:\\schema\\foo.ecschema.xml", *schemaContext);
    if (SCHEMA_READ_STATUS_Success != deserializeStat)
        {
        // Schema could not be read into memory. Do error handling here
        return ERROR;
        }

    // 2) Import ECSchema (and its references) into ECDb file. The schema and its references are available in the cache
    //    of the schema context from the XML deserialization step.
    ECDbSchemaManagerCR schemaManager = ecdb.Schemas ();
    BentleyStatus importStat = schemaManager.ImportECSchemas (schemaContext->GetCache ());
    if (SUCCESS != importStat)
        {
        // Schema(s) could not be imported into the ECDb file. Do error handling here
        ecdb.AbandonChanges ();
        return ERROR;
        }

    // commits the changes in the ECDb file (the imported ECSchema) to disk
    ecdb.SaveChanges ();
    //__PUBLISH_EXTRACT_END__
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/13
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaManagerGetSchema ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_SchemaManagerGetSchema.sampleCode

    // Get the schema manager
    ECDbSchemaManagerCR schemaManager = ecdb.Schemas ();

    // Retrieve a schema
    // (Passing false as third parameter indicates that classes of the schema should not be loaded along with the schema, 
    // but be loaded on demand)
    ECSchemaCP fooSchema = schemaManager.GetECSchema ("fooschema", false);
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
BentleyStatus ECDbSchemaManagerGetClass ()
    {
    ECDb ecdb;
    //__PUBLISH_EXTRACT_START__ Overview_ECDb_SchemaManagerGetClass.sampleCode

    // Get the schema manager
    ECDbSchemaManagerCR schemaManager = ecdb.Schemas ();

    // Retrieve an ECClass
    ECClassCP addressClass = schemaManager.GetECClass ("fooschema", "Address");
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

END_BENTLEY_SQLITE_EC_NAMESPACE
