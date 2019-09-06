/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <Windows.h>
#include <ctime>

#include <Bentley/DateTime.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE



struct RealityDataServicePerformanceTests
    {

public:
    struct timeStats
        {
        int64_t m_handshake;
        int64_t m_createRealityData;
        int64_t m_createRelationship;
        int64_t m_uploadTest;
        int64_t m_downloadTest;
        int64_t m_getDocument;
        int64_t m_getRealityData;
        int64_t m_listRealityData;
        int64_t m_getRelationship;
        int64_t m_modifyRealityData;
        int64_t m_deleteRelationship;
        int64_t m_deleteRealityData;
        int64_t m_dataLocationStats;
        int64_t m_enterpriseStats;
        int64_t m_serviceStats;
        int64_t m_userStats;
        };


    enum class DisplayOption
    {
        Info,
        Question,
        Tip,
        Error
    };
    RealityDataServicePerformanceTests();
    ~RealityDataServicePerformanceTests();

    void Run(Utf8String serverName, int numberOfLoops);
    void Usage();
    StatusInt ConfigureServerTest(Utf8String serverName, timeStats& theTimeStats, bool silent = false);
    StatusInt CreateRealityDataTest(timeStats& theTimeStats);
    StatusInt CreateRelationshipToProject(timeStats& theTimeStats);
    StatusInt UploadTest1(timeStats& theTimeStats);
    StatusInt UploadTest2(timeStats& theTimeStats);
    StatusInt GetDocumentTest(timeStats& theTimeStats);
    StatusInt DownloadTest(timeStats& theTimeStats);
    StatusInt DeleteDocumentTest(timeStats& theTimeStats);
    StatusInt UpdateTest(timeStats& theTimeStats);
    StatusInt DeleteRelationship(timeStats& theTimeStats);
    StatusInt GetRelationship(timeStats& theTimeStats);
    StatusInt DeleteRealityDataTest(timeStats& theTimeStats, bool silent = false);
    StatusInt GetRealityDataWithFilter(timeStats& theTimeStats);
    StatusInt GetRealityDataWithPolygon(timeStats& theTimeStats);
    StatusInt GetRealityData(timeStats& theTimeStats);
    StatusInt DataLocationStatTest(timeStats& theTimeStats);
    StatusInt EnterpriseStatTest(timeStats& theTimeStats);
    StatusInt ServiceStatTest(timeStats& theTimeStats);
    StatusInt UserStatTest(timeStats& theTimeStats);
    StatusInt GetFolderTest(timeStats& theTimeStats);

    
    void ComputeAndPrintStats();


    void DisplayInfo(Utf8StringCR msg, DisplayOption option= DisplayOption::Info);


private:    

    WSGServer       m_server;
    HANDLE          m_hConsole;
    Utf8String      m_tempFileName;
    RealityDataPtr  m_newRealityData;

    bvector<timeStats> m_listOfStats;

    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE