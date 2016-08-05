/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Azure/AzureServiceBusSASDTO.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../Client/WebServicesClient.h"
#include <BeHttp/HttpError.h>
#include <BeHttp/HttpResponse.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef std::shared_ptr<struct AzureServiceBusSASDTO> AzureServiceBusSASDTOPtr;

struct AzureServiceBusSASDTO
{
	//__PUBLISH_SECTION_END__
private:
	Utf8String m_sasToken;
	Utf8String m_baseAddress;

	AzureServiceBusSASDTO(Utf8String sasToken, Utf8String baseAddress);
	//__PUBLISH_SECTION_START__
public:
	WSCLIENT_EXPORT static AzureServiceBusSASDTOPtr Create(Utf8String sasToken, Utf8String baseAddress);
	WSCLIENT_EXPORT Utf8String GetSASToken();
	WSCLIENT_EXPORT Utf8String GetBaseAddress();
};

END_BENTLEY_WEBSERVICES_NAMESPACE