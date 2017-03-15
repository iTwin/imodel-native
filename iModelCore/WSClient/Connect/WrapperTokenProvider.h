/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/WrapperTokenProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct WrapperTokenProvider : IConnectTokenProvider
    {
    private:
        BeCriticalSection& m_cs;
        IConnectTokenProviderPtr& m_tokenProvider;

    public:
        WrapperTokenProvider(BeCriticalSection& cs, IConnectTokenProviderPtr& tokenProvider) :
            m_cs(cs), m_tokenProvider(tokenProvider)
            {}

        AsyncTaskPtr<SamlTokenPtr> UpdateToken() override
            {
            BeCriticalSectionHolder lock(m_cs);
            return m_tokenProvider->UpdateToken();
            }

        SamlTokenPtr GetToken() override
            {
            BeCriticalSectionHolder lock(m_cs);
            return m_tokenProvider->GetToken();
            }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
