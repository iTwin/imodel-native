/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define CHANGESUMMARIES_PROPSPEC_NAMESPACE "ec_ChangeSummaries"

//****************************************************************************
// ChangeSummaryManager
//****************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryManager::Extract(ECInstanceKey& summaryKey, BeSQLite::IChangeSet& changeset, ECDb::ChangeSummaryExtractOptions const& options) const
    {
    BeMutexHolder lock(m_ecdb.GetImpl().GetMutex());
    return m_extractor.Extract(summaryKey, const_cast<ChangeSummaryManager&>(*this), changeset, options);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::AttachChangeSummaryCacheFile(bool createIfNotExists) const
    {
    if (IsChangeSummaryCacheAttached())
        return BE_SQLITE_OK;

    BeFileName cachePath = DetermineCachePath(m_ecdb);
    if (!cachePath.DoesPathExist())
        {
        if (!createIfNotExists)
            return BE_SQLITE_OK;

        DbResult r = CreateCacheFile(cachePath);
        if (BE_SQLITE_OK != r)
            return r;
        }

    if (!cachePath.DoesPathExist())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    DbResult r = m_ecdb.AttachDb(cachePath.GetNameUtf8().c_str(), TABLESPACE_ChangeSummaries);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to attach ChangeSummaries cache file '%s': %s", cachePath.GetNameUtf8().c_str(), m_ecdb.GetLastError().c_str());
        return r;
        }

    return BE_SQLITE_OK;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::CreateCacheFile(BeFileNameCR cachePath) const
    {
    BeAssert(!IsChangeSummaryCacheAttached());

    if (cachePath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != cachePath.BeDeleteFile())
            {
            m_ecdb.GetImpl().Issues().Report("Failed to create ChangeSummaries cache file '%s'. The file already exists and could not be deleted.", cachePath.GetNameUtf8().c_str());
            return BE_SQLITE_ERROR;
            }

        LOG.warningv("A ChangeSummaries cache file '%s' already exists for the new ECDb file. The existing cache file has been deleted.", cachePath.GetNameUtf8().c_str());
        }

    ECDb cacheDb;
    DbResult r = cacheDb.CreateNewDb(cachePath);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new ChangeSummaries cache file '%s': %s", cachePath.GetNameUtf8().c_str(), Db::InterpretDbResult(r));
        cacheDb.AbandonChanges();
        return r;
        }

    //import ChangeSummaries schema
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(cacheDb.GetSchemaLocater());

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    ECN::SchemaKey schemaKey(ECSCHEMA_ECDbChangeSummaries, 1, 0, 0);
    if (context->LocateSchema(schemaKey, ECN::SchemaMatchType::LatestWriteCompatible) == nullptr)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new ChangeSummaries cache file '%s': Could not locate ECSchema " ECSCHEMA_ECDbChangeSummaries, cachePath.GetNameUtf8().c_str());
        cacheDb.AbandonChanges();
        return BE_SQLITE_ERROR;
        }

    if (SUCCESS != cacheDb.Schemas().ImportSchemas(context->GetCache().GetSchemas(), cacheDb.GetImpl().GetSettingsManager().GetSchemaImportToken()))
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new ChangeSummaries cache file '%s': Could not import ECSchema " ECSCHEMA_ECDbChangeSummaries, cachePath.GetNameUtf8().c_str());
        cacheDb.AbandonChanges();
        return BE_SQLITE_ERROR;
        }

    r = AddMetadataToChangeSummaryCacheFile(cacheDb, m_ecdb);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new ChangeSummaries cache file '%s'. Could not add metadata to the file: %s", cachePath.GetNameUtf8().c_str(), cacheDb.GetLastError().c_str());
        cacheDb.AbandonChanges();
        return r;
        }

    r = cacheDb.SaveChanges();
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new ChangeSummaries cache file '%s'. Could not commit changes: %s", cachePath.GetNameUtf8().c_str(), cacheDb.GetLastError().c_str());
        return r;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::AddMetadataToChangeSummaryCacheFile(ECDb& cacheFile, ECDbCR primaryECDb) const
    {
    BeGuid primaryFileGuid = m_ecdb.GetDbGuid();
    if (primaryFileGuid.IsValid())
        {
        const DbResult r = cacheFile.SavePropertyString(PropertySpec("ECDbGuid", CHANGESUMMARIES_PROPSPEC_NAMESPACE), primaryFileGuid.ToString().c_str());
        if (BE_SQLITE_OK != r)
            return r;
        }
    else
        {
        const DbResult r = cacheFile.SavePropertyString(PropertySpec("ECDbPath", CHANGESUMMARIES_PROPSPEC_NAMESPACE), m_ecdb.GetDbFileName());
        if (BE_SQLITE_OK != r)
            return r;
        }

    ProfileVersion ecdbProfileVersion(0, 0, 0, 0);
    DbResult r = ProfileManager::ReadProfileVersion(ecdbProfileVersion, primaryECDb);
    if (BE_SQLITE_OK != r || ecdbProfileVersion.IsEmpty())
        return r;

    return cacheFile.SavePropertyString(PropertySpec("ECDbSchemaVersion", CHANGESUMMARIES_PROPSPEC_NAMESPACE), ecdbProfileVersion.ToJson().c_str());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
void ChangeSummaryManager::RegisterSqlFunctions() const
    {
    m_ecdb.AddFunction(ChangedValueStateToOpCodeSqlFunction::GetSingleton());

    m_changedValueSqlFunction = std::make_unique<ChangedValueSqlFunction>(m_ecdb);
    m_ecdb.AddFunction(*m_changedValueSqlFunction);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
void ChangeSummaryManager::UnregisterSqlFunction() const
    {
    m_ecdb.RemoveFunction(ChangedValueStateToOpCodeSqlFunction::GetSingleton());

    if (m_changedValueSqlFunction != nullptr)
        {
        m_ecdb.RemoveFunction(*m_changedValueSqlFunction);
        m_changedValueSqlFunction = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
void ChangeSummaryManager::ClearCache()
    {
    if (m_changedValueSqlFunction != nullptr)
        m_changedValueSqlFunction->ClearCache();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
Nullable<ChangeOpCode> ChangeSummaryManager::ToChangeOpCode(DbOpcode opCode)
    {
    switch (opCode)
        {
            case DbOpcode::Delete:
                return ChangeOpCode::Delete;
            case DbOpcode::Insert:
                return ChangeOpCode::Insert;
            case DbOpcode::Update:
                return ChangeOpCode::Update;
            default:
                BeAssert(false && "DbOpcode enum was changed. This code has to be adjusted.");
                return Nullable<ChangeOpCode>();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
Nullable<ChangeOpCode> ChangeSummaryManager::ToChangeOpCode(int val)
    {
    if (val == Enum::ToInt(ChangeOpCode::Insert))
        return ChangeOpCode::Insert;

    if (val == Enum::ToInt(ChangeOpCode::Update))
        return ChangeOpCode::Update;

    if (val == Enum::ToInt(ChangeOpCode::Delete))
        return ChangeOpCode::Delete;

    return Nullable<ChangeOpCode>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
Nullable<DbOpcode> ChangeSummaryManager::ToDbOpCode(ChangeOpCode op)
    {
    switch (op)
        {
            case ChangeOpCode::Delete:
                return DbOpcode::Delete;
            case ChangeOpCode::Insert:
                return DbOpcode::Insert;
            case ChangeOpCode::Update:
                return DbOpcode::Update;
            default:
                BeAssert(false && "ChangeOpCode enum was changed. This code has to be adjusted.");
                return Nullable<DbOpcode>();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
Nullable<ChangedValueState> ChangeSummaryManager::ToChangedValueState(int val)
    {
    if (val == Enum::ToInt(ChangedValueState::AfterInsert))
        return ChangedValueState::AfterInsert;

    if (val == Enum::ToInt(ChangedValueState::BeforeUpdate))
        return ChangedValueState::BeforeUpdate;

    if (val == Enum::ToInt(ChangedValueState::AfterUpdate))
        return ChangedValueState::AfterUpdate;

    if (val == Enum::ToInt(ChangedValueState::BeforeDelete))
        return ChangedValueState::BeforeDelete;

    return Nullable<ChangedValueState>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
Nullable<ChangedValueState> ChangeSummaryManager::ToChangedValueState(Utf8CP strVal)
    {
    if (BeStringUtilities::StricmpAscii("AfterInsert", strVal) == 0)
        return ChangedValueState::AfterInsert;

    if (BeStringUtilities::StricmpAscii("BeforeUpdate", strVal) == 0)
        return ChangedValueState::BeforeUpdate;

    if (BeStringUtilities::StricmpAscii("AfterUpdate", strVal) == 0)
        return ChangedValueState::AfterUpdate;

    if (BeStringUtilities::StricmpAscii("BeforeDelete", strVal) == 0)
        return ChangedValueState::BeforeDelete;

    return Nullable<ChangedValueState>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
Nullable<ChangeOpCode> ChangeSummaryManager::DetermineOpCodeFromChangedValueState(ChangedValueState state)
    {
    if (state == ChangedValueState::AfterInsert)
        return ChangeOpCode::Insert;

    if (state == ChangedValueState::BeforeUpdate || state == ChangedValueState::AfterUpdate)
        return ChangeOpCode::Update;

    if (state == ChangedValueState::BeforeDelete)
        return ChangeOpCode::Delete;

    BeAssert(false && "ChangedValueState enum or ChangeOpCode enum was changed. This code has to be adjusted.");
    return Nullable<ChangeOpCode>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
BeFileName ChangeSummaryManager::DetermineCachePath(ECDbCR ecdb)
    {
    BeFileName path(ecdb.GetDbFileName());
    path.append(FILEEXT_ChangeSummaryCache);
    return path;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
BeFileName ChangeSummaryManager::DetermineCachePath(BeFileNameCR ecdbPath)
    {
    BeFileName path(ecdbPath);
    path.append(FILEEXT_ChangeSummaryCache);
    return path;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
