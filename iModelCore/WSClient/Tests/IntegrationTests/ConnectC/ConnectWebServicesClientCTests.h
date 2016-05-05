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
    const WString m_username = L"david.jones@bentley.com";
    const WString m_password = L"testdfijEr34";
    const WString m_temporaryDirectory = L"C:/Users/David.Jones/AppData/Local/Bentley/WSApi";
    const WString m_assetsRootDirectory = L"D:/dev/dgndb0601dev/out/Winx64/Product/DgnClientSdk-Winx64/assets";
    const WString m_ccProductId = L"1805";
    const WString m_navProductId = L"2545";
    const WString m_applicationName = L"Bentley-Test";
    const WString m_applicationVersion = L"1.0.0.0";
    const WString m_applicationGuid = L"TestAppGUID";

    CWSCCHANDLE m_api;
    void SetUp();
    };
