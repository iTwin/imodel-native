/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
 //__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include "LicenseStatus.h"

#include <WebServices/Connect/IConnectAuthenticationProvider.h> // ClientInfo.h does not compile without this
#include <WebServices/Client/ClientInfo.h>

#include <BeHttp/IHttpHandler.h>
#include <Licensing/Utils/FeatureUserDataMap.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct AccessKeyClient> AccessKeyClientPtr;

struct AccessKeyClient
    {
    private:
        std::shared_ptr<struct AccessKeyClientImpl> m_impl;

        AccessKeyClient
        (
            std::shared_ptr<struct AccessKeyClientImpl> implementation
        );

    public:
        //! Creates a client that uses accesskey
        //! @param[in] accessKey Access key generated for the ultimate to track usage against
        //! @param[in] clientInfo ClientInfoPtr from WSClient
        //! @param[in] dbPath Path for LicenseClient database
        //! @param[in] offlineMode ignored for now, no usage in this client yet
        //! @param[in] projectId ProjectID string, defaults to an empty string
        //! @param[in] featureString product feature string, defaults to an empty string
        //! @param[in] customHttpHandler CustomHttpHandler, defaults to a nullptr
        LICENSING_EXPORT static AccessKeyClientPtr Create
        (
            Utf8StringCR accessKey,
			WebServices::ClientInfoPtr clientInfo,
            BeFileNameCR dbPath,
            bool offlineMode,
            Utf8StringCR projectId = "",
            Utf8StringCR featureString = "",
            Http::IHttpHandlerPtr customHttpHandler = nullptr
        );

        //! StartApplication performs actions and creates threads required for usage posting and policy requests, returns LicenseStatus
        // TODO: Return more than BentleyStatus to indicate to the app if the user has rights to use this app or it's crippled etc...
        /*!
        * This method should be called at when the application is started or when it needs to be started again after being stopped.
        * A database is opened for use with the LicenseClient.
        * A policy is requested to check license status; if license status does not allow for proper runtime, it is returned and no further action takes place.
        * Otherwise, threads are created for policy requests and usage posting and then the license status is returned.
        */
        LICENSING_EXPORT LicenseStatus StartApplication();

        //! StopApplication signals background threads to stop and cleans up resources.
        /*!
        * Threads are signalled to stop, any remaining usage logs are saved, and the database is closed.
        */
        /*!
        * Stop Application Comment
        */
        LICENSING_EXPORT BentleyStatus StopApplication();

        //! Marks a feature as used
        //! @param[in] featureId Feature GUID to mark
        //! @param[in] featureUserData Feature tracking metadata
        LICENSING_EXPORT BentleyStatus MarkFeature(Utf8StringCR featureId, FeatureUserDataMapPtr featureUserData);

        //! GetLicenseStatus returns the LicenseStatus of the cached policy
        /*!
        * This method should be used to check the license status of the current cached policy.
        * It does NOT start the policy heartbeat, and will NOT refresh the policy.
        * StartApplication should be called to fetch a policy and start the policy heartbeat.
        */
        LICENSING_EXPORT LicenseStatus GetLicenseStatus();
    };

END_BENTLEY_LICENSING_NAMESPACE
