/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ConnectC/ConnectWebServicesClientCTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>

#include <WebServices/ConnectC/CWSCCPublic.h>

struct ConnectWebServicesClientC : public WSClientBaseTest
    {
    static const int CCPRODUCTID = 1805;
    static const int NAVPRODUCTID = 2545;

    CWSCCHANDLE m_api;
    void SetUp();
    };
