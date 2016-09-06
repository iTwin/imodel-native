#pragma once

#include <Bentley/RefCounted.h>
#include <BeSQLite\BeSQLite.h>
#include <ScalableMesh\ScalableMeshDefs.h>
#include <ScalableMesh/import/DataSQLite.h>

USING_NAMESPACE_BENTLEY_SQLITE
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class ScalableMeshDb : public BeSQLite::Db
    {

    static const SchemaVersion CURRENT_VERSION;
#ifndef VANCOUVER_API
    virtual DbResult _VerifySchemaVersion(OpenParams const& params) override;
#endif
    virtual DbResult _OnDbCreated(CreateParams const& params) override;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE