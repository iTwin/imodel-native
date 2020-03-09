/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
 //__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/ApplicationInfo.h>

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
    //! DEPRECIATED - use the other Create that takes ApplicationInfo instead of ClientInfo
    //! @param[in] accessKey Access key generated for the ultimate to track usage against
    //! @param[in] clientInfo ClientInfoPtr from WSClient
    //! @param[in] dbPath Path for LicenseClient database
    //! @param[in] offlineMode If offline, pushes usage in discrete intervals. If not offline, pushes usage continuously via stream
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

    //! Creates a client that uses accesskey
    //! @param[in] accessKey Access key generated for the ultimate to track usage against
    //! @param[in] applicationInfo ApplicationInfoPtr
    //! @param[in] dbPath Path for LicenseClient database
    //! @param[in] offlineMode If offline, pushes usage in discrete intervals. If not offline, pushes usage continuously via stream
    //! @param[in] projectId ProjectID string, defaults to an empty string
    //! @param[in] featureString product feature string, defaults to an empty string
    //! @param[in] customHttpHandler CustomHttpHandler, defaults to a nullptr
    LICENSING_EXPORT static AccessKeyClientPtr Create
        (
        Utf8StringCR accessKey,
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR dbPath,
        bool offlineMode,
        Utf8StringCR projectId = "",
        Utf8StringCR featureString = "",
        Http::IHttpHandlerPtr customHttpHandler = nullptr
        );

    //! WARNING: Do not use without consulting Daniel Bishop. This function is restricted.
    //! @param[in] accessKey Access key generated for the ultimate to track usage against
    //! @param[in] applicationInfo ApplicationInfoPtr
    //! @param[in] dbPath Path for LicenseClient database
    //! @param[in] offlineMode If offline, pushes usage in discrete intervals. If not offline, pushes usage continuously via stream
    //! @param[in] ultimateId ultimate used instead of device ID to validate accessKey
    //! @param[in] projectId ProjectID string, defaults to an empty string
    //! @param[in] featureString product feature string, defaults to an empty string
    //! @param[in] customHttpHandler CustomHttpHandler, defaults to a nullptr
    LICENSING_EXPORT static AccessKeyClientPtr AgnosticCreateWithUltimate
        (
        Utf8StringCR accessKey,
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR dbPath,
        bool offlineMode,
        Utf8StringCR ultimateId,
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

    //! GetTrialDaysRemaining returns the days remaining in a trial or evaluation policy
    /*!
    * This method is used to get the number of days remaining in a trial or evaluation policy.
    * Returns 0 for expired, a postitive integer for the remaining number of days, and -1 for no data, or if policy is not evaluation or trial.
    */
    LICENSING_EXPORT int64_t GetTrialDaysRemaining();

	//! ImportCheckout allows importing of a .belic file policy into the DB
	//! -2 for machineid mismatch, -1 for error, 0 for success  
	//! -1 covers general errors like file in wrong format, file does not exist, 
	LICENSING_EXPORT int64_t ImportCheckout(BeFileNameCR filepath);

    //! Removes local checkout from DB if present
    //! Does not check in server side checkout
    //! ERROR if param not numeric value over 1000, or DB Issue
    //! SUCCESS if valid param, will return success even if productId not in DB to remove. 
    //! If productId checkout(s) are in DB they will be removed
    LICENSING_EXPORT BentleyStatus DeleteLocalCheckout(Utf8StringCR productId);
    };

END_BENTLEY_LICENSING_NAMESPACE
