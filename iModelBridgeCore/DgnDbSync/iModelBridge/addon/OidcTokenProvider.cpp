/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "OidcTokenProvider.h"
USING_NAMESPACE_BENTLEY_DGN

#include <Bentley/Tasks/AsyncTask.h>
USING_NAMESPACE_BENTLEY_TASKS

#include <Napi/napi.h>
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
        m_token = std::make_shared<OidcToken>(Utf8String(nodeFunction({ }).ToString().Utf8Value().c_str()));
        return m_token;
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
