#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite\BeSQLite.h>
#include <ScalableMesh\ScalableMeshDefs.h>
#include <ScalableMesh/import/DataSQLite.h>


enum class SQLDatabaseType
    {
    SM_MAIN_DB_FILE,
    SM_CLIP_DEF_FILE,
    SM_DIFFSETS_FILE,
    SM_GENERATION_FILE
    };


USING_NAMESPACE_BENTLEY_SQLITE
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class ScalableMeshDb : public BeSQLite::Db
    {
    private:
        SQLDatabaseType m_type;
        SchemaVersion GetCurrentVersion();
    protected:
#ifndef VANCOUVER_API
    virtual DbResult _VerifySchemaVersion(OpenParams const& params) override;
#endif
    virtual DbResult _OnDbCreated(CreateParams const& params) override;

    public:
        ScalableMeshDb(SQLDatabaseType type) : m_type(type) {}
        static const SchemaVersion CURRENT_VERSION;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE