/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/ConnectC/IMSSearch/IMSSearchPublic.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/ConnectC/CWSCC.h>

/************************************************************************************//**
* \defgroup ConnectWebServicesClientCAPIFunctions ConnectWebServicesClientC API Function Declarations
* \{
****************************************************************************************/

/************************************************************************************//**
* \brief Generic REST utility
* \param[in] apiHandle Previously created API object
* \param[out] projectBuffer of User Project data
****************************************************************************************/
CWSCC_EXPORT CallStatus ConnectWebServicesClientC_GetIMSUserInfo(CWSCCHANDLE apiHandle, CWSCCDATABUFHANDLE* projectBuffer);

/** \} */
