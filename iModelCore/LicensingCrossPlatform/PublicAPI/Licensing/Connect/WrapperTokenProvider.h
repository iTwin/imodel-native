/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/WrapperTokenProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "IConnectTokenProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct WrapperTokenProvider : IConnectTokenProvider
    {
    private:
        BeMutex& m_cs;
        IConnectTokenProviderPtr& m_tokenProvider;

    public:
        WrapperTokenProvider(BeMutex& cs, IConnectTokenProviderPtr& tokenProvider) :
            m_cs(cs), m_tokenProvider(tokenProvider)
            {}

        AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override
            {
            BeMutexHolder lock(m_cs);
            return m_tokenProvider->UpdateToken();
            }

        ISecurityTokenPtr GetToken() override
            {
            BeMutexHolder lock(m_cs);
            return m_tokenProvider->GetToken();
            }
    };

END_BENTLEY_LICENSING_NAMESPACE
