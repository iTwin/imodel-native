/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ECDbAdapter.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/IECDbAdapter.h>
#include <WebServices/Cache/Util/ECSqlAdapterCache.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

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
        std::shared_ptr<Statement> m_findRelationshipClassesWithSourceStatement;
        std::shared_ptr<Statement> m_findRelationshipClassesInSchemaStatement;
        std::shared_ptr<ECInstanceFinder> m_finder;

    public:
        WSCACHE_EXPORT ECDbAdapter (ObservableECDb& ecDb);
        WSCACHE_EXPORT virtual ~ECDbAdapter ();

        //! React to ECDb schema change and clear runtime caches
        virtual void OnSchemaChanged () override;

        WSCACHE_EXPORT ObservableECDb& GetECDb () override;
        WSCACHE_EXPORT ECInstanceFinder& GetECInstanceFinder () override;

        WSCACHE_EXPORT ECSchemaP GetECSchema (Utf8StringCR schemaName) override;
        WSCACHE_EXPORT bool HasECSchema (Utf8StringCR schemaName) override;

        WSCACHE_EXPORT ECClassP GetECClass (Utf8StringCR classKey) override;
        WSCACHE_EXPORT ECClassP GetECClass (ECClassId classId) override;
        WSCACHE_EXPORT ECClassP GetECClass (Utf8StringCR schemaName, Utf8StringCR className) override;
        WSCACHE_EXPORT ECClassP GetECClass (ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT ECClassP GetECClass (ObjectIdCR objectId) override;

        WSCACHE_EXPORT bvector<ECClassCP> GetECClasses (const ECInstanceKeyMultiMap& instanceMultiMap) override;

        WSCACHE_EXPORT ECRelationshipClassP GetECRelationshipClass (Utf8StringCR classKey) override;
        WSCACHE_EXPORT ECRelationshipClassP GetECRelationshipClass (ECClassId classId) override;
        WSCACHE_EXPORT ECRelationshipClassP GetECRelationshipClass (Utf8StringCR schemaName, Utf8StringCR className) override;
        WSCACHE_EXPORT ECRelationshipClassP GetECRelationshipClass (ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT ECRelationshipClassP GetECRelationshipClass (ObjectIdCR objectId) override;

        WSCACHE_EXPORT bvector<ECRelationshipClassCP> FindRelationshipClasses (ECClassId sourceClassId, ECClassId targetClassId) override;
        WSCACHE_EXPORT bvector<ECRelationshipClassCP> FindRelationshipClassesWithSource (ECClassId sourceClassId, Utf8String schemaName) override;
        WSCACHE_EXPORT bvector<ECRelationshipClassCP> FindRelationshipClassesInSchema (ECClassId sourceClassId, ECClassId targetClassId, Utf8String schemaName) override;
        WSCACHE_EXPORT ECRelationshipClassCP FindRelationshipClassWithSource (ECClassId sourceClassId, ECClassId targetClassId) override;
        WSCACHE_EXPORT ECRelationshipClassCP FindRelationshipClassWithTarget (ECClassId sourceClassId, ECClassId targetClassId) override;

        WSCACHE_EXPORT ECInstanceKey GetInstanceKeyFromJsonInstance (JsonValueCR ecInstanceJson) override;

        WSCACHE_EXPORT BentleyStatus PrepareStatement (ECSqlStatement& statement, ECSqlBuilderCR builder) override;
        WSCACHE_EXPORT BentleyStatus PrepareStatement (ECSqlStatement& statement, Utf8StringCR ecsql) override;

        //! Selects as few properties as possible to acomplish valid query
        WSCACHE_EXPORT BentleyStatus BindParameters (ECSqlStatement& statement, const bvector<Utf8String>& parameters, IECSqlBinder::MakeCopy makeCopy) override;
        WSCACHE_EXPORT BentleyStatus ExtractJsonInstanceArrayFromStatement
            (
            ECSqlStatement& statement,
            ECClassCP ecClass,
            JsonValueR jsonInstancesArrayOut,
            ICancellationTokenPtr cancellationToken = nullptr
            ) override;
        WSCACHE_EXPORT BentleyStatus ExtractJsonInstanceFromStatement (ECSqlStatement& statement, ECClassCP ecClass, JsonValueR jsonInstanceOut) override;
        WSCACHE_EXPORT BentleyStatus ExtractECIdsFromStatement
            (
            ECSqlStatement& statement,
            int ecInstanceIdcolumn,
            bvector<ECInstanceId>& ecIdsOut,
            ICancellationTokenPtr cancellationToken = nullptr
            ) override;
        WSCACHE_EXPORT BentleyStatus ExtractECInstanceKeyMultiMapFromStatement
            (
            ECSqlStatement& statement,
            int ecInstanceIdcolumn,
            ECClassId classId,
            ECInstanceKeyMultiMap& keysOut,
            ICancellationTokenPtr cancellationToken = nullptr
            ) override;

        WSCACHE_EXPORT int  CountClassInstances (ECClassCP ecClass) override;
        WSCACHE_EXPORT ECInstanceId FindInstance (ECClassCP ecClass, Utf8CP whereQuery = nullptr) override;
        WSCACHE_EXPORT bset<ECInstanceId> FindInstances (ECClassCP ecClass, Utf8CP whereQuery = nullptr) override;

        WSCACHE_EXPORT BentleyStatus GetJsonInstance (JsonValueR objectOut, ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT BentleyStatus GetJsonInstance (JsonValueR objectOut, ECClassCP ecClass, ECInstanceId ecId) override;
        WSCACHE_EXPORT BentleyStatus GetJsonInstance (JsonValueR objectOut, ECClassCP ecClass, Utf8CP whereQuery = nullptr, Utf8CP select = nullptr) override;

        WSCACHE_EXPORT BentleyStatus GetJsonInstances (JsonValueR arrayOut, ECClassCP ecClass, Utf8CP whereQuery = nullptr, ICancellationTokenPtr cancellationToken = nullptr) override;
        WSCACHE_EXPORT BentleyStatus GetJsonInstances (JsonValueR arrayOut, ECClassCP ecClass, ECSqlStatement& statement, ICancellationTokenPtr cancellationToken = nullptr) override;

        WSCACHE_EXPORT ECInstanceKey RelateInstances (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;

        WSCACHE_EXPORT BentleyStatus GetRelatedTargetIds (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECClassCP targetClass, bvector<ECInstanceId>& ecIdsOut) override;
        WSCACHE_EXPORT BentleyStatus GetRelatedSourceIds (ECRelationshipClassCP relClass, ECClassCP sourceClass, bvector<ECInstanceId>& idsOut, ECInstanceKeyCR target) override;
        WSCACHE_EXPORT BentleyStatus GetJsonRelatedSources (JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP sourceClass, ECInstanceKeyCR target) override;
        WSCACHE_EXPORT BentleyStatus GetJsonRelatedTargets (JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP targetClass, ECInstanceKeyCR source, Utf8CP orderBy = nullptr) override;

        WSCACHE_EXPORT BentleyStatus GetRelatedTargetKeys (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyMultiMap& keysOut) override;

        WSCACHE_EXPORT ECInstanceKey FindRelationship (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;
        WSCACHE_EXPORT bool HasRelationship (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;

        WSCACHE_EXPORT BentleyStatus DeleteRelationship (ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) override;
    };

typedef ECDbAdapter& ECDbAdapterR;

END_BENTLEY_WEBSERVICES_NAMESPACE
