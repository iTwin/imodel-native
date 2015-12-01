/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Client/MockServerInfoListener.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Client/WSClient.h>

using namespace ::testing;

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockServerInfoListener : public IWSClient::IServerInfoListener
    {
    public:
        MOCK_METHOD1(OnServerInfoReceived, void(WSInfoCR info));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
