/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Client/WSRepositoryClient.h>

using namespace ::testing;

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct MockRepositoryInfoListener : IWSRepositoryClient::IRepositoryInfoListener
    {
    MOCK_METHOD1(OnInfoReceived, void(WSRepositoryCR info));
    };
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
