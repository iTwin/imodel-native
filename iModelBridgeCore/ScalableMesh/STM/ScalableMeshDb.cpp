#include "ScalableMeshPCH.h"
#include "ScalableMeshDb.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

const SchemaVersion ScalableMeshDb::CURRENT_VERSION = SchemaVersion(1, 0, 0, 0);

#ifndef VANCOUVER_API
DbResult ScalableMeshDb::_VerifySchemaVersion(OpenParams const& params)
    {
    return BE_SQLITE_OK;
    }
#endif

DbResult ScalableMeshDb::_OnDbCreated(CreateParams const& params)
    {
        {
        Savepoint sp(*this, "CreateVersion");
        CreateTable("SMFileMetadata", "Version TEXT");
#ifndef VANCOUVER_API
        Statement stmt(*this, "INSERT INTO SMFileMetadata VALUES(?)");
        stmt.BindText(1, ScalableMeshDb::CURRENT_VERSION.ToJson(), Statement::MakeCopy::No);
#else
        Statement stmt;
        stmt.Prepare(*this, "INSERT INTO SMFileMetadata VALUES(?)");
        stmt.BindUtf8String(1, ScalableMeshDb::CURRENT_VERSION.ToJson(), Statement::MAKE_COPY_No);
#endif
        stmt.Step();
        }
    return BeSQLite::Db::_OnDbCreated(params);
    }