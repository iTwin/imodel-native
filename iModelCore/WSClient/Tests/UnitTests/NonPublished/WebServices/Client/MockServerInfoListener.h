/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Client/WSClient.h>

using namespace ::testing;

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockServerInfoListener : IWSClient::IServerInfoListener
    {
    MOCK_METHOD1(OnServerInfoReceived, void(WSInfoCR info));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
