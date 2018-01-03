/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define CHANGE_PROPSPEC_NAMESPACE "ec_Change"

//****************************************************************************
// ChangeManager
//****************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//Must match version of ECDbChange ECSchema:
//ProfileVersion::Major == ECSchema VersionMajor
//ProfileVersion::Minor == ECSchema VersionWrite
//ProfileVersion::Sub1 == ECSchema VersionMinor
//static
ProfileVersion const* ChangeManager::s_expectedCacheVersion = new ProfileVersion(1, 0, 0, 0);

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     12/2017
//---------------------------------------------------------------------------------------
//static
bool ChangeManager::IsChangeCacheAttachedAndValid(ECDbCR ecdb, bool logError)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(ecdb, "SELECT VersionMajor,VersionWrite,VersionMinor FROM " TABLESPACE_ECChange ".meta.ECSchemaDef WHERE Name='" ECSCHEMA_ECDbChange "'"))
        {
        if (logError)
            ecdb.GetImpl().Issues().Report("Changes cache file is not attached or another file has been attached with the same table space (" TABLESPACE_ECChange ")");

        return false; //if file is not attached or not an ECDb file
        }

    if (stmt.Step() != BE_SQLITE_ROW)
        {
        if (logError)
            ecdb.GetImpl().Issues().Report("Attached file with table space '" TABLESPACE_ECChange "' is not a Changes cache file.");

        return false;
        }

    const int versionDigit1 = stmt.GetValueInt(0);
    const int versionDigit2 = stmt.GetValueInt(1);
    const int versionDigit3 = stmt.GetValueInt(2);

    if (versionDigit1 == (int) s_expectedCacheVersion->GetMajor() && versionDigit2 == (int) s_expectedCacheVersion->GetMinor() &&  versionDigit3 == (int) s_expectedCacheVersion->GetSub1())
        return true;

    if (logError)
        ecdb.GetImpl().Issues().Report("Attached file is a Change cache file with a mismatching version. Expected cache file version: %s. Actual version: %d.%d.%d",
                                         s_expectedCacheVersion->ToString().c_str(), versionDigit1, versionDigit2, versionDigit3);

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     12/2017
//---------------------------------------------------------------------------------------
//static
bool ChangeManager::IsChangeCacheValid(ECDbCR cacheECDb, bool logError)
    {
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(cacheECDb, "SELECT VersionMajor,VersionWrite,VersionMinor FROM meta.ECSchemaDef WHERE Name='" ECSCHEMA_ECDbChange "'"))
        {
        if (logError)
            cacheECDb.GetImpl().Issues().Report("Invalid Change cache file '%s' : File is not an ECDb file.", cacheECDb.GetDbFileName());

        return false; //if file is not attached or not an ECDb file
        }

    if (stmt.Step() != BE_SQLITE_ROW)
        {
        //it is an ECDb file but not a change cache (because it doesn't have the change ECSchema)
        if (logError)
            cacheECDb.GetImpl().Issues().Report("Invalid Change cache file '%s'.", cacheECDb.GetDbFileName());

        return false;
        }

    const int versionDigit1 = stmt.GetValueInt(0);
    const int versionDigit2 = stmt.GetValueInt(1);
    const int versionDigit3 = stmt.GetValueInt(2);

    if (versionDigit1 == (int) s_expectedCacheVersion->GetMajor() && versionDigit2 == (int) s_expectedCacheVersion->GetMinor() && versionDigit3 == (int) s_expectedCacheVersion->GetSub1())
        return true;

    if (logError)
        cacheECDb.GetImpl().Issues().Report("Invalid Change cache file '%s' : Mismatching versions. Expected cache file version: %s. Actual version: %d.%d.%d",
                                            cacheECDb.GetDbFileName(), s_expectedCacheVersion->ToString().c_str(), versionDigit1, versionDigit2, versionDigit3);

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeManager::AttachChangeCacheFile(BeFileNameCR cacheFilePath, bool createIfNotExists) const
    {
    if (!m_ecdb.IsDbOpen())
        return BE_SQLITE_ERROR;

    if (cacheFilePath.empty())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to attach the Changes cache file. Path to Changes cache file must not be empty.");
        return BE_SQLITE_ERROR;
        }

    if (Utf8String::IsNullOrEmpty(m_ecdb.GetDbFileName()))
        {
        m_ecdb.GetImpl().Issues().Report("Failed to attach the Changes cache file. Primary file is an in-memory or temporary ECDb file for which Change cache files are not supported.");
        return BE_SQLITE_ERROR;
        }

    if (DbUtilities::TableSpaceExists(m_ecdb, TABLESPACE_ECChange))
        return BE_SQLITE_ERROR;

    const bool cacheAlreadyExisted = cacheFilePath.DoesPathExist();
    if (!cacheAlreadyExisted)
        {
        if (!createIfNotExists)
            return BE_SQLITE_OK;

        ECDb changeCache;
        DbResult r = CreateChangeCacheFile(changeCache, cacheFilePath);
        if (BE_SQLITE_OK != r)
            return r;
        }

    if (!cacheFilePath.DoesPathExist())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    DbResult r = m_ecdb.AttachDb(cacheFilePath.GetNameUtf8().c_str(), TABLESPACE_ECChange);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to attach Change cache file '%s': %s", cacheFilePath.GetNameUtf8().c_str(), m_ecdb.GetLastError().c_str());
        return r;
        }

    if (cacheAlreadyExisted && !IsChangeCacheAttachedAndValid(m_ecdb, true))
        return BE_SQLITE_ERROR;

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeManager::CreateChangeCacheFile(ECDbR cacheDb, BeFileNameCR cachePath) const
    {
    if (cachePath.DoesPathExist())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create Change cache file '%s'. The file already exists and could not be deleted.", cachePath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
        }

    BeAssert(!cacheDb.IsDbOpen());
    DbResult r = cacheDb.CreateNewDb(cachePath);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new Change cache file '%s': %s", cachePath.GetNameUtf8().c_str(), Db::InterpretDbResult(r));
        return r;
        }

    //import ChangeSummaries schema
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(cacheDb.GetSchemaLocater());

    BeFileName ecdbStandardSchemasFolder(context->GetHostAssetsDirectory());
    ecdbStandardSchemasFolder.AppendToPath(L"ECSchemas");
    ecdbStandardSchemasFolder.AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbStandardSchemasFolder);

    ECN::SchemaKey schemaKey(ECSCHEMA_ECDbChange, 1, 0, 0);
    if (context->LocateSchema(schemaKey, ECN::SchemaMatchType::LatestWriteCompatible) == nullptr)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new Change cache file '%s': Could not locate ECSchema " ECSCHEMA_ECDbChange, cachePath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
        }

    if (SUCCESS != cacheDb.Schemas().ImportSchemas(context->GetCache().GetSchemas(), cacheDb.GetImpl().GetSettingsManager().GetSchemaImportToken()))
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new Change cache file '%s': Could not import ECSchema " ECSCHEMA_ECDbChange, cachePath.GetNameUtf8().c_str());
        cacheDb.AbandonChanges();
        return BE_SQLITE_ERROR;
        }

    r = AddMetadataToChangeCacheFile(cacheDb, m_ecdb);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new Change cache file '%s'. Could not add metadata to the file: %s", cachePath.GetNameUtf8().c_str(), cacheDb.GetLastError().c_str());
        cacheDb.AbandonChanges();
        return r;
        }

    r = cacheDb.SaveChanges();
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new Change cache file '%s'. Could not commit changes: %s", cachePath.GetNameUtf8().c_str(), cacheDb.GetLastError().c_str());
        return r;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeManager::AddMetadataToChangeCacheFile(ECDb& cacheFile, ECDbCR primaryECDb) const
    {
    BeGuid primaryFileGuid = m_ecdb.GetDbGuid();
    if (primaryFileGuid.IsValid())
        {
        const DbResult r = cacheFile.SavePropertyString(PropertySpec("ECDbGuid", CHANGE_PROPSPEC_NAMESPACE), primaryFileGuid.ToString().c_str());
        if (BE_SQLITE_OK != r)
            return r;
        }
    else
        {
        const DbResult r = cacheFile.SavePropertyString(PropertySpec("ECDbPath", CHANGE_PROPSPEC_NAMESPACE), m_ecdb.GetDbFileName());
        if (BE_SQLITE_OK != r)
            return r;
        }

    ProfileVersion ecdbProfileVersion(0, 0, 0, 0);
    DbResult r = ProfileManager::ReadProfileVersion(ecdbProfileVersion, primaryECDb);
    if (BE_SQLITE_OK != r || ecdbProfileVersion.IsEmpty())
        return r;

    return cacheFile.SavePropertyString(PropertySpec("ECDbSchemaVersion", CHANGE_PROPSPEC_NAMESPACE), ecdbProfileVersion.ToJson().c_str());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
void ChangeManager::RegisterSqlFunctions() const
    {
    m_ecdb.AddFunction(ChangedValueStateToOpCodeSqlFunction::GetSingleton());

    m_changedValueSqlFunction = std::make_unique<ChangedValueSqlFunction>(m_ecdb);
    m_ecdb.AddFunction(*m_changedValueSqlFunction);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
void ChangeManager::UnregisterSqlFunction() const
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
void ChangeManager::ClearCache()
    {
    if (m_changedValueSqlFunction != nullptr)
        m_changedValueSqlFunction->ClearCache();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
//static
Nullable<ChangeOpCode> ChangeManager::ToChangeOpCode(DbOpcode opCode)
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
Nullable<ChangeOpCode> ChangeManager::ToChangeOpCode(int val)
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
Nullable<DbOpcode> ChangeManager::ToDbOpCode(ChangeOpCode op)
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
Nullable<ChangedValueState> ChangeManager::ToChangedValueState(int val)
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
Nullable<ChangedValueState> ChangeManager::ToChangedValueState(Utf8CP strVal)
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
Nullable<ChangeOpCode> ChangeManager::DetermineOpCodeFromChangedValueState(ChangedValueState state)
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
BeFileName ChangeManager::DetermineDefaultCachePath(Utf8CP ecdbPath)
    {
    BeFileName path(ecdbPath);
    path.append(FILEEXT_ChangeCache);
    return path;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
