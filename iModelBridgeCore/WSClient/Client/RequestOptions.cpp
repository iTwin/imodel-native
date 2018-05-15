/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/RequestOptions.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/RequestOptions.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define CUSTOM_REQUEST_PROPERTY_CustomOptions "CustomOptions"

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RequestOptions::GetFailureStrategyStr(FailureStrategy failureStrategy)
    {
    switch (failureStrategy)
        {
        case FailureStrategy::Stop:
            return "Stop";
        case FailureStrategy::Continue:
            return "Continue";
        default:
            BeAssert(false);
            break;
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RequestOptions::GetResponseContentStr(ResponseContent responseContent)
    {
    switch (responseContent)
        {
        case ResponseContent::FullInstance:
            return "FullInstance";
        case ResponseContent::Empty:
            return "Empty";
        case ResponseContent::InstanceId:
            return "InstanceId";
        default:
            BeAssert(false);
            break;
        }
    return nullptr;    
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RequestOptions::ToJson(JsonValueR jsonOut) const
    {
    jsonOut["FailureStrategy"] = GetFailureStrategyStr(m_failureStrategy);
    jsonOut["ResponseContent"] = GetResponseContentStr(m_responseContent);
    jsonOut["RefreshInstances"] = m_refreshInstances;

    if (!m_customRequestOptions.empty())
        {
        jsonOut[CUSTOM_REQUEST_PROPERTY_CustomOptions] = m_customRequestOptions;
        }
    }