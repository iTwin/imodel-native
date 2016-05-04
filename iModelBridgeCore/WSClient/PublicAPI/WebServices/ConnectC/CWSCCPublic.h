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
* \param[in] productId used for usage tracking
* \return WSAPIHANDLE API object
****************************************************************************************/
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithToken(WCharCP authenticatedToken, uint32_t productId);

/************************************************************************************//**
* \brief Initialize the API with user credentials
* \param[in] username User's username
* \param[in] password User's password
* \param[in] productId used for usage tracking
* \return WSAPIHANDLE API object
****************************************************************************************/
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithCredentials(WCharCP username, WCharCP password, uint32_t productId);

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

/** \} */