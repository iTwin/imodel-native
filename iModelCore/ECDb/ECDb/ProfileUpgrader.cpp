/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <json/json.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************** ProfileUpgrader_XXX *********************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
DbResult ProfileUpgrader_4002::_Upgrade(ECDbCR ecdb) const
    {
    DbResult stat = ecdb.ExecuteDdl("ALTER TABLE " TABLE_Schema " ADD COLUMN OriginalECXmlVersionMajor INTEGER");
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

    stat = ecdb.ExecuteDdl(TABLEDDL_UnitSystem);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_UnitSystem " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteDdl(TABLEDDL_Phenomenon);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_Phenomenon " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteDdl(TABLEDDL_Unit);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_Unit " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteDdl(TABLEDDL_Format);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_Format " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    stat = ecdb.ExecuteDdl(TABLEDDL_FormatCompositeUnit);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("ECDb profile upgrade failed: Could not create table " TABLE_FormatCompositeUnit " and indexes: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Added columns OriginalECXmlVersionMajor and OriginalECXmlVersionMinor to table " TABLE_Schema ". Added tables " TABLE_Unit ", " TABLE_Phenomenon ", " TABLE_UnitSystem ", " TABLE_Format ", and " TABLE_FormatCompositeUnit ".");

    stat = FixMetaSchemaClassMapCAXml(ecdb);
    if (BE_SQLITE_OK != stat)
        return stat;

    stat = UpgradeECEnums(ecdb);
    if (BE_SQLITE_OK != stat)
        return stat;

    return UpgradeKoqs(ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileUpgrader_4002::UpgradeECEnums(ECDbCR ecdb)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT Id,Name,EnumValues FROM main." TABLE_Enumeration))
        {
        LOG.errorv("ECDb profile upgrade failed: Could not retrieve existing enumerations: %s.", ecdb.GetLastError().c_str());
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    bmap<int64_t, Utf8String> enumValues;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        int64_t enumId = stmt.GetValueInt64(0);
        Utf8CP enumName = stmt.GetValueText(1);
        Json::Value enumValuesJson;
        if (!Json::Reader::Parse(stmt.GetValueText(2), enumValuesJson))
            {
            LOG.errorv("ECDb profile upgrade failed: Could not parse ECEnumeration values JSON: %s.", enumName);
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

    for (auto const& kvPair : enumValues)
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
    stmt.Finalize();
    LOG.debug("ECDb profile upgrade: Updated table " TABLE_Enumeration " to EC3.2 format.");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileUpgrader_4002::UpgradeKoqs(ECDbCR ecdb)
    {
    KoqConversionContext ctx(ecdb);

    if (SUCCESS != ConvertKoqFuses(ctx))
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    if (ctx.m_schemasToReferenceUnits.empty())
        {
        BeAssert(!ctx.AreStandardSchemasDeserialized());
        LOG.debug("ECDb profile upgrade: Upgraded KindOfQuantities to EC3.2 format.");
        return BE_SQLITE_OK;
        }

    BeAssert(ctx.m_unitsSchema != nullptr);
    ECSchemaId unitsSchemaId = InsertSchemaStub(ecdb, ctx.m_unitsSchema->GetName(), ctx.m_unitsSchema->GetAlias());
    if (!unitsSchemaId.IsValid())
        {
        LOG.error("ECDb profile upgrade failed: Creating Units ECSchema stub failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    ECSchemaId formatsSchemaId;
    if (ctx.NeedsToImportFormatSchema())
        {
        BeAssert(ctx.m_formatsSchema != nullptr);
        formatsSchemaId = InsertSchemaStub(ecdb, ctx.m_formatsSchema->GetName(), ctx.m_formatsSchema->GetAlias());
        if (!formatsSchemaId.IsValid())
            {
            LOG.error("ECDb profile upgrade failed: Creating Formats ECSchema stub failed.");
            return BE_SQLITE_ERROR_ProfileUpgradeFailed;
            }
        }

    //now add the schema references
    if (SUCCESS != InsertReferencesToUnitsAndFormatsSchema(ctx, unitsSchemaId, formatsSchemaId))
        {
        LOG.error("ECDb profile upgrade failed: Creating schema references to new Units and Formats ECSchemas failed.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }


    //now import Units (and Formats schemas) which technically amounts to a schema upgrade of the previously inserted units and formats schema stubs
    if (SUCCESS != ImportFullUnitsAndFormatsSchemas(ctx))
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;

    LOG.debug("ECDb profile upgrade: Upgraded KindOfQuantities to EC3.2 format.");
    return BE_SQLITE_OK;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ProfileUpgrader_4002::ConvertKoqFuses(KoqConversionContext& ctx)
    {
    struct UpgradedUnitFormatStrings
        {
        Utf8String m_persistenceUnit;
        bvector<Utf8String> m_presentationFormats;
        };

    bmap<KindOfQuantityId, UpgradedUnitFormatStrings> koqs;

    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ctx.m_ecdb, "SELECT Id,SchemaId,PersistenceUnit,PresentationUnits,SchemaId FROM " TABLE_KindOfQuantity))
        {
        LOG.errorv("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed: %s.", ctx.m_ecdb.GetLastError().c_str());
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (!ctx.AreStandardSchemasDeserialized())
            {
            if (SUCCESS != DeserializeUnitsAndFormatsSchemas(ctx))
                return ERROR;
            }

        BeAssert(ctx.AreStandardSchemasDeserialized());
        UpgradedUnitFormatStrings& unitFormatStrings = koqs[stmt.GetValueId<KindOfQuantityId>(0)];
        ECSchemaId schemaId = stmt.GetValueId<ECSchemaId>(1);
        Json::Value oldPresFusesJson;
        bvector<Utf8CP> oldPresFuses; //can use Utf8CP as the string is owned by the Json::Value (-> Json::Value must not be moved into the if statement)
        if (!stmt.IsColumnNull(3))
            {
            if (!Json::Reader::Parse(stmt.GetValueText(3), oldPresFusesJson))
                {
                LOG.error("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed. Could not parse the old presentation units.");
                return ERROR;
                }

            for (Json::Value const& presFus : oldPresFusesJson)
                {
                oldPresFuses.push_back(presFus.asCString());
                }
            }

        BeAssert(ctx.m_formatsSchema != nullptr);
        if (ECObjectsStatus::Success != KindOfQuantity::UpdateFUSDescriptors(unitFormatStrings.m_persistenceUnit, unitFormatStrings.m_presentationFormats, stmt.GetValueText(2), oldPresFuses, *ctx.m_formatsSchema, *ctx.m_unitsSchema))
            {
            LOG.errorv("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed: %s.", ctx.m_ecdb.GetLastError().c_str());
            return ERROR;
            }

        ctx.m_schemasToReferenceUnits.insert(schemaId);
        if (!unitFormatStrings.m_presentationFormats.empty())
            ctx.m_schemasToReferenceFormats.insert(schemaId);

        }

    stmt.Finalize();
    if (koqs.empty())
        return SUCCESS;

    if (BE_SQLITE_OK != stmt.Prepare(ctx.m_ecdb, "UPDATE " TABLE_KindOfQuantity " SET PersistenceUnit=?,PresentationUnits=? WHERE Id=?"))
        {
        LOG.errorv("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed: %s.", ctx.m_ecdb.GetLastError().c_str());
        return ERROR;
        }

    for (auto const& kvPair : koqs)
        {
        if (BE_SQLITE_OK != stmt.BindText(1, kvPair.second.m_persistenceUnit, Statement::MakeCopy::No))
            {
            LOG.errorv("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed: %s.", ctx.m_ecdb.GetLastError().c_str());
            return ERROR;
            }

        // Presentation Format list must be turned into a JSON array
        const bool hasPresentationFormats = !kvPair.second.m_presentationFormats.empty();
        if (hasPresentationFormats)
            {
            Utf8String presFormatJson;
            if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationFormats(presFormatJson, kvPair.second.m_presentationFormats))
                {
                LOG.errorv("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed: %s.", ctx.m_ecdb.GetLastError().c_str());
                return ERROR;
                }

            if (BE_SQLITE_OK != stmt.BindText(2, presFormatJson, Statement::MakeCopy::Yes))
                {
                LOG.errorv("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed: %s.", ctx.m_ecdb.GetLastError().c_str());
                return ERROR;
                }
            }

        if (BE_SQLITE_OK != stmt.BindId(3, kvPair.first) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            LOG.errorv("ECDb profile upgrade failed: Upgrading persistence unit and presentation formats in ec_KindOfQuantity to EC3.2 format failed: %s.", ctx.m_ecdb.GetLastError().c_str());
            return ERROR;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
ECSchemaId ProfileUpgrader_4002::InsertSchemaStub(ECDbCR ecdb, Utf8StringCR schemaName, Utf8StringCR alias)
    {
    ECSchemaPtr stub = nullptr;
    if (ECObjectsStatus::Success != ECSchema::CreateSchema(stub, schemaName, alias, 1, 0, 0))
        return ECSchemaId();

    stub->SetOriginalECXmlVersion(3, 2);
    if (SUCCESS != SchemaWriter::InsertSchemaEntry(ecdb, *stub))
        return ECSchemaId();

    BeAssert(stub->HasId());
    return stub->GetId();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ProfileUpgrader_4002::InsertReferencesToUnitsAndFormatsSchema(KoqConversionContext& ctx, ECSchemaId unitsSchemaId, ECSchemaId formatsSchemaId)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ctx.m_ecdb, "INSERT INTO main." TABLE_SchemaReference "(SchemaId,ReferencedSchemaId) VALUES(?,?)"))
        {
        LOG.error("ECDb profile upgrade failed: Failed to prepare SQL to insert references to standard Units schema.");
        return ERROR;
        }

    for (ECSchemaId schemaThatNeedUnitsReference : ctx.m_schemasToReferenceUnits)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, schemaThatNeedUnitsReference) ||
            BE_SQLITE_OK != stmt.BindId(2, unitsSchemaId) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            LOG.error("ECDb profile upgrade failed: Failed to insert references to standard Units schema.");
            return ERROR;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    for (ECSchemaId schemaThatNeedFormatsReference : ctx.m_schemasToReferenceFormats)
        {
        if (BE_SQLITE_OK != stmt.BindId(1, schemaThatNeedFormatsReference) ||
            BE_SQLITE_OK != stmt.BindId(2, formatsSchemaId) ||
            BE_SQLITE_DONE != stmt.Step())
            {
            LOG.error("ECDb profile upgrade failed: Failed to insert references to standard Formats schema.");
            return ERROR;
            }

        stmt.Reset();
        stmt.ClearBindings();
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ProfileUpgrader_4002::ImportFullUnitsAndFormatsSchemas(KoqConversionContext& ctx)
    {
    bvector<ECSchemaCP> schemas;
    schemas.push_back(ctx.m_unitsSchema.get());
    if (ctx.NeedsToImportFormatSchema())
        schemas.push_back(ctx.m_formatsSchema.get());

    if (SUCCESS != ctx.m_ecdb.Schemas().ImportSchemas(schemas, ctx.m_ecdb.GetImpl().GetSettingsManager().GetSchemaImportToken()))
        {
        LOG.error("ECDb profile upgrade failed: Importing standard Units and Formats ECSchemas failed.");
        return ERROR;
        }

    ctx.m_ecdb.ClearECDbCache();
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus ProfileUpgrader_4002::DeserializeUnitsAndFormatsSchemas(KoqConversionContext& ctx)
    {
    ECSchemaReadContextPtr readCtx = ECSchemaReadContext::CreateContext();
    readCtx->AddSchemaLocater(ctx.m_ecdb.GetSchemaLocater());
    //Formats schema references Units schema. We can just locate Formats and it will deserialize Units as well
    SchemaKey formatsSchemaKey(FormatsSchemaName, 1, 0, 0);
    ctx.m_formatsSchema = ECSchema::LocateSchema(formatsSchemaKey, *readCtx);
    if (ctx.m_formatsSchema == nullptr)
        {
        LOG.error("ECDb profile upgrade failed: Deserializing standard Units and Formats ECSchemas failed.");
        return ERROR;
        }

    BeAssert(readCtx->GetCache().GetCount() == 2);
    SchemaKey unitsSchemaKey(UnitsSchemaName, 1, 0, 0);
    auto it = ctx.m_formatsSchema->GetReferencedSchemas().Find(unitsSchemaKey, SchemaMatchType::LatestReadCompatible);
    if (it == ctx.m_formatsSchema->GetReferencedSchemas().end())
        {
        LOG.error("ECDb profile upgrade failed: Deserializing standard Units and Formats ECSchemas failed.");
        return ERROR;
        }

    ctx.m_unitsSchema = it->second;
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileUpgrader_4002::FixMetaSchemaClassMapCAXml(ECDbCR ecdb)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT ca.Id, ca.Instance FROM main." TABLE_CustomAttribute " ca "
                                     "JOIN main." TABLE_Class " caClass ON caClass.Id = ca.ClassId "
                                     "JOIN main." TABLE_Schema " caSchema ON caSchema.Id = caClass.SchemaId "
                                     "JOIN main." TABLE_Class " containerClass ON containerClass.id = ca.ContainerId "
                                     "JOIN main." TABLE_Schema " containerSchema ON containerSchema.Id = containerClass.SchemaId "
                                     "WHERE containerSchema.Name='ECDbMeta' AND containerClass.Name='ClassHasAllBaseClasses' AND "
                                     "caSchema.Name='ECDbMap' AND caClass.Name='ClassMap'") ||
        BE_SQLITE_ROW != stmt.Step())
        {
        LOG.error("ECDb profile upgrade failed: Failed to execute SQL to find malformed ClassMap CA XML on ECDbMeta.ClassHasAllBaseClasses.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    BeInt64Id caId = stmt.GetValueId<BeInt64Id>(0);
    Utf8String caXml(stmt.GetValueText(1));
    stmt.Finalize();

    const size_t mapStrategyStartPos = caXml.find("<MapStrategy>");
    const size_t mapStrategyEndPos = caXml.find("</MapStrategy>");
    caXml.replace(mapStrategyStartPos, mapStrategyEndPos - mapStrategyStartPos, "<MapStrategy>ExistingTable");
    BeAssert(caXml.Contains("<MapStrategy>ExistingTable</MapStrategy>"));
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "UPDATE main." TABLE_CustomAttribute " SET Instance=? WHERE Id=?") ||
        BE_SQLITE_OK != stmt.BindText(1, caXml, Statement::MakeCopy::No) ||
        BE_SQLITE_OK != stmt.BindId(2, caId) ||
        BE_SQLITE_DONE != stmt.Step())
        {
        LOG.error("ECDb profile upgrade failed: Failed to execute SQL to fix malformed ClassMap CA XML on ECDbMeta.ClassHasAllBaseClasses.");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    LOG.debug("ECDb profile upgrade: Fixed ClassMap custom attribute ECXML of MetaSchema.ClassHasAllBaseClasses.");
    return BE_SQLITE_OK;
    }

//******************************************************************************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
DbResult ProfileSchemaUpgrader::ImportProfileSchemas(ECDbCR ecdb)
    {
    ECDB_PERF_LOG_SCOPE("Profile schema import");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext(false, true);
    // Don't add ecdb as schema locater so that referenced schemas of the profile schemas
    // get a chance to be upgraded as well (in case there are newer versions of them in the assets folder)

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    if (SUCCESS != ReadECDbSystemSchema(*context, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    SchemaKey schemaKey("ECDbFileInfo", 2, 0, 1);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    schemaKey = SchemaKey("ECDbMeta", 4, 0, 2);
    if (SUCCESS != ReadSchemaFromDisk(*context, schemaKey, ecdb.GetDbFileName()))
        return BE_SQLITE_ERROR;

    //import if already existing
    if (SUCCESS != ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas(), ecdb.GetImpl().GetSettingsManager().GetSchemaImportToken()))
        {
        LOG.errorv("Creating / upgrading ECDb file failed because importing the ECDb standard ECSchemas into the file '%s' failed.", ecdb.GetDbFileName());
        return BE_SQLITE_ERROR;
        }

    // This sql update fixes an issue that occurs when upgrading profile version to 4002.
    // The profile upgrade code modifies values in memory, but is leaving the ExtendedTypeName value inconsistent between memory and DB for the property ECInstanceID in the table ec_Property.
    // So, we run this sql update to make the DB value consistent with the one in memory.
    if (BE_SQLITE_OK != ecdb.ExecuteSql("UPDATE main." TABLE_Property " SET ExtendedTypeName = '" EXTENDEDTYPENAME_Id "' WHERE Id = (select p.Id from main." TABLE_Property " p join main." TABLE_Class " c on c.id = p.ClassId join main." TABLE_Schema " s on s.id = c.SchemaId where s.Name = '" ECSCHEMA_ECDbSystem "' and c.Name = '" ECDBSYS_CLASS_ClassECSqlSystemProperties "' and p.Name = '" ECDBSYS_PROP_ECInstanceId "')"))
        {
        LOG.errorv("ECDb profile upgrade failed with error: Failed to update ExtendedTypeName value for property '" ECDBSYS_PROP_ECInstanceId "' in table " TABLE_Property ".");
        return BE_SQLITE_ERROR_ProfileUpgradeFailed;
        }

    PERFLOG_FINISH("ECDb", "Profile schema import");
    return BE_SQLITE_OK;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
        "        <ECProperty propertyName='" ECDBSYS_PROP_ECInstanceId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' description='Represents the Id system property in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_ECClassId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' readOnly='True' description='Represents the ECClassId system property in ECSQL.' />"
        "    </ECEntityClass> "
        "    <ECEntityClass typeName='" ECDBSYS_CLASS_RelationshipECSqlSystemProperties "' modifier='Abstract' description='Defines the ECSQL system properties of an ECRelationshipClass in an ECSQL statement.'>"
        "       <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_SourceECInstanceId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' description='Represents the SourceId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_SourceECClassId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' description='Represents the SourceECClassId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_TargetECInstanceId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' description='Represents the TargetId system property of an ECRelationship in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_TargetECClassId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' description='Represents the TargetECClassId system property of an ECRelationship in ECSQL.' />"
        "    </ECEntityClass> "
        "    <ECStructClass typeName='" ECDBSYS_CLASS_PointECSqlSystemProperties "' modifier='Abstract' description='Represents the ECSQL data type of a Point property in an ECSQL statement.'>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointX "' typeName='double' description='Represents the X component of Point2d and Point3d in ECSQL' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointY "' typeName='double' description='Represents the Y component of Point2d and Point3d in ECSQL' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_PointZ "' typeName='double' description='Represents the Z component of Point3d in ECSQL' />"
        "    </ECStructClass> "
        "    <ECStructClass typeName='" ECDBSYS_CLASS_NavigationECSqlSystemProperties "' modifier='Abstract' description='Represents the ECSQL data type of a navigation property in an ECSQL statement.'>"
        "        <ECProperty propertyName='" ECDBSYS_PROP_NavPropId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' description='Represents the Id system property of an NavigationProperty in ECSQL.' />"
        "        <ECProperty propertyName='" ECDBSYS_PROP_NavPropRelECClassId "' typeName='long' extendedTypeName='" EXTENDEDTYPENAME_Id "' description='Represents the Relationship ClassId system property of an NavigationProperty in ECSQL.' />"
        "    </ECStructClass> "
        "</ECSchema>";
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

