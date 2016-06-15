/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ECDbAdapter.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/IECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlAdapterCache.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbAdapter : public IECDbAdapter, public IECDbSchemaChangeListener
    {
    private:
        ObservableECDb* m_ecDb;

        ECSqlAdapterCache<ECInstanceInserter> m_inserters;
        ECSqlStatementCache m_statementCache;
        std::shared_ptr<Statement> m_findRelationshipClassesStatement;
        std::shared_ptr<ECInstanceFinder> m_finder;
        bset<DeleteListener*> m_deleteListeners;

    private:
        static bool DoesConstraintSupportECClass(ECRelationshipConstraintCR constraint, ECClassCR ecClass, bool allowPolymorphic);
        static int FindDistanceFromBaseClass(ECClassCP ecClass, ECClassCP targetClass, int dist = 0);
        BentleyStatus FindInstancesBeingDeleted
            (
            const ECInstanceKeyMultiMap& seedInstancesBeingDeleted,
            bset<ECInstanceKey>& allInstancesBeingDeletedOut
            );
        BentleyStatus FindInstancesBeingDeleted
            (
            ECInstanceKeyCR instanceToDelete,
            bset<ECInstanceKey>& allInstancesBeingDeletedOut
            );
        BentleyStatus FindInstancesBeingDeletedForRelationship
            (
            ECRelationshipClassCR relClass,
            ECInstanceKeyCR instanceToDelete,
            bset<ECInstanceKey>& allInstancesBeingDeletedOut
            );
        size_t CountHoldingParents(ECInstanceKeyCR instanceKey, const bset<ECInstanceKey>* parentsToIgnore);
        BentleyStatus OnBeforeDelete(ECClassCR ecClass, ECInstanceId instanceId, bset<ECInstanceKey>& additionalToDeleteOut);

        BentleyStatus DeleteInstance(ECClassId classId, ECInstanceId instanceId);

    public:
        WSCACHE_EXPORT ECDbAdapter(ObservableECDb& ecDb);
        WSCACHE_EXPORT virtual ~ECDbAdapter();

        //! React to ECDb schema change and clear runtime caches
        virtual void OnSchemaChanged() override;

        WSCACHE_EXPORT ObservableECDb& GetECDb() override;
        WSCACHE_EXPORT ECInstanceFinder& GetECInstanceFinder() override;

        WSCACHE_EXPORT ECSchemaCP GetECSchema(Utf8StringCR schemaName) override;
        WSCACHE_EXPORT bool HasECSchema(Utf8StringCR schemaName) override;

        WSCACHE_EXPORT ECClassCP GetECClass(Utf8StringCR classKey) override;
        WSCACHE_EXPORT ECClassCP GetECClass(ECClassId classId) override;
        WSCACHE_EXPORT ECClassCP GetECClass(Utf8StringCR schemaName, Utf8StringCR className) override;
        WSCACHE_EXPORT ECClassCP GetECClass(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT ECClassCP GetECClass(ObjectIdCR objectId) override;

        WSCACHE_EXPORT bvector<ECClassCP> GetECClasses(const ECInstanceKeyMultiMap& instanceMultiMap) override;

        WSCACHE_EXPORT ECRelationshipClassCP GetECRelationshipClass(Utf8StringCR classKey) override;
        WSCACHE_EXPORT ECRelationshipClassCP GetECRelationshipClass(ECClassId classId) override;
        WSCACHE_EXPORT ECRelationshipClassCP GetECRelationshipClass(Utf8StringCR schemaName, Utf8StringCR className) override;
        WSCACHE_EXPORT ECRelationshipClassCP GetECRelationshipClass(ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT ECRelationshipClassCP GetECRelationshipClass(ObjectIdCR objectId) override;

        WSCACHE_EXPORT bvector<ECRelationshipClassCP> FindRelationshipClasses(ECClassId sourceClassId, ECClassId targetClassId) override;
        WSCACHE_EXPORT bvector<ECRelationshipClassCP> FindRelationshipClassesWithSource(ECClassId sourceClassId, Utf8String schemaName) override;
        WSCACHE_EXPORT bvector<ECRelationshipClassCP> FindRelationshipClassesInSchema(ECClassId sourceClassId, ECClassId targetClassId, Utf8String schemaName) override;
        WSCACHE_EXPORT ECRelationshipClassCP FindRelationshipClassWithSource(ECClassId sourceClassId, ECClassId targetClassId) override;
        WSCACHE_EXPORT ECRelationshipClassCP FindRelationshipClassWithTarget(ECClassId sourceClassId, ECClassId targetClassId) override;
        WSCACHE_EXPORT ECRelationshipClassCP FindClosestRelationshipClassWithSource(ECClassId sourceClassId, ECClassId targetClassId) override;

        WSCACHE_EXPORT ECInstanceKey GetInstanceKeyFromJsonInstance(JsonValueCR ecInstanceJson) override;

        //! Prepare statement. Assert and return error if failed
        WSCACHE_EXPORT BentleyStatus PrepareStatement(ECSqlStatement& statement, Utf8StringCR ecsql) override;

        //! Selects as few properties as possible to acomplish valid query
        WSCACHE_EXPORT BentleyStatus BindParameters(ECSqlStatement& statement, const bvector<Utf8String>& parameters, IECSqlBinder::MakeCopy makeCopy) override;
        WSCACHE_EXPORT BentleyStatus ExtractJsonInstanceArrayFromStatement
            (
            ECSqlStatement& statement,
            ECClassCP ecClass,
            JsonValueR jsonInstancesArrayOut,
            ICancellationTokenPtr ct = nullptr
            ) override;
        WSCACHE_EXPORT BentleyStatus ExtractJsonInstanceFromStatement(ECSqlStatement& statement, ECClassCP ecClass, JsonValueR jsonInstanceOut) override;
        WSCACHE_EXPORT BentleyStatus ExtractECIdsFromStatement
            (
            ECSqlStatement& statement,
            int ecInstanceIdcolumn,
            bvector<ECInstanceId>& ecIdsOut,
            ICancellationTokenPtr ct = nullptr
            ) override;
        WSCACHE_EXPORT BentleyStatus ExtractECInstanceKeyMultiMapFromStatement
            (
            ECSqlStatement& statement,
            int ecInstanceIdcolumn,
            ECClassId classId,
            ECInstanceKeyMultiMap& keysOut,
            ICancellationTokenPtr ct = nullptr
            ) override;

        WSCACHE_EXPORT int  CountClassInstances(ECClassCP ecClass) override;
        WSCACHE_EXPORT ECInstanceId FindInstance(ECClassCP ecClass, Utf8CP whereClause = nullptr) override;
        WSCACHE_EXPORT bset<ECInstanceId> FindInstances(ECClassCP ecClass, Utf8CP whereClause = nullptr) override;

        WSCACHE_EXPORT BentleyStatus GetJsonInstance(JsonValueR objectOut, ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT BentleyStatus GetJsonInstance(JsonValueR objectOut, ECClassCP ecClass, ECInstanceId ecId) override;
        WSCACHE_EXPORT BentleyStatus GetJsonInstance(JsonValueR objectOut, ECClassCP ecClass, Utf8CP whereClause = nullptr, Utf8CP select = nullptr) override;

        WSCACHE_EXPORT BentleyStatus GetJsonInstances(JsonValueR arrayOut, ECClassCP ecClass, Utf8CP whereClause = nullptr, ICancellationTokenPtr ct = nullptr) override;
        WSCACHE_EXPORT BentleyStatus GetJsonInstances(JsonValueR arrayOut, ECClassCP ecClass, ECSqlStatement& statement, ICancellationTokenPtr ct = nullptr) override;

        WSCACHE_EXPORT ECInstanceKey RelateInstances(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;

        WSCACHE_EXPORT BentleyStatus GetRelatedTargetIds(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECClassCP targetClass, bvector<ECInstanceId>& ecIdsOut) override;
        WSCACHE_EXPORT BentleyStatus GetRelatedSourceIds(ECRelationshipClassCP relClass, ECClassCP sourceClass, bvector<ECInstanceId>& idsOut, ECInstanceKeyCR target) override;
        WSCACHE_EXPORT BentleyStatus GetJsonRelatedSources(JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP sourceClass, ECInstanceKeyCR target) override;
        WSCACHE_EXPORT BentleyStatus GetJsonRelatedTargets(JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP targetClass, ECInstanceKeyCR source, Utf8CP orderBy = nullptr) override;

        WSCACHE_EXPORT BentleyStatus GetRelatedTargetKeys(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyMultiMap& keysOut) override;

        WSCACHE_EXPORT ECInstanceKey FindRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;
        WSCACHE_EXPORT bool HasRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;

        WSCACHE_EXPORT BentleyStatus DeleteInstances(const ECInstanceKeyMultiMap& instances) override;
        WSCACHE_EXPORT BentleyStatus DeleteRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;

        WSCACHE_EXPORT void RegisterDeleteListener(DeleteListener* listener) override;
        WSCACHE_EXPORT void UnRegisterDeleteListener(DeleteListener* listener) override;
    };

typedef ECDbAdapter& ECDbAdapterR;

END_BENTLEY_WEBSERVICES_NAMESPACE
