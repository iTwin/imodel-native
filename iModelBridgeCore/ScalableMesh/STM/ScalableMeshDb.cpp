#include "ScalableMeshPCH.h"
#include "ScalableMeshDb.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

const SchemaVersion ScalableMeshDb::CURRENT_VERSION = SchemaVersion(1, 1, 0, 0);
static bool s_checkShemaVersion = false;


DbResult ScalableMeshDb::_VerifySchemaVersion(OpenParams const& params)
    {
    CachedStatementPtr stmtTest;
    GetCachedStatement(stmtTest, "SELECT Version FROM SMFileMetadata");
    assert(stmtTest != nullptr);
    DbResult status = stmtTest->Step();

    assert(status == BE_SQLITE_ROW);
       
    Utf8String schemaVs(stmtTest->GetValueText(0));

    SchemaVersion databaseSchema(schemaVs.c_str());
   
    if (s_checkShemaVersion && databaseSchema.CompareTo(ScalableMeshDb::CURRENT_VERSION, SchemaVersion::VERSION_All) != 0)
        return BE_SQLITE_SCHEMA;

    return BE_SQLITE_OK;
    }

DbResult ScalableMeshDb::_OnDbCreated(CreateParams const& params)
    {        
    Savepoint sp(*this, "CreateVersion");
    CreateTable("SMFileMetadata", "Version TEXT");
    //m_database->GetCachedStatement(stmt, "INSERT INTO SMMasterHeader (MasterHeaderId, Balanced, RootNodeId, SplitTreshold, Depth, TerrainDepth, IsTextured, IsTerrain) VALUES(?,?,?,?,?,?,?,?)");

    CachedStatementPtr stmt;
    GetCachedStatement(stmt, "INSERT INTO SMFileMetadata (Version) VALUES(?)");
    Utf8String versonJson(ScalableMeshDb::CURRENT_VERSION.ToJson());

    stmt->BindText(1, versonJson.c_str(), Statement::MakeCopy::Yes);
    DbResult status = stmt->Step();
    status = status;
        
    return BeSQLite::Db::_OnDbCreated(params);
    }