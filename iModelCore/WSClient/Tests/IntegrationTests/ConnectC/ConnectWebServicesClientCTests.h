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
#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>
#include <Bentley/Bentley.h>

struct ConnectWebServicesClientCTests : public WSClientBaseTest
    {
    const WString m_username = L"david.jones@bentley.com";
    const WString m_password = L"testdfijEr34";
    const WString m_ccProductId = L"1805";
    const WString m_navProductId = L"2545";
    const WString m_applicationName = L"Bentley-Test";
    const WString m_applicationVersion = L"1.0.0.0";
    const WString m_applicationGuid = L"TestAppGUID";
    const WString m_fiddlerProxyUsername = L"1";                //Fiddler default Username
    const WString m_fiddlerProxyPassword = L"1";                //Fiddler default password
    WString m_fiddlerProxyUrl;

    static void SetUpTestCase();
    void SetUp ();
    void TearDown ();
    };
