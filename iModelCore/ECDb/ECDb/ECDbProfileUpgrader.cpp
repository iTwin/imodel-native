/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//*************************************** ECDbProfileSchemaUpgrader *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        07/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ECDbProfileECSchemaUpgrader::ImportProfileSchemas(ECDbCR ecdb)
    {
    PERFLOG_START("ECDb", "profile schema import");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    if (SUCCESS != ReadECDbSystemSchema(*context, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    SchemaKey schemaKey("ECDbFileInfo", 2, 0, 0);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    schemaKey = SchemaKey("MetaSchema", 4, 0, 0);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    //import if already existing
    BentleyStatus importStat = ecdb.Schemas().ImportECSchemas(context->GetCache().GetSchemas(), ecdb.GetECDbImplR().GetSettings().GetECSchemaImportToken());
    PERFLOG_FINISH("ECDb", "profile schema import");
    if (importStat != SUCCESS)
        {
        LOG.errorv("Creating / upgrading ECDb file failed because importing the ECDb standard ECSchemas and the MetaSchema ECSchema into the file '%s' failed.",
                   ecdb.GetDbFileName());
        return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECDbProfileECSchemaUpgrader::ReadECDbSystemSchema(ECSchemaReadContextR readContext, Utf8CP ecdbFileName)
    {
    ECSchemaPtr ecdbSystemSchema = nullptr;
    const SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlString(ecdbSystemSchema, GetECDbSystemECSchemaXml(), readContext);
    if (SchemaReadStatus::Success != deserializeStat)
        {
        if (SchemaReadStatus::ReferencedSchemaNotFound == deserializeStat)
            LOG.errorv("Creating / upgrading ECDb file %s failed because required standard ECSchemas could not be found.", ecdbFileName);
        else
            {
            //other error codes are considered programmer errors and therefore have an assertion, too
            LOG.errorv("Creating / upgrading ECDb file %s failed because ECDbSystem ECSchema could not be deserialized. Error code SchemaReadStatus::%d", ecdbFileName, Enum::ToInt(deserializeStat));
            BeAssert(false && "ECDb upgrade: Failed to deserialize ECDbSystem ECSchema");
            }

        return ERROR;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ECDbProfileECSchemaUpgrader::ReadSchemaFromDisk(ECSchemaReadContextR readContext, SchemaKey& schemaKey, Utf8CP ecdbFileName)
    {
    ECSchemaPtr schema = readContext.LocateSchema(schemaKey, SchemaMatchType::LatestWriteCompatible);
    if (schema == nullptr)
        {
        LOG.errorv("Creating / upgrading ECDb file %s failed because required ECSchema '%s' could not be found.", ecdbFileName,
                   schemaKey.GetFullSchemaName().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+-
//static
Utf8CP ECDbProfileECSchemaUpgrader::GetECDbSystemECSchemaXml()
    {
    return "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='" ECSCHEMA_ECDbSystem "' alias='" ECSCHEMA_ALIAS_ECDbSystem "' description='Helper ECSchema for ECDb internal purposes.' version='5.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' /> "
        "    <ECEntityClass typeName='" ECDBSYS_CLASS_ClassECSqlSystemProperties "' modifier='Abstract' description='Defines the ECSQL system properties of an ECClass in an ECSQL statement.'>"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_ECInstanceId "' typeName='long' description='Represents the ECInstanceId system property in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_ECClassId "' typeName='long' readOnly='True' description='Represents the ECClassId system property in ECSQL.' />"
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='" ECDBSYS_CLASS_RelationshipECSqlSystemProperties "' modifier='Abstract' description='Defines the ECSQL system properties of an ECRelationshipClass in an ECSQL statement.'>"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_SourceECInstanceId "' typeName='long' description='Represents the SourceECInstanceId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_SourceECClassId "' typeName='long' description='Represents the SourceECClassId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_TargetECInstanceId "' typeName='long' description='Represents the TargetECInstanceId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_TargetECClassId "' typeName='long' description='Represents the TargetECClassId system property of an ECRelationship in ECSQL.' />"
        "    </ECEntityClass> "
        "    <ECStructClass typeName='" ECDBSYS_CLASS_PointECSqlSystemProperties "' modifier='Abstract' description='Represents the ECSQL data type of a Point property in an ECSQL statement.'>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointX "' typeName='double' description='Represents the X component of Point2d and Point3d in ECSQL' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointY "' typeName='double' description='Represents the Y component of Point2d and Point3d in ECSQL' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointZ "' typeName='double' description='Represents the Z component of Point3d in ECSQL' />"
        "    </ECStructClass> "
        "    <ECStructClass typeName='" ECDBSYS_CLASS_NavigationECSqlSystemProperties "' modifier='Abstract' description='Represents the ECSQL data type of a navigation property in an ECSQL statement.'>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_NavPropId "' typeName='long' description='Represents the Id system property of an NavigationProperty in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_NavPropRelECClassId "' typeName='long' description='Represents the Relationship ClassId system property of an NavigationProperty in ECSQL.' />"
        "    </ECStructClass> "
        "</ECSchema>";
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

