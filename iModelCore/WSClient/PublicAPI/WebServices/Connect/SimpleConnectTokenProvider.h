/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/SimpleConnectTokenProvider.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/ISecurityToken.h>
#include <WebServices/Connect/SecurityToken.h>
#include <Bentley/Tasks/AsyncTask.h>

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/IImsClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SimpleConnectTokenProvider> SimpleConnectTokenProviderPtr;
struct SimpleConnectTokenProvider : IConnectTokenProvider
    {
public:
    typedef std::function<AsyncTaskPtr<SecurityTokenPtr>()> UpdateTokenCallback;

private:
    SecurityTokenPtr m_token;
    UpdateTokenCallback m_onUpdate;

private:
    static UpdateTokenCallback GetDefaultTokenCallback ()
        {
        auto lambda([]() {return CreateCompletedAsyncTask(std::make_shared<SecurityToken>()); });
        return std::cref(lambda);
        }

public:
    SimpleConnectTokenProvider(SecurityTokenPtr token, UpdateTokenCallback callback = GetDefaultTokenCallback())
        : m_token(token), m_onUpdate(callback) {};
 
    AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override {return m_onUpdate()
        ->Then<ISecurityTokenPtr>([=](SecurityTokenPtr token)
            {
            m_token = std::static_pointer_cast<SecurityToken>(token);
            return m_token;
            });
    }
    std::shared_ptr<ISecurityToken> GetToken() override { return m_token; };
    };
 
END_BENTLEY_WEBSERVICES_NAMESPACE
