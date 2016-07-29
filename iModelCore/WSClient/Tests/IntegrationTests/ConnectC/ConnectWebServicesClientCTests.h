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
#include <WebServices/ConnectC/CWSCCBufferPublic.h>
#include <WebServices/ConnectC/ConnectWsgGlobal/GlobalSchemaBufferGenPublic.h>
#include <WebServices/ConnectC/ConnectWsgGlobal/GlobalSchemaGenPublic.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <Bentley/Bentley.h>

struct ConnectWebServicesClientCTests : public WSClientBaseTest
    {
    const WString m_regUsername = L"cwsccDEV_reg1@mailinator.com";
    const WString m_regPassword = L"cwsccreg1";
    const WString m_pmUsername = L"cwsccDEV_pm1@mailinator.com";
    const WString m_pmPassword = L"cwsccpm1";
    const WString m_coadmUsername = L"cwsccDEV_coadm1";
    const WString m_coadmPassword = L"cwscccoadm1";
    const WString m_pmadmUsername = L"cwsccDEV_pmadm1@mailinator.com";
    const WString m_pmadmPassword = L"cwsccpmadm1";
    const WString m_ccProductId = L"1805";
    const WString m_navProductId = L"2545";
    const WString m_applicationName = L"Bentley-Test";
    const WString m_applicationVersion = L"1.0.0.0";
    const WString m_applicationGuid = L"TestAppGUID";
    const WString m_fiddlerProxyUsername = L"1";                //Fiddler default Username
    const WString m_fiddlerProxyPassword = L"1";                //Fiddler default password
    WString m_fiddlerProxyUrl;

    static void SetUpTestCase();
    void SetUp();
    void TearDown();
    };
