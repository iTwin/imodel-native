/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/CacheQueryHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/ISelectProvider.h>
#include <WebServices/Cache/Util/IECDbAdapter.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECObjects.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheQueryHelper
    {
    public:
        struct ClassReadInfo;
        typedef std::function<BentleyStatus(const CacheQueryHelper::ClassReadInfo& info, ECSqlStatement& statement)> ReadCallback;

    private:
        const ISelectProvider& m_selectProvider;

    private:
        static Utf8String CreateSelectClause
            (
            const ISelectProvider::SelectProperties& selectProperties,
            Utf8CP classAlias,
            Utf8CP infoAlias,
            bool* pInfoNeedsSelecting
            );

        static Utf8String CreateOrderByClause
            (
            const ISelectProvider::SortProperties& sortProperties,
            ECClassCR ecClass,
            Utf8CP classAlias
            );

        static bool IsTypeString(ECPropertyCR property);
        static int GetRemoteIdColumnIndex(const ClassReadInfo& info, bool& columnNeedsVerification);

    public:
        WSCACHE_EXPORT CacheQueryHelper(const ISelectProvider& selectProvider);

        //! Create ordered list of class read information. Each one includes ordered list of selected and order by properties.
        //! specify classLocater if you want to get read infos for classes polymorphicly.
        //! TODO: revise design how to deal with polymorphic reads
        WSCACHE_EXPORT bvector<ClassReadInfo> CreateReadInfos(ECClassCP ecClass) const;
        WSCACHE_EXPORT bvector<ClassReadInfo> CreateReadInfos(bvector<ECClassCP> ecClasses) const;
        WSCACHE_EXPORT bvector<ClassReadInfo> CreateReadInfos(const bvector<ECClassP>& ecClasses) const;

        //! Read all instances from map, extract EC data from statement using custom callback or use reader function from below
        WSCACHE_EXPORT BentleyStatus ReadInstances
            (
            IECDbAdapter& dbAdapter,
            const ECInstanceKeyMultiMap& instances,
            const ReadCallback& readCallback
            );

        //! Read one json instance  and handle selected RemoteId property. Does not do stepping of statmenet.
        WSCACHE_EXPORT static BentleyStatus ReadJsonInstance
            (
            const ClassReadInfo& info,
            ECSqlStatement& statement,
            JsonValueR jsonInstanceOut,
            ICancellationTokenPtr ct = nullptr
            );

        //! Reader function specificlly to handle selected RemoteId property
        WSCACHE_EXPORT static BentleyStatus ReadJsonInstances
            (
            const ClassReadInfo& info,
            ECSqlStatement& statement,
            JsonValueR jsonInstancesArrayOut,
            ICancellationTokenPtr ct = nullptr
            );

        //! Functions for creating different purpose ECSql
        //! Note about RemoteId - it is not considred to be instance property, but rather special property that needs different queries.
        struct ECSql
            {
            //! Create ECSql to select properties from related target instances
            //! ECSql columns:    {From ClassReadInfo}
            //! ECSql parameters: SourceClassInstance.ECInstanceId
            WSCACHE_EXPORT static Utf8String SelectPropertiesByRelatedSourceECInstanceId
                (
                const ClassReadInfo& info,
                ECClassCR sourceClass,
                ECRelationshipClassCR relationshipClass
                );

            //! Create ECSql to select properties from instances by their remote ids. Use "instance" alias in optional where clause
            //! ECSql columns:    {From ClassReadInfo}
            //! ECSql parameters: none
            WSCACHE_EXPORT static Utf8String SelectPropertiesByRemoteIds
                (
                const ClassReadInfo& info,
                Utf8StringCR commaSeperatedRemoteIds,
                Utf8StringCR optionalWhereClause = ""
                );

            //! Create ECSql to select properties from instance by its remote id
            //! ECSql columns:    {From ClassReadInfo}
            //! ECSql parameters: CachedInstance.$RemoteId
            WSCACHE_EXPORT static Utf8String SelectPropertiesByRemoteId
                (
                const ClassReadInfo& info
                );

            //! Create ECSql to select properties from instances by custom where clause. Use "instance" alias in where clause
            //! ECSql columns:    {From ClassReadInfo}
            //! ECSql parameters: {depends on where clause}
            WSCACHE_EXPORT static Utf8String SelectPropertiesByWhereClause
                (
                const ClassReadInfo& info,
                Utf8StringCR customWhereClause
                );

            //! Create ECSql to select remoteIds from all target instances
            //! ECSql columns:    TargetECClassId, TargetClassInstance.$RemoteId
            //! ECSql parameters: SourceClassInstance.ECInstanceId
            WSCACHE_EXPORT static Utf8String SelectRemoteIdsByRelatedSourceECInstanceId
                (
                ECRelationshipClassCR relationshipClass
                );

            //! Create ECSql to select remote ids from instances by their local ECInstanceIds
            //! ECSql columns:    CachedInstance.$RemoteId
            //! ECSql parameters: none
            WSCACHE_EXPORT static Utf8String SelectRemoteIdsByECInstanceIds
                (
                ECClassCR cachedInstanceClass,
                Utf8StringCR commaSeperatedECInstanceIds
                );

            //! Create ECSql to select all properties and remote id based on instance ECInstanceId
            //! ECSql columns:    CachedInstance.$RemoteId, CachedInstance.*
            //! ECSql parameters: CachedInstance.ECInstanceId
            WSCACHE_EXPORT static Utf8String SelectAllPropertiesAndRemoteIdByECInstanceId(ECClassCR cachedInstanceClass);

            //! Create ECSql to select properties from instance by its remote id. Will not select remote id.
            //! ECSql columns:    CachedInstance.*
            //! ECSql parameters: CachedInstance.$RemoteId
            //! Limit: 1
            WSCACHE_EXPORT static Utf8String SelectAllPropertiesByRemoteId(ECClassCR cachedInstanceClass);

            //! Create ECSql to select cached instance local ECInstanceId by its remote id
            //! ECSql columns:    CachedInstance.ECInstanceId
            //! ECSql parameters: CachedInstance.$RemoteId
            //! Limit: 1
            WSCACHE_EXPORT static Utf8String SelectECInstanceIdByRemoteId(ECClassCR cachedInstanceClass);
            };
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheQueryHelper::ClassReadInfo
    {
    private:
        ECClassCP                                           m_ecClass;
        std::shared_ptr<ISelectProvider::SelectProperties>  m_selectProperties;
        std::shared_ptr<ISelectProvider::SortProperties>    m_sortProperties;

    public:
        WSCACHE_EXPORT ClassReadInfo
            (
            ECClassCR ecClass,
            std::shared_ptr<ISelectProvider::SelectProperties> selectProperties,
            std::shared_ptr<ISelectProvider::SortProperties> sortProperties
            );

        WSCACHE_EXPORT ECClassCR GetECClass() const;
        WSCACHE_EXPORT const ISelectProvider::SelectProperties& GetSelectProperties() const;
        WSCACHE_EXPORT const ISelectProvider::SortProperties& GetSortProperties() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
