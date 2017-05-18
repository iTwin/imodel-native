#include "ScalableMeshPCH.h"
#include "ScalableMeshDb.h"
#include "SMSQLiteClipDefinitionsFile.h"
#include "SMSQLiteDiffsetFile.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

const SchemaVersion ScalableMeshDb::CURRENT_VERSION = SchemaVersion(1, 1, 0, 1);
static bool s_checkShemaVersion = true;

SchemaVersion ScalableMeshDb::GetCurrentVersion()
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

    SchemaVersion databaseSchema(schemaVs.c_str());
   
    SchemaVersion currentVersion = GetCurrentVersion();
    if (s_checkShemaVersion && (databaseSchema.CompareTo(currentVersion, SchemaVersion::VERSION_All) < 0 || databaseSchema.CompareTo(currentVersion, SchemaVersion::VERSION_MajorMinor) != 0))
        return BE_SQLITE_SCHEMA;

    return BE_SQLITE_OK;
    }
#endif

DbResult ScalableMeshDb::_OnDbCreated(CreateParams const& params)
    {        
    Savepoint sp(*this, "CreateVersion");
    CreateTable("SMFileMetadata", "Version TEXT");
    //m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, Balanced, RootNodeId, SplitTreshold, Depth, TerrainDepth, IsTextured, IsTerrain) VALUES(?,?,?,?,?,?,?,?)");

    CachedStatementPtr stmt;
    GetCachedStatement(stmt, "INSERT INTO SMFileMetadata (Version) VALUES(?)");
    SchemaVersion currentVersion = GetCurrentVersion();
    Utf8String versonJson(currentVersion.ToJson());
#ifndef VANCOUVER_API
       stmt->BindText(1, versonJson.c_str(), Statement::MakeCopy::Yes);
#else
        stmt->BindUtf8String(1, versonJson, Statement::MAKE_COPY_Yes);
#endif
    DbResult status = stmt->Step();
    status = status;
        
    return BeSQLite::Db::_OnDbCreated(params);
    }