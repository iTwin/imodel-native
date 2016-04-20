#include "ScalableMeshPCH.h"
#include "ScalableMeshDb.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

const SchemaVersion ScalableMeshDb::CURRENT_VERSION = SchemaVersion(1, 0, 0, 0);

DbResult ScalableMeshDb::_VerifySchemaVersion(OpenParams const& params)
    {
    return BE_SQLITE_OK;
    }

DbResult ScalableMeshDb::_OnDbCreated(CreateParams const& params)
    {
        {
        Savepoint sp(*this, "CreateVersion");
        CreateTable("SMFileMetadata", "Version TEXT");
        Statement stmt(*this, "INSERT INTO SMFileMetadata VALUES(?)");
        stmt.BindText(1, ScalableMeshDb::CURRENT_VERSION.ToJson(), Statement::MakeCopy::No);
        stmt.Step();
        }
    return BeSQLite::Db::_OnDbCreated(params);
    }