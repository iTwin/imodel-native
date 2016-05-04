/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/ConnectC/ConnectWebServicesClientCTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include "ConnectWebServicesClientCTests.h"

#include <Bentley/Bentley.h>

void ConnectWebServicesClientC::SetUp()
    {
    m_api = ConnectWebServicesClientC_InitializeApiWithCredentials(L"david.jones@bentley.com", L"testdfijEr34", CCPRODUCTID);
    ASSERT_TRUE(m_api != nullptr);
    }

TEST_F(ConnectWebServicesClientC, ReadProject_ProjectExists_SuccessfulRetreival)
    {
    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(m_api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);

    status = ConnectWebServicesClientC_DataBufferFree(m_api, project);
    ASSERT_TRUE(status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, ReadProject_InvalidDataBufHandle_ErrorCodeReturned)
    {
    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CallStatus status = ConnectWebServicesClientC_ReadProject(m_api, instanceId, nullptr);
    ASSERT_TRUE(status == INVALID_PARAMETER);
    }

TEST_F(ConnectWebServicesClientC, Ctor_ValidCredentialsAndProductId_SuccessfulInialization)
    {
    WCharP username = L"david.jones@bentley.com";
    WCharP password = L"testdfijEr34";
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials(username, password, CCPRODUCTID);
    ASSERT_FALSE(api == nullptr);
    }

TEST_F(ConnectWebServicesClientC, Ctor_InvalidCredentialsAndValidProductId_NullptrReturned)
    {
    WChar *username = L"david.jones@bentley.com";
    WChar *password = L"password";
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials(username, password, CCPRODUCTID);
    ASSERT_TRUE(api == nullptr);
    }

TEST_F(ConnectWebServicesClientC, Ctor_ValidCredentialsAndInvalidProductId_NullptrReturned)
    {
    WChar *username = L"david.jones@bentley.com";
    WChar *password = L"testdfijEr34";
    auto api = ConnectWebServicesClientC_InitializeApiWithCredentials(username, password, 9999);
    ASSERT_TRUE(api == nullptr);
    }

TEST_F(ConnectWebServicesClientC, DataBufferGetCount_Only1ProjectIsReturned_SuccessfulRetreival)
    {
    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(m_api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    status = ConnectWebServicesClientC_DataBufferFree(m_api, project);
    ASSERT_TRUE(status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_Only1ProjectIsReturnedWithFulfilledProjectProperties_SuccessfulRetreivalOfProperties)
    {
    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(m_api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    wchar_t stringBuf[4096];
    status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_api, project, PROJECT_BUFF_NAME, 0, 4096, stringBuf);
    ASSERT_TRUE(status == SUCCESS);
    ASSERT_STREQ(stringBuf, L"Davids New QA Project");

    status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_api, project, PROJECT_BUFF_NUMBER, 0, 4096, stringBuf);
    ASSERT_TRUE(status == SUCCESS);
    ASSERT_STREQ(stringBuf, L"-1234567890-0987654321");

    double longitude;
    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(m_api, project, PROJECT_BUFF_LONGITUDE, 0, &longitude);
    ASSERT_TRUE(status == PROPERTY_HAS_NOT_BEEN_SET);

    status = ConnectWebServicesClientC_DataBufferFree(m_api, project);
    ASSERT_TRUE(status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_NULLBuffer_AppropriateStatusCodeReturned)
    {
    wchar_t stringBuf[4096];
    CWSCCDATABUFHANDLE buf = NULL;
    CallStatus status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_api, buf, PROJECT_BUFF_NAME, 0, 4096, stringBuf);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    size_t outSize;
    status = ConnectWebServicesClientC_DataBufferGetStringLength(m_api, buf, PROJECT_BUFF_NAME, 0, &outSize);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    int integer;
    status = ConnectWebServicesClientC_DataBufferGetIntProperty(m_api, buf, PROJECT_BUFF_STATUS, 0, &integer);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    double pDouble;
    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(m_api, buf, PROJECT_BUFF_LONGITUDE, 0, &pDouble);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    int64_t pLong;
    status = ConnectWebServicesClientC_DataBufferGetLongProperty(m_api, buf, PROJECTMRUDETAIL_BUFF_LASTACCESSEDBYUSER, 0, &pLong);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    wchar_t guid[4096];
    status = ConnectWebServicesClientC_DataBufferGetGuidProperty(m_api, buf, CONNECTUSER_BUFF_CONNECTORGGUID, 0, 4096, guid);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(m_api, buf);
    ASSERT_TRUE(status == INVALID_PARAMETER);
    }

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_BufferWithProjectTypeButInvalidPropertyType_AppropriateStatusCodeReturned)
    {
    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(m_api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    int64_t pLong;
    status = ConnectWebServicesClientC_DataBufferGetLongProperty(m_api, project, PROJECTMRUDETAIL_BUFF_LASTACCESSEDBYUSER, 0, &pLong);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    wchar_t guid[4096];
    status = ConnectWebServicesClientC_DataBufferGetGuidProperty(m_api, project, CONNECTUSER_BUFF_CONNECTORGGUID, 0, 4096, guid);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(m_api, project);
    ASSERT_TRUE(status == SUCCESS);
    }

TEST_F(ConnectWebServicesClientC, GetPropertyMethods_BufferWithProjectTypeAndValidPropertyTypeButInvalidProperty_AppropriateStatusCodeReturned)
    {
    WCharP instanceId = L"8faf2677-4540-40d3-964d-252826089c7f";
    CWSCCDATABUFHANDLE project;
    CallStatus status = ConnectWebServicesClientC_ReadProject(m_api, instanceId, &project);
    ASSERT_TRUE(status == SUCCESS);
    uint64_t bufferCount = ConnectWebServicesClientC_DataBufferGetCount(project);
    ASSERT_TRUE(bufferCount == 1);

    int16_t INVALID_PROPERTY = 0;

    wchar_t stringBuf[4096];
    status = ConnectWebServicesClientC_DataBufferGetStringProperty(m_api, project, INVALID_PROPERTY, 0, 4096, stringBuf);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    size_t outSize;
    status = ConnectWebServicesClientC_DataBufferGetStringLength(m_api, project, INVALID_PROPERTY, 0, &outSize);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    int integer;
    status = ConnectWebServicesClientC_DataBufferGetIntProperty(m_api, project, INVALID_PROPERTY, 0, &integer);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    double pDouble;
    status = ConnectWebServicesClientC_DataBufferGetDoubleProperty(m_api, project, INVALID_PROPERTY, 0, &pDouble);
    ASSERT_TRUE(status == INVALID_PARAMETER);

    status = ConnectWebServicesClientC_DataBufferFree(m_api, project);
    ASSERT_TRUE(status == SUCCESS);
    }