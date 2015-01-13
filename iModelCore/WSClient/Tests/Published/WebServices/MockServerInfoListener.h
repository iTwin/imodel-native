/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/MockServerInfoListener.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifdef USE_GTEST
#include <gmock/gmock.h>
#endif

#include <WebServices/WSClient.h>

using namespace ::testing;

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockServerInfoListener : public IWSClient::IServerInfoListener
    {
    public:
#ifdef USE_GTEST
        MOCK_METHOD1 (OnServerInfoReceived, void (WSInfoCR info));
#endif
    };

END_BENTLEY_WEBSERVICES_NAMESPACE