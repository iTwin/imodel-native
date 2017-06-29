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

BESQL_VERSION_STRUCT ScalableMeshDb::GetCurrentVersion()
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
DbResult ScalableMeshDb::_VerifySchemaVersion(OpenParams const& params)
    {

    CachedStatementPtr stmtTest;
    GetCachedStatement(stmtTest, "SELECT Version FROM SMFileMetadata");
    assert(stmtTest != nullptr);
    DbResult status = stmtTest->Step();

    assert(status == BE_SQLITE_ROW);
       
    Utf8String schemaVs(stmtTest->GetValueText(0));

    BESQL_VERSION_STRUCT databaseSchema(schemaVs.c_str());
   
    BESQL_VERSION_STRUCT currentVersion = GetCurrentVersion();
    if (s_checkShemaVersion && (databaseSchema.CompareTo(currentVersion, BESQL_VERSION_STRUCT::VERSION_All) < 0 || databaseSchema.CompareTo(currentVersion, SchemaVersion::VERSION_MajorMinor) != 0))
        return BE_SQLITE_SCHEMA;

    return BE_SQLITE_OK;
    }
#else
static Utf8CP s_versionfmt = "{\"major\":%d,\"minor\":%d,\"sub1\":%d,\"sub2\":%d}";
Utf8String SchemaVersion::ToJson() const { return Utf8PrintfString(s_versionfmt, m_major, m_minor, m_sub1, m_sub2); }
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
    DbResult status = stmt->Step();
    status = status;
        
    return BeSQLite::Db::_OnDbCreated(params);
    }