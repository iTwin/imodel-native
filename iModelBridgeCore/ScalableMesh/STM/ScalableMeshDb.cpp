/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include "ScalableMeshDb.h"
#include "SMSQLiteClipDefinitionsFile.h"
#include "SMSQLiteDiffsetFile.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

/*
Here is how the version logic works. All files (including secondary files) have individual 4-part version numbers.
All files with an inferior version number get automatically upgraded.
On platforms that support version checks, we also refuse to open too-recent files (files that have higher major or minor versions).
This is consistent with other BeSQLite profiles.
*/

const BESQL_VERSION_STRUCT ScalableMeshDb::CURRENT_VERSION = BESQL_VERSION_STRUCT(1, 1, 0, 1);
static bool s_checkShemaVersion = true;

BESQL_VERSION_STRUCT ScalableMeshDb::GetCurrentVersion() const
    {
    switch (m_type)
        {
        case SQLDatabaseType::SM_MAIN_DB_FILE:
            return SMSQLiteFile::CURRENT_VERSION;
            break;
        case SQLDatabaseType::SM_CLIP_DEF_FILE:
            return SMSQLiteClipDefinitionsFile::CURRENT_VERSION;
            break;
        case SQLDatabaseType::SM_DIFFSETS_FILE:
            return  SMSQLiteDiffsetFile::CURRENT_VERSION;
            break;
        case SQLDatabaseType::SM_GENERATION_FILE:
        default:
            return ScalableMeshDb::CURRENT_VERSION;
            break;
        }
    }

#ifndef VANCOUVER_API

#ifdef DGNDB06_API

DbResult ScalableMeshDb::_VerifySchemaVersion(OpenParams const& params)
    {
    CachedStatementPtr stmtTest;
    GetCachedStatement(stmtTest, "SELECT Version FROM SMFileMetadata");
    assert(stmtTest != nullptr);
    DbResult status = stmtTest->Step();

    assert(status == BE_SQLITE_ROW);

    Utf8String schemaVs(stmtTest->GetValueText(0));

    SchemaVersion databaseSchema(schemaVs.c_str());

    SchemaVersion currentVersion = GetCurrentVersion();
    if (s_checkShemaVersion && (databaseSchema.CompareTo(currentVersion, SchemaVersion::VERSION_All) < 0 || databaseSchema.CompareTo(currentVersion, SchemaVersion::VERSION_MajorMinor) != 0))
        return BE_SQLITE_SCHEMA;

    return BE_SQLITE_OK;
    }

#else

ProfileState ScalableMeshDb::_CheckProfileVersion() const
    {
    CachedStatementPtr stmtTest;
    DbResult status = GetCachedStatement(stmtTest, "SELECT Version FROM SMFileMetadata");
    assert(stmtTest != nullptr && status == BE_SQLITE_OK);
    status = stmtTest->Step();

    assert(status == BE_SQLITE_ROW);

    Utf8String schemaVs(stmtTest->GetValueText(0));

    BESQL_VERSION_STRUCT databaseSchema(schemaVs.c_str());

    BESQL_VERSION_STRUCT currentVersion = GetCurrentVersion();
    if (s_checkShemaVersion && (databaseSchema.CompareTo(currentVersion, BESQL_VERSION_STRUCT::VERSION_All) < 0 || databaseSchema.CompareTo(currentVersion, BESQL_VERSION_STRUCT::VERSION_MajorMinor) != 0))
        return ProfileState::Older(ProfileState::CanOpen::No, true);

    return ProfileState::UpToDate();
    }

DbResult ScalableMeshDb::_UpgradeProfile()
    {
    DbResult updateResult = m_smFile->UpdateDatabase();

    return updateResult;
    }

#endif

#else


static Utf8CP s_versionfmt = "{\"major\":%d,\"minor\":%d,\"sub1\":%d,\"sub2\":%d}";
Utf8String SchemaVersion::ToJson() const { return Utf8PrintfString(s_versionfmt, m_major, m_minor, m_sub1, m_sub2); }

void SchemaVersion::FromJson(Utf8CP json) {/* FromString(json, "{\"major\":%d,\"minor\":%d,\"sub1\":%d,\"sub2\":%d}");*/  };

DbResult ScalableMeshDb::IsProfileVersionUpToDate(OpenParams const& params)
{
    CachedStatementPtr stmtTest;
    DbResult status = GetCachedStatement(stmtTest, "SELECT Version FROM SMFileMetadata");
    assert(stmtTest != nullptr && status == BE_SQLITE_OK);
    status = stmtTest->Step();

    assert(status == BE_SQLITE_ROW);

    Utf8String schemaVs(stmtTest->GetValueText(0));

    BESQL_VERSION_STRUCT databaseSchema(schemaVs.c_str());

    BESQL_VERSION_STRUCT currentVersion = GetCurrentVersion();
    if (s_checkShemaVersion && (databaseSchema.CompareTo(currentVersion, BESQL_VERSION_STRUCT::VERSION_All) < 0 || databaseSchema.CompareTo(currentVersion, BESQL_VERSION_STRUCT::VERSION_MajorMinor) != 0))
        return BE_SQLITE_SCHEMA;

    return BE_SQLITE_OK;
}
#endif


