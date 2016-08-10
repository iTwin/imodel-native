/*--------------------------------------------------------------------------------------+
|
|     $Source: Azure/AzureServiceBusSASDTO.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Azure/AzureServiceBusSASDTO.h>
#include <iomanip>

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
AzureServiceBusSASDTO::AzureServiceBusSASDTO
(
	Utf8String sasToken,
	Utf8String baseAddress
)
{
	m_sasToken = sasToken;
	m_baseAddress = baseAddress;
}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
AzureServiceBusSASDTOPtr AzureServiceBusSASDTO::Create
(
	Utf8String sasToken,
	Utf8String baseAddress
)
{
	return AzureServiceBusSASDTOPtr(new AzureServiceBusSASDTO(sasToken, baseAddress));
}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String AzureServiceBusSASDTO::GetSASToken()
{
	return m_sasToken;
}

//---------------------------------------------------------------------------------------
//@bsimethod                                   Arvind.Venkateswaran             06/2016
//---------------------------------------------------------------------------------------
Utf8String AzureServiceBusSASDTO::GetBaseAddress()
{
	return m_baseAddress;
}