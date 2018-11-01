/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/MockConnectSignInManagerListener.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Connect/ConnectSignInManager.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#if defined (USE_GTEST)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockConnectSignInManagerListener : public ConnectSignInManager::IListener
    {
    MOCK_METHOD0(_OnUserTokenExpired, void());
    MOCK_METHOD0(_OnUserChanged, void());
    MOCK_METHOD0(_OnUserSignedIn, void());
    MOCK_METHOD0(_OnUserSignedOut, void());
    MOCK_METHOD0(_OnUserSignedInViaConnectionClient, void());
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
