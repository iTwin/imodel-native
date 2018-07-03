/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ECInstanceChangeHandlers.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/ECInstanceChangeHandlers.h>
#include "ECInstanceChangesDirector.h"
#include "NavigationQuery.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetErrorMessage(L10N::NameSpace const& ns, L10N::StringId const& stringId)
    {
    Utf8String msg = IECPresentationManager::GetLocalizationProvider().GetString("", bvector<Utf8CP>{RulesEngineL10N::GetNameSpace(), RulesEngineL10N::ERROR_ECInstanceChangeResult_CantChangeECInstance()});
    msg.append(" - ").append(IECPresentationManager::GetLocalizationProvider().GetString("", bvector<Utf8CP>{ns, stringId}));
    return msg;
    }

typedef bmap<bpair<IECInstanceChangeHandler*, ECClassCP>, bool> ECClassChangeHandlerChecksCache;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool CanHandle(ECClassChangeHandlerChecksCache& cache, 
    IECInstanceChangeHandler& handler, ECDbCR connection, ECClassCR ecClass)
    {
    ECClassChangeHandlerChecksCache::key_type key(&handler, &ecClass);
    auto iter = cache.find(key);
    if (cache.end() != iter)
        return iter->second;

    bool canHandle = handler.CanHandle(connection, ecClass);
    cache[key] = canHandle;
    return canHandle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceChangeResult> ECInstanceChangesDirector::Handle(IConnectionCR connection, bvector<ChangedECInstanceInfo> const& infos, Utf8CP propertyAccessor, ECValueCR value)
    {
    bvector<ECInstanceChangeResult> results;
    if (connection.IsReadOnly())
        {
        results.resize(infos.size(), ECInstanceChangeResult::Error(GetErrorMessage(RulesEngineL10N::GetNameSpace(), RulesEngineL10N::ERROR_ECInstanceChangeResult_ConnectionReadOnly())));
        LoggingHelper::LogMessage(Log::Update, "The connection is read-only - can not change ECInstances.", NativeLogging::LOG_WARNING);
        return results;
        }

    bool didSucceedAny = false;
    ECClassChangeHandlerChecksCache checksCache;
    for (ChangedECInstanceInfo const& info : infos)
        {
        bool didSucceed = false;
        for (IECInstanceChangeHandlerPtr const& handler : m_handlers)
            {
            if (CanHandle(checksCache, *handler, connection.GetECDb(), info.GetPrimaryInstanceClass()))
                {
                ECInstanceChangeResult result = handler->Change(connection.GetECDb(), info, propertyAccessor, value);
                if (SUCCESS == result.GetStatus())
                    {
                    results.push_back(result);
                    didSucceed = true;
                    break;
                    }
                }
            }
        if (!didSucceed)
            {
            results.push_back(ECInstanceChangeResult::Error(GetErrorMessage(RulesEngineL10N::GetNameSpace(), ECPresentationL10N::ERROR_General_Unknown())));
            LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("ECInstance change for ECClass %s could not be handled", 
                info.GetPrimaryInstanceClass().GetFullName()).c_str(), NativeLogging::LOG_ERROR);
            }
        didSucceedAny |= didSucceed;
        }
    if (didSucceedAny)
        connection.GetDb().SaveChanges();
    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefaultECInstanceChangeHandler::_CanHandle(ECDbCR connection, ECClassCR) const
    {
    if (connection.GetECDbSettings().RequiresECCrudWriteToken())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceChangeResult DefaultECInstanceChangeHandler::_Change(ECDbR connection, ChangedECInstanceInfo const& changedInstanceInfo, Utf8CP propertyAccessor, ECValueCR value)
    {
    Utf8String stmtStr("UPDATE ");
    stmtStr.append(changedInstanceInfo.GetChangedInstanceClass().GetECSqlName());
    stmtStr.append(" SET ").append(propertyAccessor).append(" = ?");
    stmtStr.append(" WHERE ECInstanceId = ?");

    ECSqlStatement stmt;
    if (!stmt.Prepare(connection, stmtStr.c_str()).IsSuccess())
        {
        BeAssert(false);
        return ECInstanceChangeResult::Error(GetErrorMessage(RulesEngineL10N::GetNameSpace(), ECPresentationL10N::ERROR_General_Unknown()));
        }

    BoundQueryECValue(value).Bind(stmt, 1);
    stmt.BindId(2, changedInstanceInfo.GetChangedInstanceId());

    if (BE_SQLITE_DONE != stmt.Step())
        {
        BeAssert(false);
        return ECInstanceChangeResult::Error(GetErrorMessage(RulesEngineL10N::GetNameSpace(), ECPresentationL10N::ERROR_General_Unknown()));
        }

    return ECInstanceChangeResult::Success(value);
    }
