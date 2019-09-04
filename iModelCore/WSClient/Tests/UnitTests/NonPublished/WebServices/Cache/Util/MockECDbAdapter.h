/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct MockECDbAdapter : IECDbAdapter
    {
    MOCK_METHOD0 (GetECDb,
        ObservableECDb& ());
    MOCK_METHOD0 (GetECInstanceFinder,
        ECInstanceFinder& ());
    MOCK_METHOD1 (GetECSchema,
        ECSchemaCP (Utf8StringCR schemaName));
    MOCK_METHOD1 (HasECSchema,
        bool (Utf8StringCR schemaName));
    MOCK_METHOD1 (GetECClass,
        ECClassCP (Utf8StringCR classKey));
    MOCK_METHOD1 (GetECClass,
        ECClassCP (ECClassId classId));
    MOCK_METHOD2 (GetECClass,
        ECClassCP (Utf8StringCR schemaName, Utf8StringCR className));
    MOCK_METHOD1 (GetECClass,
        ECClassCP (ECInstanceKeyCR instanceKey));
    MOCK_METHOD1 (GetECClass,
        ECClassCP (ObjectIdCR objectId));
    MOCK_METHOD1 (GetECClasses,
        bvector<ECClassCP> (const ECInstanceKeyMultiMap& instanceMultiMap));
    MOCK_METHOD1 (GetECRelationshipClass,
        ECRelationshipClassCP (Utf8StringCR classKey));
    MOCK_METHOD1 (GetECRelationshipClass,
        ECRelationshipClassCP (ECClassId classId));
    MOCK_METHOD2 (GetECRelationshipClass,
        ECRelationshipClassCP (Utf8StringCR schemaName, Utf8StringCR className));
    MOCK_METHOD1 (GetECRelationshipClass,
        ECRelationshipClassCP (ECInstanceKeyCR instanceKey));
    MOCK_METHOD1 (GetECRelationshipClass,
        ECRelationshipClassCP (ObjectIdCR objectId));
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
    MOCK_METHOD2 (FindClosestRelationshipClassWithSource, 
        ECRelationshipClassCP (ECClassId sourceClassId, ECClassId targetClassId));
    MOCK_METHOD1 (GetInstanceKeyFromJsonInstance,
        ECInstanceKey (JsonValueCR ecInstanceJson));
    MOCK_METHOD1(GetInstanceKeyFromJsonInstance,
        ECInstanceKey(RapidJsonValueCR ecInstanceJson));
    MOCK_METHOD2 (PrepareStatement,
        BentleyStatus (ECSqlStatement&, Utf8StringCR ecsql));
    MOCK_METHOD3 (BindParameters,
        BentleyStatus (ECSqlStatement&, const bvector<Utf8String>& parameters, IECSqlBinder::MakeCopy makeCopy));
    MOCK_METHOD4 (ExtractJsonInstanceArrayFromStatement,
        BentleyStatus (ECSqlStatement&, ECClassCP, JsonValueR, ICancellationTokenPtr));
    MOCK_METHOD3 (ExtractJsonInstanceFromStatement,
        BentleyStatus (ECSqlStatement&, ECClassCP ecClass, JsonValueR jsonInstanceOut));
    MOCK_METHOD4 (ExtractECIdsFromStatement,
        BentleyStatus (ECSqlStatement&, int ecInstanceIdcolumn, bvector<ECInstanceId>& ecIdsOut, ICancellationTokenPtr ct));
    MOCK_METHOD5 (ExtractECInstanceKeyMultiMapFromStatement,
        BentleyStatus (ECSqlStatement&, int ecInstanceIdcolumn, ECClassId classId, ECInstanceKeyMultiMap& keysOut, ICancellationTokenPtr ct));
    MOCK_METHOD5(ExtractECInstanceKeys,
        BentleyStatus (ECSqlStatement&, ECInstanceKeyMultiMap& keysOut, ICancellationTokenPtr, int ecClassIdColumn, int ecInstanceIdcolumn));
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
    MOCK_METHOD3(GetRelatedSourceKeys,
                    BentleyStatus(ECRelationshipClassCP relClass, ECInstanceKeyCR target, ECInstanceKeyMultiMap& keysOut));
    MOCK_METHOD3 (FindRelationship,
        ECInstanceKey (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target));
    MOCK_METHOD3 (HasRelationship,
        bool (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target));
    MOCK_METHOD3 (DeleteRelationship,
        BentleyStatus (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target));
    MOCK_METHOD1(DeleteInstance,
        BentleyStatus(ECInstanceKeyCR));
    MOCK_METHOD1(DeleteInstances,
        BentleyStatus(const ECInstanceKeyMultiMap&));
    MOCK_METHOD1(RegisterDeleteListener,
        void(DeleteListener*));
    MOCK_METHOD1(UnRegisterDeleteListener,
        void(DeleteListener*));
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
#endif