/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ECSqlStatementCache.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECSqlStatementCache.h>
#include "../Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::ECSqlStatementCache::ECSqlStatementCache(ObservableECDb& ecDb) :
m_ecDb(&ecDb)
    {
    m_ecDb->RegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::ECSqlStatementCache::~ECSqlStatementCache()
    {
    m_ecDb->UnRegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WebServices::ECSqlStatementCache::OnSchemaChanged()
    {
    Clear();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatementPtr WebServices::ECSqlStatementCache::GetPreparedStatement(Utf8String key, CreateECSqlCallbackCR createECSqlCallback)
    {
    ECSqlStatementPtr statement;

    auto it = m_cache.find(key);
    if (it == m_cache.end())
        {
        statement = std::make_shared<ECSqlStatement>();
        Utf8String ecsql = createECSqlCallback();
        ECSqlStatus status = statement->Prepare(*m_ecDb, ecsql.c_str());

        if (ECSqlStatus::Success != status)
            {
            LOG.errorv("Failed to prepare statement: %s", ecsql.c_str());
            BeAssert(false);
            return statement;
            }

        m_cache[key] = statement;
        }
    else
        {
        statement = it->second;
        auto status = statement->Reset();
        if (ECSqlStatus::Success != status)
            {
            BeAssert(false && "Failed to reset statement");
            }
        status = statement->ClearBindings();
        if (ECSqlStatus::Success != status)
            {
            BeAssert(false && "Failed to clear statement bindings");
            }
        }

    return statement;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WebServices::ECSqlStatementCache::Clear()
    {
    m_cache.clear();
    }
