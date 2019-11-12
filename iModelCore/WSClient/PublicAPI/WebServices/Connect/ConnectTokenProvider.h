/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! DEPRECATED CODE - Use ConnectSignInManager
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/IImsClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

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
        WSCLIENT_EXPORT ConnectTokenProvider(IImsClientPtr client, IConnectAuthenticationPersistencePtr customPersistence = nullptr);

        //! Set new token lifetime in minutes
        WSCLIENT_EXPORT void Configure(uint64_t tokenLifetime);

        WSCLIENT_EXPORT AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override;
        WSCLIENT_EXPORT ISecurityTokenPtr GetToken() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
