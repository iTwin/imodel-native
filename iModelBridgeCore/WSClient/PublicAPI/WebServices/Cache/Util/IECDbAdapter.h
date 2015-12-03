/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/IECDbAdapter.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>

#include <Bentley/Bentley.h>
#include <Bentley/bset.h>
#include <ECDb/ECSqlStatement.h>
#include <ECObjects/ECObjectsAPI.h>
#include <MobileDgn/Utils/Threading/CancellationToken.h>
#include <WebServices/Cache/Util/ObservableECDb.h>
#include <WebServices/Client/ObjectId.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<Savepoint> SavepointPtr;

struct EXPORT_VTABLE_ATTRIBUTE IECDbAdapter
    {
    public:
        virtual ~IECDbAdapter()
            {};

        //! Get direct access to ECDb
        virtual ObservableECDb& GetECDb() = 0;

        //! Get reusable ECInstanceFinder. Reusing same finder allows speeding up relationship traversion.
        virtual ECInstanceFinder& GetECInstanceFinder() = 0;

        // Load whole schema. Large schemas take time to load so consider loading seperate classes instead.
        virtual ECSchemaP GetECSchema(Utf8StringCR schemaName) = 0;
        // Check if ECDb has schema without loading it.
        virtual bool HasECSchema(Utf8StringCR schemaName) = 0;

        virtual ECClassP GetECClass(Utf8StringCR classKey) = 0;
        virtual ECClassP GetECClass(ECClassId classId) = 0;
        virtual ECClassP GetECClass(Utf8StringCR schemaName, Utf8StringCR className) = 0;
        virtual ECClassP GetECClass(ECInstanceKeyCR instanceKey) = 0;
        virtual ECClassP GetECClass(ObjectIdCR objectId) = 0;

        virtual bvector<ECClassCP> GetECClasses(const ECInstanceKeyMultiMap& instanceMultiMap) = 0;

        virtual ECRelationshipClassP GetECRelationshipClass(Utf8StringCR classKey) = 0;
        virtual ECRelationshipClassP GetECRelationshipClass(ECClassId classId) = 0;
        virtual ECRelationshipClassP GetECRelationshipClass(Utf8StringCR schemaName, Utf8StringCR className) = 0;
        virtual ECRelationshipClassP GetECRelationshipClass(ECInstanceKeyCR instanceKey) = 0;
        virtual ECRelationshipClassP GetECRelationshipClass(ObjectIdCR objectId) = 0;

        //! Find relationship classes by end classes polymorphically
        virtual bvector<ECRelationshipClassCP> FindRelationshipClasses(ECClassId sourceClassId, ECClassId targetClassId) = 0;

        //! Find relationship classes that matches given source or target class polymorphically
        //! TODO: change misleading method name to to FindRelationshipClassesWithEnd
        virtual bvector<ECRelationshipClassCP> FindRelationshipClassesWithSource(ECClassId endClassId, Utf8String schemaName) = 0;

        //! Find relationship classes from given schema by end classes polymorphically
        virtual bvector<ECRelationshipClassCP> FindRelationshipClassesInSchema(ECClassId sourceClassId, ECClassId targetClassId, Utf8String schemaName) = 0;

        //! Find relationship class that matches source exactly and target polymorphically. Will return null if found more or none.
        virtual ECRelationshipClassCP FindRelationshipClassWithSource(ECClassId sourceClassId, ECClassId targetClassId) = 0;

        //! Find relationship class that matches target exactly and source polymorphically. Will return null if found more or none.
        virtual ECRelationshipClassCP FindRelationshipClassWithTarget(ECClassId sourceClassId, ECClassId targetClassId) = 0;

        //! Find relationship class that matches target exactly and source polymorphically. Will return closest matching if found more
        virtual ECRelationshipClassCP FindClosestRelationshipClassWithSource(ECClassId sourceClassId, ECClassId targetClassId) = 0;

        virtual ECInstanceKey GetInstanceKeyFromJsonInstance(JsonValueCR ecInstanceJson) = 0;

        //! DEPRECATED
        virtual BentleyStatus PrepareStatement(ECSqlStatement& statement, ECSqlBuilderCR builder) = 0;

        virtual BentleyStatus PrepareStatement(ECSqlStatement& statement, Utf8StringCR ecsql) = 0;

        //! Selects as few properties as possible to acomplish valid query
        virtual BentleyStatus BindParameters(ECSqlStatement& statement, const bvector<Utf8String>& parameters, IECSqlBinder::MakeCopy makeCopy) = 0;
        virtual BentleyStatus ExtractJsonInstanceArrayFromStatement
            (
            ECSqlStatement& statement,
            ECClassCP ecClass,
            JsonValueR jsonInstancesArrayOut,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;
        virtual BentleyStatus ExtractJsonInstanceFromStatement(ECSqlStatement& statement, ECClassCP ecClass, JsonValueR jsonInstanceOut) = 0;
        virtual BentleyStatus ExtractECIdsFromStatement
            (
            ECSqlStatement& statement,
            int ecInstanceIdcolumn,
            bvector<ECInstanceId>& ecIdsOut,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        virtual BentleyStatus ExtractECInstanceKeyMultiMapFromStatement
            (
            ECSqlStatement& statement,
            int ecInstanceIdcolumn,
            ECClassId classId,
            ECInstanceKeyMultiMap& keysOut,
            ICancellationTokenPtr cancellationToken = nullptr
            ) = 0;

        virtual int  CountClassInstances(ECClassCP ecClass) = 0;
        virtual ECInstanceId FindInstance(ECClassCP ecClass, Utf8CP whereClause = nullptr) = 0;
        virtual bset<ECInstanceId> FindInstances(ECClassCP ecClass, Utf8CP whereClause = nullptr) = 0;

        virtual BentleyStatus GetJsonInstance(JsonValueR objectOut, ECInstanceKeyCR instanceKey) = 0;
        virtual BentleyStatus GetJsonInstance(JsonValueR objectOut, ECClassCP ecClass, ECInstanceId ecId) = 0;
        virtual BentleyStatus GetJsonInstance(JsonValueR objectOut, ECClassCP ecClass, Utf8CP whereClause = nullptr, Utf8CP select = nullptr) = 0;

        virtual BentleyStatus GetJsonInstances(JsonValueR arrayOut, ECClassCP ecClass, Utf8CP whereClause = nullptr, ICancellationTokenPtr cancellationToken = nullptr) = 0;
        virtual BentleyStatus GetJsonInstances(JsonValueR arrayOut, ECClassCP ecClass, ECSqlStatement& statement, ICancellationTokenPtr cancellationToken = nullptr) = 0;

        virtual ECInstanceKey RelateInstances(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) = 0;

        virtual BentleyStatus GetRelatedTargetIds(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECClassCP targetClass, bvector<ECInstanceId>& ecIdsOut) = 0;
        virtual BentleyStatus GetRelatedSourceIds(ECRelationshipClassCP relClass, ECClassCP sourceClass, bvector<ECInstanceId>& idsOut, ECInstanceKeyCR target) = 0;
        virtual BentleyStatus GetJsonRelatedSources(JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP sourceClass, ECInstanceKeyCR target) = 0;
        virtual BentleyStatus GetJsonRelatedTargets(JsonValueR arrayOut, ECRelationshipClassCP relClass, ECClassCP targetClass, ECInstanceKeyCR source, Utf8CP orderBy = nullptr) = 0;

        virtual BentleyStatus GetRelatedTargetKeys(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyMultiMap& keysOut) = 0;

        virtual ECInstanceKey FindRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) = 0;
        virtual bool HasRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) = 0;

        virtual BentleyStatus DeleteRelationship(ECRelationshipClassCP relClass, ECInstanceKeyCR source, ECInstanceKeyCR target) = 0;
    };

typedef IECDbAdapter& IECDbAdapterR;

END_BENTLEY_WEBSERVICES_NAMESPACE
