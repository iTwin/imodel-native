/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/Authentication.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <MobileDgn/MobileDgnApplication.h>

#define CONNECT_COMMAND_SHOW_USER_DATA  "CONNECT.Command.ShowUserData"
#define CONNECT_REQUEST_SETUP           "CONNECT.Message.Setup"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

WS_EXPORT void Authenticate(JsonValueCR messageDataObj);

END_BENTLEY_WEBSERVICES_NAMESPACE
