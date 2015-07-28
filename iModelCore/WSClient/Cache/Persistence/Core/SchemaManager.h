/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Core/SchemaManager.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    00/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaManager
    {
    private:
        ObservableECDb& m_db;
        ECSchemaList m_repositorySchemas;

    public:
        SchemaManager(ObservableECDb& m_db);

        BentleyStatus ImportCacheSchemas();
        BentleyStatus ImportSchemas(const std::vector<BeFileName>& schemaPaths);
        BentleyStatus ImportSchemas(const std::vector<ECSchemaPtr>& schemas);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
