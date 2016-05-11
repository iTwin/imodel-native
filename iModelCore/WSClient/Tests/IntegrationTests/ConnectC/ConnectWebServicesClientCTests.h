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
#include <Boost_1_5_7_0/Boost/uuid/uuid.hpp>             // uuid class
#include <Boost_1_5_7_0/Boost/uuid/random_generator.hpp> //uuid Gen
#include <Boost_1_5_7_0/Boost/uuid/uuid_io.hpp>          // streaming operators etc.

struct ConnectWebServicesClientC : public WSClientBaseTest
    {
    const WString m_username = L"david.jones@bentley.com";
    const WString m_password = L"testdfijEr34";
    const WString m_temporaryDirectory = L"C:/Users/David.Jones/AppData/Local/Bentley/WSApi";
    const WString m_assetsRootDirectory = L"D:/dev/dgndb0601dev/out/Winx64/Product/DgnClientSdk-Winx64/assets";
    const WString m_ccProductId = L"1805";
    const WString m_navProductId = L"2545";
    const WString m_applicationName = L"Bentley-Test";
    const WString m_applicationVersion = L"1.0.0.0";
    const WString m_applicationGuid = L"TestAppGUID";
    const WString m_fiddlerProxyUsername = L"1";                //Fiddler default Username
    const WString m_fiddlerProxyPassword = L"1";                //Fiddler default password
    WString m_fiddlerProxyUrl;

    void SetUp ();
    void TearDown ();
    };
