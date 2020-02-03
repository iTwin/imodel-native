/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/Bentley.h>
#include <Bentley/BeTest.h>
#include <Windows.h>
#include <curl/curl.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <RealityPlatformTools/WSGServices.h>
#include <RealityPlatformTools/GeoCoordinationService.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

struct RealityModFrameworkTestsUtils
    {
private:
    static WString s_exePath;
public:
    EXPORT_ATTRIBUTE static Utf8String GetTestDataContent(WString filename);
    EXPORT_ATTRIBUTE static WString GetTestDataPath(WString filename);
    EXPORT_ATTRIBUTE static WString GetDirectory();
    };

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct ErrorClass
    {
    static ErrorClass* s_error;
    MOCK_CONST_METHOD2(errorCallBack, void(Utf8String basicMessage, const RawServerResponse& rawResponse));

    static void callBack(Utf8String basicMessage, const RealityPlatform::RawServerResponse& rawResponse)
    {
        s_error->errorCallBack(basicMessage, rawResponse);
    }

    ErrorClass()
    {
        if(s_error == nullptr)
        {
            s_error = this;
        }
    }
    };

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              05/2017
//=====================================================================================
struct MockWSGRequest : WSGRequest
    {
    MockWSGRequest() : WSGRequest()
    {}

	~MockWSGRequest()
        {
		s_instance = nullptr;
        }

    MOCK_CONST_METHOD1(SetCertificatePath, void(Utf8String certificate));
    MOCK_CONST_METHOD5(PerformAzureRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
    MOCK_CONST_METHOD4(PrepareRequest, CURL*(const WSGURL& wsgRequest, RawServerResponse& responseString, bool verifyPeer, BeFile* file));
    MOCK_CONST_METHOD5(_PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
    MOCK_CONST_METHOD5(PerformRequest, void(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry));
    };

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              06/2017
//! MockWSGRequestFixture
//=====================================================================================
class MockWSGRequestFixture : public testing::Test
    {
public:
    static MockWSGRequest* s_mockWSGInstance;

    static void SetUpTestCase()
        {
        s_mockWSGInstance = new MockWSGRequest();
        }
    static void TearDownTestCase()
        {
        delete s_mockWSGInstance;
        s_mockWSGInstance = nullptr;
        }
    };

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              06/2017
//! MockRealityDataServiceFixture
//=====================================================================================
class MockRealityDataServiceFixture : public MockWSGRequestFixture
    {
public:
    static RealityDataService* s_realityDataService;
    static ErrorClass* s_errorClass;

    // We need to setup a new error class and destroy it for each test that run under the fixture
    //    that way it will ensure that unfill expectation will make the test fail
    virtual void SetUp()
        {
        s_errorClass = new ErrorClass();
        s_realityDataService->SetErrorCallback(mockErrorCallBack);
        }

    virtual void TearDown()
        {
        delete s_errorClass;
        s_errorClass = nullptr;
        }
    static void SetUpTestCase()
        {
        s_mockWSGInstance = new MockWSGRequest();
        s_realityDataService = new RealityDataService();
        s_realityDataService->SetServerComponents("myserver.com", "9.9", "myRepo", "mySchema", "zz:\\mycertificate.pfx", "myProjectID");
        s_realityDataService->SetUserAgent("RealityModFrameworkTest - dummy user agent");


        }

    static void TearDownTestCase()
        {

        delete s_realityDataService;
        s_realityDataService = nullptr;        
        delete s_mockWSGInstance;
        s_mockWSGInstance = nullptr;
        }

    static void mockErrorCallBack(Utf8String basicMessage, const RealityPlatform::RawServerResponse& rawResponse)
        {
        if (s_errorClass != nullptr)
            {
            s_errorClass->errorCallBack(basicMessage, rawResponse);
            }
        }
    };

//=====================================================================================
//! @bsiclass                                   Remi.Charbonneau              06/2017
//! MockRealityDataServiceFixture
//=====================================================================================
 class MockGeoCoordinationServiceFixture : public MockWSGRequestFixture
{
public:
    static GeoCoordinationService* s_geoCoordinateService;
    static ErrorClass* s_errorClass;

    static void SetUpTestCase()
        {
        s_mockWSGInstance = new MockWSGRequest();
        s_geoCoordinateService = new GeoCoordinationService();
        s_geoCoordinateService->SetServerComponents("example.com", "99", "Dummy-Server", "VirtualModeling");
        s_geoCoordinateService->SetUserAgent("MockGeoCoordinationServiceFixture - dummy user agent");
        s_errorClass = new ErrorClass();
        s_geoCoordinateService->SetErrorCallback(mockErrorCallBack);
        }

    static Utf8String GetURLString()
        {
        return Utf8String("https://example.com/v99/Repositories/Dummy-Server/VirtualModeling");
        }


    static void TearDownTestCase()
        {
        delete s_errorClass;
        s_errorClass = nullptr;
        delete s_geoCoordinateService;
        s_geoCoordinateService = nullptr;        
        delete s_mockWSGInstance;
        s_mockWSGInstance = nullptr;
        }

    static void mockErrorCallBack(Utf8String basicMessage, const RawServerResponse& rawResponse)
        {
        if (s_errorClass != nullptr)
            {
            s_errorClass->errorCallBack(basicMessage, rawResponse);
            }
        }
    };