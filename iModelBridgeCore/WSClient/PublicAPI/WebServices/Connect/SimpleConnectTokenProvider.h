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
#include <WebServices/Connect/SecurityToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SimpleConnectTokenProvider> SimpleConnectTokenProviderPtr;
struct SimpleConnectTokenProvider : IConnectTokenProvider
    {
public:
    typedef std::function<AsyncTaskPtr<ISecurityTokenPtr>()> UpdateTokenCallback;

private:
    ISecurityTokenPtr m_token;
    UpdateTokenCallback m_onUpdate;

public:
    //! Create token provider 
    //! @param token initial token to use. Simple wrapper SecurityToken could be used.
    //! @param onUpdate optional callback that is called when existing token is expired. Should return new token or null token if it cannot be updated (default).
    SimpleConnectTokenProvider(ISecurityTokenPtr token, UpdateTokenCallback onUpdate = [] { return CreateCompletedAsyncTask(ISecurityTokenPtr()); })
        : m_token(token), m_onUpdate(onUpdate) {};

    //! Call onUpdate callback and cache received token
    AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override
        {
        return m_onUpdate()->Then<ISecurityTokenPtr>([=] (ISecurityTokenPtr token)
            {
            m_token = token;
            return m_token;
            });
        }

    //! Return original or cached token
    ISecurityTokenPtr GetToken() override { return m_token; };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
