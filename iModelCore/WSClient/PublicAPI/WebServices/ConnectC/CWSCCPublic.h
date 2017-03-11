/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/ConnectC/CWSCCPublic.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/ConnectC/CWSCC.h>

/************************************************************************************//**
* \addtogroup ConnectWebServicesClientCAPIFunctions ConnectWebServicesClientC API Function Declarations
* \{
****************************************************************************************/
	
/************************************************************************************//**
* \brief Initialize the API with the authenticated token string
* \param[in] authenticatedToken used to initizalize HttpClients
* \param[in] temporaryDirectory directory path used for temporary files
* \param[in] sqlangFilePath full path to the sqlang file for localization strings - needed for localized HttpResponses
* \param[in] applicationName human readable string with company and application name. Format: "Bentley-TestApplication"
* \param[in] applicationVersion major and minor numbers could be used to identify application in server side
* \param[in] applicationGUID unique application GUID used for registering WSG usage
* \param[in] applicationProductId application product ID (e.g. "1234") used for federated sign-in
* \param[in] proxyUrl [optional] The base url for the Web Proxy
* \param[in] proxyUsername [optional] The username used to configure the Web Proxy
* \param[in] proxyPassword [optional] The password used to configure the Web Proxy
* \param[in] customHandler [forbidden] Custom HttpHandler used for testing purposes ONLY
* \return WSAPIHANDLE API object
****************************************************************************************/
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithToken
(
WCharCP authenticatedToken,
WCharCP temporaryDirectory,
WCharCP sqlangFilePath,
WCharCP applicationName,
WCharCP applicationVersion,
WCharCP applicationGUID,
WCharCP applicationProductId,
WCharCP proxyUrl,
WCharCP proxyUsername,
WCharCP proxyPassword,
IHTTPHANDLERPTR customHandler
);

/************************************************************************************//**
* \brief Initialize the API with user credentials
* \param[in] username username used to login
* \param[in] password password used to login
* \param[in] temporaryDirectory directory path used for temporary files
* \param[in] sqlangFilePath full path to the sqlang file for localization strings - needed for localized HttpResponses
* \param[in] applicationName human readable string with company and application name. Format: "Bentley-TestApplication"
* \param[in] applicationVersion major and minor numbers could be used to identify application in server side
* \param[in] applicationGUID unique application GUID used for registering WSG usage
* \param[in] applicationProductId application product ID (e.g. "1234") used for federated sign-in
* \param[in] proxyUrl [optional] The base url for the Web Proxy
* \param[in] proxyUsername [optional] The username used to configure the Web Proxy
* \param[in] proxyPassword [optional] The password used to configure the Web Proxy
* \param[in] customHandler [forbidden] Custom HttpHandler used for testing purposes ONLY
* \param[in] securityStoreInitializer  The value used to initialize Security Store (see SecurityStore::Initialize)
* \return WSAPIHANDLE API object
****************************************************************************************/
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithCredentials
(
WCharCP username,
WCharCP password,
WCharCP temporaryDirectory,
WCharCP sqlangFilePath,
WCharCP applicationName,
WCharCP applicationVersion,
WCharCP applicationGUID,
WCharCP applicationProductId,
WCharCP proxyUrl,
WCharCP proxyUsername,
WCharCP proxyPassword,
IHTTPHANDLERPTR customHandler,
void* securityStoreInitializer
);

/************************************************************************************//**
* \brief API handle free function
* \param[in] apiHandle API object
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_FreeApi(CWSCCHANDLE apiHandle);

/************************************************************************************//**
* \brief Create a new projectfavorite_v2
* \param[in] apiHandle API object
* \param[in] ProjectGuid
* \return Success or error code. See \ref ConnectWebServicesClientCStatusCodes
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_CreateProjectFavorite_V4
(
CWSCCHANDLE apiHandle,
WCharCP ProjectGuid
);

/************************************************************************************//**
* \brief Retrieve the previous status message
* \param[in] apiHandle Previously created API object
* \return last status message
****************************************************************************************/
CWSCC_EXPORT CharCP ConnectWebServicesClientC_GetLastStatusMessage(CWSCCHANDLE apiHandle);

/************************************************************************************//**
* \brief Retrieve the previous status description
* \param[in] apiHandle Previously created API object
* \return last status description
****************************************************************************************/
CWSCC_EXPORT CharCP ConnectWebServicesClientC_GetLastStatusDescription(CWSCCHANDLE apiHandle);

/************************************************************************************//**
* \brief Retrieve the instanceId of the last created object (if there is one, empty string otherwise)
* \param[in] apiHandle Previously created API object
* \return last instance Id
****************************************************************************************/
CWSCC_EXPORT CharCP ConnectWebServicesClientC_GetLastCreatedObjectInstanceId(CWSCCHANDLE apiHandle);

/************************************************************************************//**
* \brief Retrieve the previous status description
* \param[in] apiHandle Previously created API object
* \param[in] proxyUrl The base url for the Web Proxy
* \param[in] username  [optional] The username used to configure the Web Proxy
* \param[in] password  [optional] The password used to configure the Web Proxy
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_ConfigureWebProxy
(
CWSCCHANDLE apiHandle,
Utf8CP proxyUrl,
Utf8CP username,
Utf8CP password
);

/** \} */