/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/MockRepositoryInfoListener.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Client/WSRepositoryClient.h>

using namespace ::testing;

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockRepositoryInfoListener : public IWSRepositoryClient::IRepositoryInfoListener
    {
    public:
        MOCK_METHOD1(OnInfoReceived, void(WSRepositoryCR info));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
