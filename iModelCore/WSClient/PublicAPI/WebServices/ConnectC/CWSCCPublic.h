/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/ConnectC/CWSCCPublic.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/ConnectC/CWSCCGenPublic.h>
#include <WebServices/ConnectC/CWSCCGenBufferPublic.h>

/************************************************************************************//**
* \addtogroup ConnectWebServicesClientCAPIFunctions ConnectWebServicesClientC API Function Declarations
* \{
****************************************************************************************/
	
/************************************************************************************//**
* \brief Initialize the API with the authenticated token string
* \param[in] authenticatedToken used to initizalize HttpClients
* \param[in] temporaryDirectory directory path used for temporary files
* \param[in] assetsRootDirectory directory path which contains needed asset files (
* \param[in] applicationName - human readable string with company and application name. Format: "Bentley-TestApplication"
* \param[in] applicationVersion - major and minor numbers could be used to identify application in server side
* \param[in] applicationGUID - unique application GUID used for registering WSG usage
* \param[in] applicationProductId - application product ID (e.g. "1234") used for federated sign-in [optional otherwise]
* \return WSAPIHANDLE API object
****************************************************************************************/
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithToken
(
WCharCP authenticatedToken,
WCharCP temporaryDirectory,
WCharCP assetsRootDirectory,
WCharCP applicationName,
WCharCP applicationVersion,
WCharCP applicationGUID,
WCharCP applicationProductId
);

/************************************************************************************//**
* \brief Initialize the API with user credentials
* \param[in] username username used to login
* \param[in] password password used to login
* \param[in] temporaryDirectory directory path used for temporary files
* \param[in] assetsRootDirectory directory path which contains needed asset files (
* \param[in] applicationName - human readable string with company and application name. Format: "Bentley-TestApplication"
* \param[in] applicationVersion - major and minor numbers could be used to identify application in server side
* \param[in] applicationGUID - unique application GUID used for registering WSG usage
* \param[in] applicationProductId - application product ID (e.g. "1234") used for federated sign-in [optional otherwise]
* \return WSAPIHANDLE API object
****************************************************************************************/
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithCredentials
(
WCharCP username,
WCharCP password,
WCharCP temporaryDirectory,
WCharCP assetsRootDirectory,
WCharCP applicationName,
WCharCP applicationVersion,
WCharCP applicationGUID,
WCharCP applicationProductId
);

/************************************************************************************//**
* \brief Generic REST utility
* \param[in] apiHandle Previously created API object
* \param[out] projectBuffer of User Project data
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_GetIMSUserInfo(CWSCCHANDLE apiHandle, CWSCCDATABUFHANDLE* projectBuffer);

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
* \brief Retrieve the previous status description
* \param[in] apiHandle Previously created API object
* \param[in] proxyUrl The base url for the Web Proxy
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_ConfigureWebProxy(CWSCCHANDLE apiHandle, Utf8CP proxyUrl);

/************************************************************************************//**
* \brief Retrieve the previous status description
* \param[in] apiHandle Previously created API object
* \param[in] proxyUrl The base url for the Web Proxy
* \param[in] username The username used to configure the Web Proxy
* \param[in] password The password used to configure the Web Proxy
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_ConfigureWebProxyWithCredentials
(
CWSCCHANDLE apiHandle,
Utf8CP proxyUrl,
Utf8CP username,
Utf8CP password
);

/** \} */