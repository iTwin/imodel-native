/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectSetup.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! LEGACY CODE - CONSIDER REVIEWING !!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <WebServices/Client/WebServicesClient.h>
#include <DgnClientFx/DgnClientApp.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

WSCLIENT_EXPORT extern void ConnectSetup(JsonValueCR messageDataObj, bool requireToken);

END_BENTLEY_WEBSERVICES_NAMESPACE
