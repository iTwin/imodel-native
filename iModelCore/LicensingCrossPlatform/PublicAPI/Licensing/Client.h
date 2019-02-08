/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Licensing/Client.h $
 |
 |  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include "LicenseStatus.h"

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

 #include <Licensing/Utils/FeatureUserDataMap.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct Client> ClientPtr;

struct Client
{
private:
    std::shared_ptr<struct IClient> m_impl;

    Client
        (
        std::shared_ptr<struct IClient> implementation
        );

public:
	//! Client Creator
	/*!
	* Initializes an instance of Client, returns a ClientPtr to the prepared Client instance.
	* 
	* 
	*/
	LICENSING_EXPORT static ClientPtr Create
        (
		const ConnectSignInManager::UserInfo& userInfo, /** For determining UserID */
        ClientInfoPtr clientInfo, /** A ClientInfoPtr */
        std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider, /** For  */
        BeFileNameCR dbPath, /** Path for LicenseClient database */
        bool offlineMode, /** If offline, pushes usage in discrete intervals. If not offline, pushes usage continuously via stream */
        Utf8StringCR projectId = "", /** ProjectID string, defaults to an empty string */
        Utf8StringCR featureString = "", /** FeatureString, defaults to an empty string */
        IHttpHandlerPtr customHttpHandler = nullptr /** CustomHttpHandler, defaults to a nullptr */
        );

	LICENSING_EXPORT static ClientPtr CreateWithKey
	(
		Utf8StringCR accessKey, /** AccessKey */
		ClientInfoPtr clientInfo, /** A ClientInfoPtr */
		BeFileNameCR dbPath, /** Path for LicenseClient database */
		bool offlineMode, /** If offline, pushes usage in discrete intervals. If not offline, pushes usage continuously via stream */
		Utf8StringCR projectId = "", /** ProjectID string, defaults to an empty string */
		Utf8StringCR featureString = "", /** FeatureString, defaults to an empty string */
		IHttpHandlerPtr customHttpHandler = nullptr /** CustomHttpHandler, defaults to a nullptr */
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
	/*!
	* Mark Feature Comment
	*/
    LICENSING_EXPORT BentleyStatus MarkFeature(Utf8StringCR featureId, FeatureUserDataMap* featureUserData);
};

END_BENTLEY_LICENSING_NAMESPACE
