/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ProfileUpgrader.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <json/json.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************** ProfileUpgrader_XXX *********************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    01/2018
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ProfileUpgrader_4002::_Upgrade(ECDbCR ecdb) const
    {
    DbResult stat = ecdb.ExecuteDdl("ALTER TABLE " TABLE_Schema " ADD COLUMN ECVersion INTEGER");
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not add column ECVersion to table " TABLE_Schema ": %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteDdl("ALTER TABLE " TABLE_Schema " ADD COLUMN OriginalECXmlVersionMajor INTEGER");
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not add column OriginalECXmlVersionMajor to table " TABLE_Schema ": %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteDdl("ALTER TABLE " TABLE_Schema " ADD COLUMN OriginalECXmlVersionMinor INTEGER");
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not add column OriginalECXmlVersionMinor to table " TABLE_Schema ": %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteSql(TABLEDDL_UnitSystem);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_UnitSystem " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }
    
    stat = ecdb.ExecuteSql(TABLEDDL_Phenomenon);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_Phenomenon " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteSql(TABLEDDL_Unit);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_Unit " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Added columns ECVersion, OriginalECXmlVersionMajor and OriginalECXmlVersionMinor to table " TABLE_Schema ". Added tables " TABLE_Unit ", " TABLE_Phenomenon ", and " TABLE_UnitSystem ".");

    stat = UpgradeECEnums(ecdb);
    if (BE_SQLITE_OK != stat)
        return stat;

    return UpgradeKoqs(ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    01/2018
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileUpgrader_4002::UpgradeECEnums(ECDbCR ecdb)
    {
    bset<ECSchemaId> ecdbSchemas;
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Id FROM main." TABLE_Schema " WHERE Name IN ('ECDbChange', 'ECDbFileInfo', 'ECDbMeta')"))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not retrieve existing enumerations: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        ecdbSchemas.insert(stmt.GetValueId<ECSchemaId>(0));
        }
    }

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Id,SchemaId,Name,EnumValues FROM main." TABLE_Enumeration))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not retrieve existing enumerations: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    bmap<int64_t, Utf8String> enumValues;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        int64_t enumId = stmt.GetValueInt64(0);
        Utf8CP enumName = stmt.GetValueText(2);
        if (ecdbSchemas.find(stmt.GetValueId<ECSchemaId>(1)) != ecdbSchemas.end())
            {
            UpgradeECDbEnum(enumValues, enumId, enumName);
            continue;
            }

        Json::Value enumValuesJson;
        if (!Json::Reader::Parse(stmt.GetValueText(3), enumValuesJson))
            {
            LOG.errorv("ECDb profile upgrade failed: Could not parse ECEnumeration values JSON: %s.", stmt.GetValueText(1));
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }


        // now upgrade the enum values json
        for (Json::Value& enumValueJson : enumValuesJson)
            {
            Utf8CP strVal = nullptr;
            Nullable<int32_t> intVal;
            if (enumValueJson.isMember(ECDBMETA_PROP_ECEnumerator_StringValue))
                strVal = enumValueJson[ECDBMETA_PROP_ECEnumerator_StringValue].asCString();
            else if (enumValueJson.isMember(ECDBMETA_PROP_ECEnumerator_IntValue))
                intVal = (int32_t) enumValueJson[ECDBMETA_PROP_ECEnumerator_IntValue].asInt();
            else
                {
                BeAssert(false);
                return BE_SQLITE_ERROR_ProfileUpgradeFailed;
                }

            enumValueJson[ECDBMETA_PROP_ECEnumerator_Name] = ECEnumerator::DetermineName(enumName, strVal, intVal.IsValid() ? &intVal.Value() : nullptr);
            }

        enumValues[enumId] = enumValuesJson.ToString();
        }

    stmt.Finalize();
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "UPDATE main." TABLE_Enumeration " SET EnumValues=? WHERE Id=?"))
        {
        LOG.errorv("ECDb profile upgrade failed: Failed to update the table " TABLE_Enumeration ": %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    for (bpair<int64_t, Utf8String> kvPair : enumValues)
        {
        if (BE_SQLITE_OK != stmt.BindText(1, kvPair.second, Statement::MakeCopy::No) ||
            BE_SQLITE_OK != stmt.BindInt64(2, kvPair.first) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            LOG.errorv("ECDb profile upgrade failed: Failed to update the table " TABLE_Enumeration ": %s.", ecdb.GetLastError().c_str());
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    LOG.debug("ECDb profile upgrade: Updated table " TABLE_Enumeration " to EC3.2 format.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    01/2018
//+---------------+---------------+---------------+---------------+---------------+--------
//static
void ProfileUpgrader_4002::UpgradeECDbEnum(bmap<int64_t, Utf8String>& enumMap, int64_t enumId, Utf8CP enumName)
    {
    if (BeStringUtilities::StricmpAscii(enumName, "StandardRootFolderType") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"DocumentsFolder\", \"IntValue\":0, \"DisplayLabel\":\"DocumentsFolder\"},"
                           "{\"Name\":\"TemporaryFolder\", \"IntValue\":1, \"DisplayLabel\":\"TemporaryFolder\"},"
                           "{\"Name\":\"CachesFolder\", \"IntValue\":2, \"DisplayLabel\":\"CachesFolder\"},"
                           "{\"Name\":\"LocalStateFolder\", \"IntValue\":3, \"DisplayLabel\":\"LocalStateFolder\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "OpCode") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Insert\", \"IntValue\":1, \"DisplayLabel\":\"Insert\"},"
                           "{\"Name\":\"Update\", \"IntValue\":2, \"DisplayLabel\":\"Update\"},"
                           "{\"Name\":\"Delete\", \"IntValue\":4, \"DisplayLabel\":\"Delete\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "ECCustomAttributeContainerType") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Schema\", \"IntValue\":1, \"DisplayLabel\":\"Schema\"},"
            "{\"Name\":\"EntityClass\", \"IntValue\":2, \"DisplayLabel\":\"EntityClass\"},"
            "{\"Name\":\"CustomAttributeClass\", \"IntValue\":4, \"DisplayLabel\":\"CustomAttributeClass\"},"
            "{\"Name\":\"StructClass\", \"IntValue\":8, \"DisplayLabel\":\"StructClass\"},"
            "{\"Name\":\"RelationshipClass\", \"IntValue\":16, \"DisplayLabel\":\"RelationshipClass\"},"
            "{\"Name\":\"AnyClass\", \"IntValue\":30, \"DisplayLabel\":\"AnyClass\"},"
            "{\"Name\":\"PrimitiveProperty\", \"IntValue\":32, \"DisplayLabel\":\"PrimitiveProperty\"},"
            "{\"Name\":\"StructProperty\", \"IntValue\":64, \"DisplayLabel\":\"StructProperty\"},"
            "{\"Name\":\"PrimitiveArrayProperty\", \"IntValue\":128, \"DisplayLabel\":\"PrimitiveArrayProperty\"},"
            "{\"Name\":\"StructArrayProperty\", \"IntValue\":256, \"DisplayLabel\":\"StructArrayProperty\"},"
            "{\"Name\":\"NavigationProperty\", \"IntValue\":512, \"DisplayLabel\":\"NavigationProperty\"},"
            "{\"Name\":\"AnyProperty\", \"IntValue\":992, \"DisplayLabel\":\"AnyProperty\"},"
            "{\"Name\":\"SourceRelationshipConstraint\", \"IntValue\":1024, \"DisplayLabel\":\"SourceRelationshipConstraint\"},"
            "{\"Name\":\"TargetRelationshipConstraint\", \"IntValue\":2048, \"DisplayLabel\":\"TargetRelationshipConstraint\"},"
            "{\"Name\":\"AnyRelationshipConstraint\", \"IntValue\":3072, \"DisplayLabel\":\"AnyRelationshipConstraint\"},"
            "{\"Name\":\"Any\", \"IntValue\":4095, \"DisplayLabel\":\"Any\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "ECClassModifier") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"None\", \"IntValue\":0, \"DisplayLabel\":\"None\"},"
            "{\"Name\":\"Abstract\", \"IntValue\":1, \"DisplayLabel\":\"Abstract\"},"
            "{\"Name\":\"Sealed\", \"IntValue\":2, \"DisplayLabel\":\"Sealed\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "ECRelationshipEnd") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Source\", \"IntValue\":0, \"DisplayLabel\":\"Source\"},"
                           "{\"Name\":\"Target\", \"IntValue\":1, \"DisplayLabel\":\"Target\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "ECClassType") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Entity\", \"IntValue\":0, \"DisplayLabel\":\"Entity\"},"
                            "{\"Name\":\"Relationship\", \"IntValue\":1, \"DisplayLabel\":\"Relationship\"},"
                            "{\"Name\":\"Struct\", \"IntValue\":2, \"DisplayLabel\":\"Struct\"},"
                            "{\"Name\":\"CustomAttribute\", \"IntValue\":3, \"DisplayLabel\":\"CustomAttribute\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "ECPropertyKind") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Primitive\", \"IntValue\":0, \"DisplayLabel\":\"Primitive\"},"
            "{\"Name\":\"Struct\", \"IntValue\":1, \"DisplayLabel\":\"Struct\"},"
        "{\"Name\":\"PrimitiveArray\", \"IntValue\":2, \"DisplayLabel\":\"PrimitiveArray\"},"
        "{\"Name\":\"StructArray\", \"IntValue\":3, \"DisplayLabel\":\"StructArray\"},"
        "{\"Name\":\"Navigation\", \"IntValue\":4, \"DisplayLabel\":\"Navigation\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "ECRelationshipDirection") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Forward\", \"IntValue\":1, \"DisplayLabel\":\"Forward\"},"
            "{\"Name\":\"Backward\", \"IntValue\":2, \"DisplayLabel\":\"Backward\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "ECRelationshipStrength") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Referencing\", \"IntValue\":0, \"DisplayLabel\":\"Referencing\"},"
            "{\"Name\":\"Holding\", \"IntValue\":1, \"DisplayLabel\":\"Holding\"},"
            "{\"Name\":\"Embedding\", \"IntValue\":2, \"DisplayLabel\":\"Embedding\"}]";
        return;
        }

    if (BeStringUtilities::StricmpAscii(enumName, "PrimitiveType") == 0)
        {
        enumMap[enumId] = "[{\"Name\":\"Binary\", \"IntValue\":257, \"DisplayLabel\":\"Binary\"},"
            "{\"Name\":\"Boolean\", \"IntValue\":513, \"DisplayLabel\":\"Boolean\"},"
            "{\"Name\":\"DateTime\", \"IntValue\":769, \"DisplayLabel\":\"DateTime\"},"
            "{\"Name\":\"Double\", \"IntValue\":1025, \"DisplayLabel\":\"Double\"},"
            "{\"Name\":\"Integer\", \"IntValue\":1281, \"DisplayLabel\":\"Integer\"},"
            "{\"Name\":\"Long\", \"IntValue\":1537, \"DisplayLabel\":\"Long\"},"
            "{\"Name\":\"Point2d\", \"IntValue\":1793, \"DisplayLabel\":\"Point2d\"},"
            "{\"Name\":\"Point3d\", \"IntValue\":2049, \"DisplayLabel\":\"Point3d\"},"
            "{\"Name\":\"String\", \"IntValue\":2305, \"DisplayLabel\":\"String\"},"
            "{\"Name\":\"IGeometry\", \"IntValue\":2561, \"DisplayLabel\":\"IGeometry\"}]";
        return;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    03/2018
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileUpgrader_4002::UpgradeKoqs(ECDbCR ecdb)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT distinct SchemaId FROM main." TABLE_KindOfQuantity))
        {
        LOG.errorv("ECDb profile upgrade failed: Selecting all KindOfQuantities failed: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    bset<ECSchemaId> schemasWithKoqs;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        schemasWithKoqs.insert(stmt.GetValueId<ECSchemaId>(0));
        }

    stmt.Finalize();

    if (schemasWithKoqs.empty())
        return BE_SQLITE_OK;

    ECSchemaId unitSchemaId;
    {
    ECSchemaReadContextPtr readCtx = ECSchemaReadContext::CreateContext();
    readCtx->AddSchemaLocater(ecdb.GetSchemaLocater());
    SchemaKey key("Units", 1, 0, 0);
    ECSchemaPtr unitSchema = ECSchema::LocateSchema(key, *readCtx);
    if (unitSchema == nullptr)
        {
        LOG.error("ECDb profile upgrade failed: Could not deserialize the standard Units ECSchema.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    bvector<ECSchemaCP> unitSchemaList;
    unitSchemaList.push_back(unitSchema.get());
    if (SUCCESS != ecdb.Schemas().ImportSchemas(unitSchemaList, ecdb.GetImpl().GetSettingsManager().GetSchemaImportToken()) ||
        !unitSchema->HasId())
        {
        LOG.error("ECDb profile upgrade failed: Importing the standard Units ECSchema failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    unitSchemaId = unitSchema->GetId();
    ecdb.ClearECDbCache();
    }

    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "INSERT INTO main." TABLE_SchemaReference "(SchemaId,ReferencedSchemaId) VALUES(?,?)"))
        {
        LOG.error("ECDb profile upgrade failed: Failed to prepare SQL to insert references to standard Units schema.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    for (ECSchemaId schemaWithKoq : schemasWithKoqs)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, schemaWithKoq) ||
            BE_SQLITE_OK != stmt.BindId(2, unitSchemaId) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            LOG.error("ECDb profile upgrade failed: Failed to insert references to standard Units schema.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    LOG.debug("ECDb profile upgrade: Upgraded KindOfQuantities to EC3.2 format.");
    return BE_SQLITE_OK;
    }

//******************************************************************************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    10/2017
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ProfileUpgrader_4001::_Upgrade(ECDbCR ecdb) const
    {
    if (BE_SQLITE_OK != ecdb.ExecuteSql("DELETE FROM main." BEDB_TABLE_Local " WHERE Name NOT LIKE 'ec_instanceidsequence' COLLATE NOCASE AND NAME LIKE 'ec_%sequence' COLLATE NOCASE"))
        {
        LOG.errorv("ECDb profile upgrade failed: Deleting ECDb profile table id sequences from table '" BEDB_TABLE_Local "' failed: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    const int actualModifiedRowCount = ecdb.GetModifiedRowCount();
    if (17 != actualModifiedRowCount)
        {
        LOG.errorv("ECDb profile upgrade failed: Expected to delete 17 ECDb profile table id sequences from table '" BEDB_TABLE_Local "'. %d were deleted though.", actualModifiedRowCount);
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Deleted ECDb profile table id sequences from table '" BEDB_TABLE_Local "'.");
    return BE_SQLITE_OK;
    }


//*************************************** ProfileSchemaUpgrader *********************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle        07/2012
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileSchemaUpgrader::ImportProfileSchemas(ECDbCR ecdb)
    {
    PERFLOG_START("ECDb", "Profile schema import");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    if (SUCCESS != ReadECDbSystemSchema(*context, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    SchemaKey schemaKey("ECDbFileInfo", 2, 0, 1);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    schemaKey = SchemaKey("ECDbMeta", 4, 0, 1);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    //import if already existing
    if (SUCCESS != ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas(), ecdb.GetImpl().GetSettingsManager().GetSchemaImportToken()))
        {
        LOG.errorv("Creating / upgrading ECDb file failed because importing the ECDb standard ECSchemas into the file '%s' failed.", ecdb.GetDbFileName());
        return BE_SQLITE_ERROR;
        }

    PERFLOG_FINISH("ECDb", "Profile schema import");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    10/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ProfileSchemaUpgrader::ReadECDbSystemSchema(ECSchemaReadContextR readContext, Utf8CP ecdbFileName)
    {
    ECSchemaPtr ecdbSystemSchema = nullptr;
    const SchemaReadStatus deserializeStat = ECSchema::ReadFromXmlString(ecdbSystemSchema, GetECDbSystemSchemaXml(), readContext);
    if (SchemaReadStatus::Success == deserializeStat)
        return SUCCESS;

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


//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    04/2016
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ProfileSchemaUpgrader::ReadSchemaFromDisk(ECSchemaReadContextR readContext, SchemaKey& schemaKey, Utf8CP ecdbFileName)
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
Utf8CP ProfileSchemaUpgrader::GetECDbSystemSchemaXml()
    {
    return "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='" ECSCHEMA_ECDbSystem "' alias='" ECSCHEMA_ALIAS_ECDbSystem "' description='Helper ECSchema for ECDb internal purposes.' version='5.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' /> "
        "    <ECEntityClass typeName='" ECDBSYS_CLASS_ClassECSqlSystemProperties "' modifier='Abstract' description='Defines the ECSQL system properties of an ECClass in an ECSQL statement.'>"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_ECInstanceId "' typeName='long' extendedTypeName='Id' description='Represents the Id system property in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_ECClassId "' typeName='long' extendedTypeName='Id' readOnly='True' description='Represents the ECClassId system property in ECSQL.' />"
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='" ECDBSYS_CLASS_RelationshipECSqlSystemProperties "' modifier='Abstract' description='Defines the ECSQL system properties of an ECRelationshipClass in an ECSQL statement.'>"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_SourceECInstanceId "' typeName='long' extendedTypeName='Id' description='Represents the SourceId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_SourceECClassId "' typeName='long' extendedTypeName='Id' description='Represents the SourceECClassId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_TargetECInstanceId "' typeName='long' extendedTypeName='Id' description='Represents the TargetId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_TargetECClassId "' typeName='long' extendedTypeName='Id' description='Represents the TargetECClassId system property of an ECRelationship in ECSQL.' />"
        "    </ECEntityClass> "
        "    <ECStructClass typeName='" ECDBSYS_CLASS_PointECSqlSystemProperties "' modifier='Abstract' description='Represents the ECSQL data type of a Point property in an ECSQL statement.'>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointX "' typeName='double' description='Represents the X component of Point2d and Point3d in ECSQL' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointY "' typeName='double' description='Represents the Y component of Point2d and Point3d in ECSQL' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointZ "' typeName='double' description='Represents the Z component of Point3d in ECSQL' />"
        "    </ECStructClass> "
        "    <ECStructClass typeName='" ECDBSYS_CLASS_NavigationECSqlSystemProperties "' modifier='Abstract' description='Represents the ECSQL data type of a navigation property in an ECSQL statement.'>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_NavPropId "' typeName='long' extendedTypeName='Id' description='Represents the Id system property of an NavigationProperty in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_NavPropRelECClassId "' typeName='long' extendedTypeName='Id' description='Represents the Relationship ClassId system property of an NavigationProperty in ECSQL.' />"
        "    </ECStructClass> "
        "</ECSchema>";
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

