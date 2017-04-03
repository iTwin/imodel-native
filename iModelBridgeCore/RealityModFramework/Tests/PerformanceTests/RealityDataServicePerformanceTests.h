/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/PerformanceTests/RealityDataServicePerformanceTests.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__


#include <ctime>

#include <Bentley/DateTime.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE



struct RealityDataServicePerformanceTests
    {

public:

    enum class DisplayOption
    {
        Info,
        Question,
        Tip,
        Error
    };
    RealityDataServicePerformanceTests();
    ~RealityDataServicePerformanceTests();

    void Run(Utf8String serverName);
    void Usage();
    StatusInt ConfigureServerTest(Utf8String serverName, bool silent = false);
    StatusInt CreateRealityDataTest();
    StatusInt CreateRelationshipToProject();
    StatusInt UploadTest1();
    StatusInt UploadTest2();
    StatusInt GetDocumentTest();
    StatusInt DownloadTest();
    StatusInt DeleteDocumentTest();
    StatusInt InformationExtractionTest();
    StatusInt UpdateTest();
    StatusInt DeleteRelationship();
    StatusInt GetRelationship();
    StatusInt DeleteRealityDataTest(bool silent = false);
    StatusInt GetRealityDataWithFilter();
    StatusInt GetRealityDataWithPolygon();
    StatusInt GetRealityData();
    StatusInt EnterpriseStatTest();
    StatusInt GetFolderTest();

    



    void DisplayInfo(Utf8StringCR msg, DisplayOption option= DisplayOption::Info);

private:    

    WSGServer       m_server;
    HANDLE          m_hConsole;
    Utf8String      m_tempFileName;
    RealityDataPtr  m_newRealityData;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE