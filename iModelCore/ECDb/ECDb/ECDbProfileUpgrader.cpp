/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    StopWatch timer(true);
    auto context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    if (SUCCESS != ReadECDbSystemSchema(*context, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    SchemaKey schemaKey("ECDb_FileInfo", 2, 0, 0);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    schemaKey = SchemaKey("MetaSchema", 3, 1, 0);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    //import if already existing
    BentleyStatus importStat = ecdb.Schemas().ImportECSchemas(context->GetCache().GetSchemas(), ecdb.GetECDbImplR().GetTokenManager().GetDbSchemaModificationToken());
    timer.Stop();
    if (importStat != SUCCESS)
        {
        LOG.errorv("Creating / upgrading ECDb file failed because importing the ECDb standard ECSchemas and the MetaSchema ECSchema into the file '%s' failed.",
                   ecdb.GetDbFileName());
        return BE_SQLITE_ERROR;
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        LOG.debugv("Imported ECDb system ECSchemas and MetaSchema ECSchema into the file '%s' in %.4f msecs.",
                   ecdb.GetDbFileName(),
                   timer.GetElapsedSeconds() * 1000.0);
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
            LOG.errorv("Creating / upgrading ECDb file %s failed because ECDb_System ECSchema could not be deserialized. Error code SchemaReadStatus::%d", ecdbFileName, Enum::ToInt(deserializeStat));
            BeAssert(false && "ECDb upgrade: Failed to deserialize ECDb_System ECSchema");
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
        "<ECSchema schemaName='ECDb_System' alias='ecdbsys' version='3.0.3' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' /> "
        "    <ECEntityClass typeName='PrimitiveArray' modifier='Abstract'> "
        "        <ECCustomAttributes> "
        "            <ClassMap xmlns='ECDbMap.02.00.00'> "
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap> "
        "        </ECCustomAttributes> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='BinaryArray' modifier='Sealed'> "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='binary'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='BooleanArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='boolean'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='DateTimeArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='dateTime'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='DoubleArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='double'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='IntegerArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='int'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='GeometryArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='Bentley.Geometry.Common.IGeometry'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='LongArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='long'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='Point2dArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='point2d'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='Point3dArray' modifier='Sealed'> "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='point3d'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='StringArray' modifier='Sealed' > "
        "        <BaseClass>PrimitiveArray</BaseClass> "
        "        <ECArrayProperty propertyName='Array' typeName='string'/> "
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='ECSqlSystemProperties' modifier='Abstract' >"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='ECInstanceId' typeName='long' description='Represents the ECInstanceId system property used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='ECClassId' typeName='long' readOnly='True' description='Represents the ECClassId system property used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECInstanceId' typeName='long' description='Represents the SourceECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='SourceECClassId' typeName='long' description='Represents the SourceECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECInstanceId' typeName='long' description='Represents the TargetECInstanceId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='TargetECClassId' typeName='long' description='Represents the TargetECClassId system property of an ECRelationship used by the EC->DB Mapping.' />"
        "        <ECProperty propertyName='Id' typeName='long' description='Represents the Id system property of an NavigationPropertyMap' />"
        "        <ECProperty propertyName='RelECClassId' typeName='long' description='Represents the Relationship ClassId system property of an NavigationPropertyMap' />"
        "        <ECProperty propertyName='X' typeName='double' description='Represents the X component of Point2d and Point3d' />"
        "        <ECProperty propertyName='Y' typeName='double' description='Represents the Y component of Point2d and Point3d' />"
        "        <ECProperty propertyName='Z' typeName='double' description='Represents the Z component of Point3d' />"
        "    </ECEntityClass> "
        "</ECSchema>";
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

