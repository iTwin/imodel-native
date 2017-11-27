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

    if (!IsChangeSummaryCacheAttached())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to extract ChangeSummary. ChangeSummary cache hasn't been attached yet.");
        return ERROR;
        }

    return m_extractor.Extract(summaryKey, changeset, options);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::OnCreatingECDb() const
    {
    BeAssert(!IsChangeSummaryCacheAttached());

    BeFileName cachePath = DetermineCachePath(m_ecdb);
    if (cachePath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != cachePath.BeDeleteFile())
            {
            m_ecdb.GetImpl().Issues().Report("Failed to create ChangeSummaries cache file '%s'. The file already exists and could not be deleted.", cachePath.GetNameUtf8().c_str());
            return BE_SQLITE_ERROR;
            }

        LOG.warningv("A ChangeSummaries cache file '%s' already exists for the new ECDb file. The existing cache file has been deleted.", cachePath.GetNameUtf8().c_str());
        }

    Db cacheDb;
    DbResult r = cacheDb.CreateNewDb(cachePath);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new ChangeSummaries cache file '%s': %s", cachePath.GetNameUtf8().c_str(), Db::InterpretDbResult(r));
        cacheDb.AbandonChanges();
        return r;
        }

    //cannot read profile version from ecdb file as the profile wasn't created yet. But as this method is only called when creating an ECDb
    //we know the profile version.
    r = AddMetadataToChangeSummaryCacheFile(cacheDb, ProfileManager::GetExpectedVersion());
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to create new ChangeSummaries cache file '%s'. Could not add metadata to the file: %s", cachePath.GetNameUtf8().c_str(), cacheDb.GetLastError().c_str());
        cacheDb.AbandonChanges();
        return r;
        }

    cacheDb.SaveChanges();
    cacheDb.CloseDb();
    return DoAttachChangeSummaryCacheFile(cachePath);
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

        DbResult r = CreateChangeSummaryCacheFile(cachePath);
        if (BE_SQLITE_OK != r)
            return r;
        }

    BeAssert(cachePath.DoesPathExist());
    return DoAttachChangeSummaryCacheFile(cachePath);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::CreateChangeSummaryCacheFile(BeFileName const& cachePath) const
    {
    BeAssert(!cachePath.DoesPathExist());

    SeedFile seed(m_ecdb.GetDbFileName(), cachePath);

    if (!seed.IsValid())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to recreate ChangeSummaries cache file '%s'. Could not create a temporary seed file folder.", cachePath.GetNameUtf8().c_str());
        return BE_SQLITE_ERROR;
        }

    DbResult r = seed.CreateCacheFromSeed(cachePath);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to recreate ChangeSummaries cache file '%s'. Could not create seed ChangeSummary cache file from seed: %s", cachePath.GetNameUtf8().c_str(), Db::InterpretDbResult(r));
        return r;
        }

    Db cacheDb;
    r = cacheDb.OpenBeSQLiteDb(cachePath, Db::OpenParams(Db::OpenMode::ReadWrite));
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to recreate ChangeSummaries cache file '%s'. Opening new ChangeSummaries cache file failed: %s", cachePath.GetNameUtf8().c_str(), Db::InterpretDbResult(r));
        return r;
        }

    ProfileVersion ecdbProfileVersion(0, 0, 0, 0);
    r = ProfileManager::ReadProfileVersion(ecdbProfileVersion, m_ecdb);
    if (BE_SQLITE_OK != r || ecdbProfileVersion.IsEmpty())
        {
        m_ecdb.GetImpl().Issues().Report("Failed to recreate ChangeSummaries cache file '%s'. Could not read ECDb profile version from file %s.", cachePath.GetNameUtf8().c_str(),
                                         m_ecdb.GetDbFileName());
        return r;
        }

    r = AddMetadataToChangeSummaryCacheFile(cacheDb, ecdbProfileVersion);
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to recreate ChangeSummaries cache file '%s'. Adding metadata to new ChangeSummaries cache file failed: %s", cachePath.GetNameUtf8().c_str(), cacheDb.GetLastError().c_str());
        return r;
        }

    r = cacheDb.SaveChanges();
    if (BE_SQLITE_OK != r)
        {
        m_ecdb.GetImpl().Issues().Report("Failed to recreate ChangeSummaries cache file '%s'. Could not commit changes: %s", cachePath.GetNameUtf8().c_str(), cacheDb.GetLastError().c_str());
        return r;
        }

    cacheDb.CloseDb();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::AddMetadataToChangeSummaryCacheFile(Db& cacheFile, ProfileVersion const& ecdbProfileVersion) const
    {
    if (ecdbProfileVersion.IsEmpty())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

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

    return cacheFile.SavePropertyString(PropertySpec("ECDbSchemaVersion", CHANGESUMMARIES_PROPSPEC_NAMESPACE), ecdbProfileVersion.ToJson().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::DoAttachChangeSummaryCacheFile(BeFileNameCR cachePath) const
    {
    BeAssert(!IsChangeSummaryCacheAttached());

    if (!cachePath.DoesPathExist())
        return BE_SQLITE_OK;

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
    m_extractor.ClearCache();

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

//****************************************************************************
// ChangeSummaryManager::SeedFile
//****************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
ChangeSummaryManager::SeedFile::SeedFile(Utf8CP ecdbPath, BeFileName const& cachePath) : m_ecdbPath(ecdbPath), m_cachePath(cachePath)
    {
    BeFileName seedFolderName(m_ecdbPath);
    seedFolderName.ReplaceAll(L".", L"_");

    int suffix = 0;
    BeFileName seedFolderTemp(seedFolderName);
    while (seedFolderTemp.DoesPathExist())
        {
        if (suffix >= 100)
            return; //abort after 100 attempts 

        suffix++;

        seedFolderTemp.Sprintf(L"%ls_%d", seedFolderName.c_str(), suffix);
        }

    BeAssert(!seedFolderTemp.DoesPathExist());
    m_seedFolder.assign(seedFolderTemp);

    m_seedECDbPath = m_seedFolder;
    m_seedECDbPath.AppendToPath(L"seed.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
ChangeSummaryManager::SeedFile::~SeedFile()
    {
    if (!IsValid() || !m_seedFolder.DoesPathExist())
        return;

    if (BeFileNameStatus::Success != BeFileName::EmptyAndRemoveDirectory(m_seedFolder))
        LOG.warningv("Failed to delete seed ChangeSummaries cache files: %s", m_seedFolder.GetNameUtf8().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle     11/2017
//---------------------------------------------------------------------------------------
DbResult ChangeSummaryManager::SeedFile::CreateCacheFromSeed(BeFileNameCR targetCachePath) const
    {
    if (!IsValid())
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    if (!m_seedFolder.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(m_seedFolder.c_str()))
            return BE_SQLITE_ERROR;
        }

    //this will create a new ECDb and a new ChangeSummary cache file which we will use as seed file
    ECDb seedECDb;
    const DbResult r = seedECDb.CreateNewDb(m_seedECDbPath);
    if (BE_SQLITE_OK != r)
        return r;

    m_seedCachePath = ChangeSummaryManager::DetermineCachePath(seedECDb);

    seedECDb.SaveChanges();
    seedECDb.CloseDb();

    BeAssert(m_seedCachePath.DoesPathExist()); 
    if (BeFileNameStatus::Success != BeFileName::BeMoveFile(m_seedCachePath, targetCachePath))
        return BE_SQLITE_ERROR;

    return BE_SQLITE_OK;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
