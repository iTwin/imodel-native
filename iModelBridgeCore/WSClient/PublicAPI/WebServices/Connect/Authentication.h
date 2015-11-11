/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/Authentication.h $
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

#define CONNECT_COMMAND_SHOW_USER_DATA  "CONNECT.Command.ShowUserData"
#define CONNECT_REQUEST_SETUP           "CONNECT.Message.Setup"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

WSCLIENT_EXPORT extern void Authenticate(JsonValueCR messageDataObj);

END_BENTLEY_WEBSERVICES_NAMESPACE
