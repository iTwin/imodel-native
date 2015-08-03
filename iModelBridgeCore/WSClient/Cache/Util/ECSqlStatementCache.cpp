/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ECSqlStatementCache.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECSqlStatementCache.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatementCache::ECSqlStatementCache(ObservableECDb& ecDb) :
m_ecDb(&ecDb)
    {
    m_ecDb->RegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlStatementCache::~ECSqlStatementCache()
    {
    m_ecDb->UnRegisterSchemaChangeListener(this);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlStatementCache::OnSchemaChanged()
    {
    Clear();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ECSqlStatement> ECSqlStatementCache::GetPreparedStatement(Utf8String key, CreateECSqlCallbackCR createECSqlCallback)
    {
    std::shared_ptr<ECSqlStatement> statement;

    auto it = m_cache.find(key);
    if (it == m_cache.end())
        {
        statement = std::make_shared<ECSqlStatement>();
        ECSqlStatus status = statement->Prepare(*m_ecDb, createECSqlCallback().c_str());

        if (ECSqlStatus::Success != status)
            {
            BeAssert(false && "Failed to prepare statement");
            return statement;
            }

        m_cache[key] = statement;
        }
    else
        {
        statement = it->second;
        if (ECSqlStatus::Success != statement->Reset())
            {
            BeAssert(false && "Failed to reset statement");
            }
        if (ECSqlStatus::Success != statement->ClearBindings())
            {
            BeAssert(false && "Failed to clear statement bindings");
            }
        }

    return statement;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlStatementCache::Clear()
    {
    m_cache.clear();
    }

