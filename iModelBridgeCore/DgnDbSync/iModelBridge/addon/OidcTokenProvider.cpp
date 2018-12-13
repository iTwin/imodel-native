/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/addon/OidcTokenProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "OidcTokenProvider.h"
USING_NAMESPACE_BENTLEY_DGN

#include <Bentley/Tasks/AsyncTask.h>
USING_NAMESPACE_BENTLEY_TASKS

#include <node-addon-api/napi.h>
using namespace Napi;
Napi::Function RequestTokenFunction();

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    John.Majerle                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OidcTokenProvider::OidcTokenProvider(Utf8String encodedToken)
    {
    m_token = std::make_shared<OidcToken>(encodedToken);        
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    John.Majerle                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<WebServices::ISecurityTokenPtr> OidcTokenProvider::UpdateToken()
    {
    auto nodeFunction = RequestTokenFunction();

    auto task = std::make_shared<PackagedAsyncTask<WebServices::ISecurityTokenPtr>> ([&] 
        {
        return std::make_shared<OidcToken>(Utf8String(nodeFunction({ }).ToString().Utf8Value().c_str()));
        }
        );

    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    John.Majerle                  12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::ISecurityTokenPtr OidcTokenProvider::GetToken()
    {
    //Is token valid ?
    return m_token;
    }
