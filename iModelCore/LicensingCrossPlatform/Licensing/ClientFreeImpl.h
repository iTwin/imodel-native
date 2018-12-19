/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientFreeImpl.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ClientImpl.h"


BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ClientFreeImpl> ClientFreeImplPtr;

struct ClientFreeImpl : ClientImpl
{
protected:

public:
	LICENSING_EXPORT ClientFreeImpl
		(
		Utf8StringCR accessToken,
		BeVersionCR clientVersion,
		BeFileNameCR db_path,
		bool offlineMode,
		Utf8StringCR projectId,
		Utf8StringCR featureString,
		IHttpHandlerPtr httpHandler
		);
	LICENSING_EXPORT LicenseStatus StartApplication();
	LICENSING_EXPORT BentleyStatus StopApplication();
};

END_BENTLEY_LICENSING_NAMESPACE