DbResult ScalableMeshDb::_OnDbCreated(CreateParams const& params)
    {
    Savepoint sp(*this, "CreateVersion");
    CreateTable("SMFileMetadata", "Version TEXT, Properties TEXT");
    //m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, Balanced, RootNodeId, SplitTreshold, Depth, TerrainDepth, IsTextured, IsTerrain) VALUES(?,?,?,?,?,?,?,?)");

    CachedStatementPtr stmt;
    GetCachedStatement(stmt, "INSERT INTO SMFileMetadata (Version, Properties) VALUES(?,?)");
    BESQL_VERSION_STRUCT currentVersion = GetCurrentVersion();
    Utf8String versonJson(currentVersion.ToJson());
    stmt->BindText(1, versonJson.c_str(), Statement::MakeCopy::Yes);
    stmt->BindText(2, "", Statement::MakeCopy::Yes);
    stmt->Step();

    return BeSQLite::Db::_OnDbCreated(params);
    }

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

bool ScalableMeshDb::s_enableSharedDatabase = false;

END_BENTLEY_SCALABLEMESH_NAMESPACE

int InfiniteRetries::_OnBusy(int count) const
{
    return 1;
}

#ifndef VANCOUVER_API
DbResult ScalableMeshDb::OpenShared(BENTLEY_NAMESPACE_NAME::Utf8CP path, bool readonly,bool allowBusyRetry)
{
    m_path = path;
    DbResult result = this->OpenBeSQLiteDb(path, BeSQLite::Db::OpenParams(readonly ? BeSQLite::Db::OpenMode::Readonly : BeSQLite::Db::OpenMode::ReadWrite, readonly ? BeSQLite::DefaultTxn::No : BeSQLite::DefaultTxn::Immediate, allowBusyRetry? new InfiniteRetries(): nullptr));

    if (result == BE_SQLITE_ERROR_FileNotFound)
        {
        result = this->OpenBeSQLiteDb(path, BeSQLite::Db::OpenParams(readonly ? BeSQLite::Db::OpenMode::Readonly : BeSQLite::Db::OpenMode::ReadWrite));

        if (result != BE_SQLITE_OK)

            return result;

        this->CloseDb();

        result = this->OpenBeSQLiteDb(path, BeSQLite::Db::OpenParams(readonly ? BeSQLite::Db::OpenMode::Readonly : BeSQLite::Db::OpenMode::ReadWrite, readonly ? BeSQLite::DefaultTxn::No : BeSQLite::DefaultTxn::Immediate, allowBusyRetry? new InfiniteRetries(): nullptr));

        assert(result == BE_SQLITE_OK);
        }

#ifdef IMODEL02
    this->SetAllowImplicitTransactions(true);
#else
    this->SetAllowImplictTransactions(true);
#endif
    return result;
}

#ifndef NDEBUG
void ScalableMeshDb::SetCanReopenShared(bool canReopenShared)
    {
    m_canReopenShared = canReopenShared;
    }
#endif

bool ScalableMeshDb::ReOpenShared(bool readonly, bool allowBusyRetry)
    {
#ifndef NDEBUG
    assert(m_canReopenShared);
#endif

    if (m_path.empty())
        return false;

    DbResult result = this->OpenBeSQLiteDb(m_path.c_str(), BeSQLite::Db::OpenParams(readonly ? BeSQLite::Db::OpenMode::Readonly : BeSQLite::Db::OpenMode::ReadWrite, readonly ? BeSQLite::DefaultTxn::No : BeSQLite::DefaultTxn::Immediate, allowBusyRetry ? new InfiniteRetries() : nullptr));

    if (result != BE_SQLITE_OK)
        return false;

#ifdef IMODEL02
    this->SetAllowImplicitTransactions(true);
#else
    this->SetAllowImplictTransactions(true);
#endif
    return true;
}

bool ScalableMeshDb::StartTransaction()
{
    if (m_currentSavepoint != nullptr)
        return false;
    if (!this->IsDbOpen())
        return false;

    m_currentSavepoint = new BeSQLite::Savepoint(*this, "DefaultSavePoint");
    return true;
}

bool ScalableMeshDb::CommitTransaction()
    {
    if (m_currentSavepoint == nullptr)
        return false;

    if (!this->IsDbOpen())
        return false;

    m_currentSavepoint->Commit();

    delete m_currentSavepoint;
    m_currentSavepoint = nullptr;

    return true;
    }

void ScalableMeshDb::CloseShared(bool& wasTransactionAbandoned)
{
    if (!this->IsDbOpen())
        return;
    wasTransactionAbandoned = false;

    if (m_currentSavepoint != nullptr)
        {
        m_currentSavepoint->Cancel();
        wasTransactionAbandoned = true;

        delete m_currentSavepoint;
        m_currentSavepoint = nullptr;
        }

    this->CloseDb();
}

void ScalableMeshDb::GetSharedDbFileName(BENTLEY_NAMESPACE_NAME::Utf8String& path)
    {
    path = m_path;
    }

#endif

bool ScalableMeshDb::GetEnableSharedDatabase()
    {
    return s_enableSharedDatabase;
    }

void  ScalableMeshDb::SetEnableSharedDatabase(bool isShared)
    {
    s_enableSharedDatabase = isShared;
    }
