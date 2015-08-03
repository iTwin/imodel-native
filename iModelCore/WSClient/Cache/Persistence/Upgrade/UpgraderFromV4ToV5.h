/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/Upgrade/UpgraderFromV4ToV5.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include "UpgraderBase.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct UpgraderFromV4ToV5 : private UpgraderBase
    {
    private:
        struct UpgradeInstance;

    private:
        BentleyStatus GetInstancesWithAppliedChanges(bvector<UpgradeInstance>& changedInstancesOut);

        void ApplyChangesToRootInstance(JsonValueR root);
        void ApplyChangesToCachedFileInfoInstance(JsonValueR info);
        void ApplyChangesToCachedInstanceInfoInstance(JsonValueR info);
        void ApplyChangesToChangeInfoStruct(JsonValueR changeInfoStruct);

        BentleyStatus ReadInstances(bvector<UpgradeInstance>& instancesOut, ECClassCP ecClass);
        BentleyStatus UpdateInstances(const bvector<UpgradeInstance>& instances);

        bvector<ECClassCP> GetDataSourceNodeClasses(ECSchemaCR ecSchema);
        bool IsDataSourceObjectClass(ECClassCP ecClass);
        void CreateWeakRootRelationship(ECSchemaR schema, ECSchemaCR cacheSchema, const bvector<ECClassCP>& childClasses);

    public:
        UpgraderFromV4ToV5(ECDbAdapter& adapter);
        BentleyStatus Upgrade();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
