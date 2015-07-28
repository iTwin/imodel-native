/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/MockECDbAdapter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>

#ifdef USE_GTEST
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockECDbAdapter : public IECDbAdapter
    {
    public:
        MOCK_METHOD0 (GetECDb,
            ObservableECDb& ());
        MOCK_METHOD0 (GetECInstanceFinder,
            ECInstanceFinder& ());
        MOCK_METHOD1 (GetECSchema,
            ECSchemaP (Utf8StringCR schemaName));
        MOCK_METHOD1 (HasECSchema,
            bool (Utf8StringCR schemaName));
        MOCK_METHOD1 (GetECClass,
            ECClassP (Utf8StringCR classKey));
        MOCK_METHOD1 (GetECClass,
            ECClassP (ECClassId classId));
        MOCK_METHOD2 (GetECClass,
            ECClassP (Utf8StringCR schemaName, Utf8StringCR className));
        MOCK_METHOD1 (GetECClass,
            ECClassP (ECInstanceKeyCR instanceKey));
        MOCK_METHOD1 (GetECClass,
            ECClassP (ObjectIdCR objectId));
        MOCK_METHOD1 (GetECClasses,
            bvector<ECClassCP> (const ECInstanceKeyMultiMap& instanceMultiMap));
        MOCK_METHOD1 (GetECRelationshipClass,
            ECRelationshipClassP (Utf8StringCR classKey));
        MOCK_METHOD1 (GetECRelationshipClass,
            ECRelationshipClassP (ECClassId classId));
        MOCK_METHOD2 (GetECRelationshipClass,
            ECRelationshipClassP (Utf8StringCR schemaName, Utf8StringCR className));
        MOCK_METHOD1 (GetECRelationshipClass,
            ECRelationshipClassP (ECInstanceKeyCR instanceKey));
        MOCK_METHOD1 (GetECRelationshipClass,
            ECRelationshipClassP (ObjectIdCR objectId));
        MOCK_METHOD2 (FindRelationshipClasses,
            bvector<ECRelationshipClassCP> (ECClassId sourceClassId, ECClassId targetClassId));
        MOCK_METHOD2 (FindRelationshipClassesWithSource,
            bvector<ECRelationshipClassCP> (ECClassId sourceClassId, Utf8String schemaName));
        MOCK_METHOD3 (FindRelationshipClassesInSchema,
            bvector<ECRelationshipClassCP> (ECClassId sourceClassId, ECClassId targetClassId, Utf8String schemaName));
        MOCK_METHOD2 (FindRelationshipClassWithSource,
            ECRelationshipClassCP (ECClassId sourceClassId, ECClassId targetClassId));
        MOCK_METHOD2 (FindRelationshipClassWithTarget,
            ECRelationshipClassCP (ECClassId sourceClassId, ECClassId targetClassId));
        MOCK_METHOD1 (GetInstanceKeyFromJsonInstance,
            ECInstanceKey (JsonValueCR ecInstanceJson));
        MOCK_METHOD2 (PrepareStatement,
            BentleyStatus (ECSqlStatement& statement, ECSqlBuilderCR builder));
        MOCK_METHOD2 (PrepareStatement,
            BentleyStatus (ECSqlStatement& statement, Utf8StringCR ecsql));
        MOCK_METHOD3 (BindParameters,
            BentleyStatus (ECSqlStatement& statement, const bvector<Utf8String>& parameters, IECSqlBinder::MakeCopy makeCopy));
        MOCK_METHOD4 (ExtractJsonInstanceArrayFromStatement,
            BentleyStatus (ECSqlStatement&, ECClassCP, JsonValueR, ICancellationTokenPtr));
        MOCK_METHOD3 (ExtractJsonInstanceFromStatement,
            BentleyStatus (ECSqlStatement& statement, ECClassCP ecClass, JsonValueR jsonInstanceOut));
        MOCK_METHOD4 (ExtractECIdsFromStatement,
            BentleyStatus (ECSqlStatement& statement, int ecInstanceIdcolumn, bvector<ECInstanceId>& ecIdsOut, ICancellationTokenPtr cancellationToken));
        MOCK_METHOD5 (ExtractECInstanceKeyMultiMapFromStatement,
            BentleyStatus (ECSqlStatement& statement, int ecInstanceIdcolumn, ECClassId classId, ECInstanceKeyMultiMap& keysOut, ICancellationTokenPtr cancellationToken));
        MOCK_METHOD1 (CountClassInstances,
            int (ECClassCP ecClass));
        MOCK_METHOD2 (FindInstance,
            ECInstanceId (ECClassCP, Utf8CP));
        MOCK_METHOD2 (FindInstances,
            bset<ECInstanceId> (ECClassCP, Utf8CP));
        MOCK_METHOD2 (GetJsonInstance,
            BentleyStatus (JsonValueR objectOut, ECInstanceKeyCR instanceKey));
        MOCK_METHOD3 (GetJsonInstance,
            BentleyStatus (JsonValueR objectOut, ECClassCP ecClass, ECInstanceId ecId));
        MOCK_METHOD4 (GetJsonInstance,
            BentleyStatus (JsonValueR, ECClassCP, Utf8CP, Utf8CP));
        MOCK_METHOD4 (GetJsonInstances,
            BentleyStatus (JsonValueR, ECClassCP, Utf8CP, ICancellationTokenPtr));
        MOCK_METHOD4 (GetJsonInstances,
            BentleyStatus (JsonValueR, ECClassCP, ECSqlStatement&, ICancellationTokenPtr));
        MOCK_METHOD3 (RelateInstances,
            ECInstanceKey (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target));
        MOCK_METHOD4 (GetRelatedTargetIds,
            BentleyStatus (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECClassCP targetClass, bvector<ECInstanceId>& ecIdsOut));
        MOCK_METHOD4 (GetRelatedSourceIds,
            BentleyStatus (ECRelationshipClassCP relClass, ECClassCP sourceClass, bvector<ECInstanceId>& idsOut, ECInstanceKeyCR target));
        MOCK_METHOD4 (GetJsonRelatedSources,
            BentleyStatus (JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP sourceClass, ECInstanceKeyCR target));
        MOCK_METHOD5 (GetJsonRelatedTargets,
            BentleyStatus (JsonValueR, ECRelationshipClassCP, ECClassCP, ECInstanceKeyCR, Utf8CP));
        MOCK_METHOD3 (GetRelatedTargetKeys,
            BentleyStatus (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyMultiMap& keysOut));
        MOCK_METHOD3 (FindRelationship,
            ECInstanceKey (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target));
        MOCK_METHOD3 (HasRelationship,
            bool (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target));
        MOCK_METHOD3 (DeleteRelationship,
            BentleyStatus (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif