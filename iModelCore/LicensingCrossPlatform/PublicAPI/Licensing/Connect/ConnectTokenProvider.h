/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/ConnectTokenProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! DEPRECATED CODE - Use ConnectSignInManager
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <Licensing/Licensing.h>
#include "IConnectAuthenticationPersistence.h"
#include "IConnectTokenProvider.h"
#include "IImsClient.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ConnectTokenProvider : public IConnectTokenProvider
    {
    private:
        IImsClientPtr m_client;
        IConnectAuthenticationPersistencePtr m_persistence;
        uint64_t m_tokenLifetime;

    public:
        LICENSING_EXPORT ConnectTokenProvider(IImsClientPtr client, IConnectAuthenticationPersistencePtr customPersistence = nullptr);

        //! Set new token lifetime in minutes
		LICENSING_EXPORT void Configure(uint64_t tokenLifetime);

		LICENSING_EXPORT AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override;
		LICENSING_EXPORT ISecurityTokenPtr GetToken() override;
    };

END_BENTLEY_LICENSING_NAMESPACE
