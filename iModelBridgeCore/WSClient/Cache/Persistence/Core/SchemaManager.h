/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Core/SchemaManager.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        bvector<ECN::ECSchemaCP> m_repositorySchemas;

    private:
        ECSchemaPtr LoadSchema(SchemaKey key, ECSchemaReadContext& context);
        ECSchemaPtr LoadSchema(BeFileNameCR schemaPath, ECSchemaReadContext& context);
        BentleyStatus LoadSchemas(const std::vector<BeFileName>& schemaPaths, std::vector<ECSchemaPtr>& schemasOut);
        BentleyStatus FixLegacySchema(ECSchema& schema, ECSchemaReadContextR context);
        Utf8String ToFullNameListString(const std::vector<ECSchemaPtr>& schemas);

    public:
        SchemaManager(ObservableECDb& m_db);

        BentleyStatus ImportCacheSchemas();
        BentleyStatus ImportSchemas(const std::vector<BeFileName>& schemaPaths);
        BentleyStatus ImportSchemas(const std::vector<ECSchemaPtr>& schemas);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
