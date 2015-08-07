/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/ECSqlStatementCache.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/ObservableECDb.h>
#include <ECDb/ECDbApi.h>
#include <map>
#include <memory>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                             Benediktas.Lipnickas    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSqlStatementCache : IECDbSchemaChangeListener
    {
    public:
        typedef std::function<Utf8String()> CreateECSqlCallback;
        typedef const CreateECSqlCallback& CreateECSqlCallbackCR;

    private:
        ObservableECDb* m_ecDb;
        std::map<Utf8String, std::shared_ptr<ECSqlStatement>> m_cache;

    public:
        WSCACHE_EXPORT ECSqlStatementCache(ObservableECDb& ecDb);
        WSCACHE_EXPORT ~ECSqlStatementCache();

        //! Automatically react to schema change on ECDb and clear statement cache.
        WSCACHE_EXPORT void OnSchemaChanged() override; // IECDbSchemaChangeListener

        //! Get cached and reset or newly prepared statement.
        //! @param key - identifier for statement. Cached statement is returned if such key is found or callback called in other case.
        //! @param createECSqlCallback - callback to create ECSql string if statement is not cached yet.
        //! @return - prepared statement if successful or invalid statement if error occurred. Will not return nullptr.
        WSCACHE_EXPORT std::shared_ptr<ECSqlStatement> GetPreparedStatement(Utf8String key, CreateECSqlCallbackCR createECSqlCallback);

        //! Clear statement cache.
        WSCACHE_EXPORT void Clear();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
