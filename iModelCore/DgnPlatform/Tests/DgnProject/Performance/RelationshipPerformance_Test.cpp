/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/RelationshipPerformance_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//**** NOTE: This test needs to be ported to ECSqlStatement once DgnECPersistence was ported to ECSqlStatement ****

#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <Bentley/BeTimeUtilities.h>
#include <ECObjects/ECObjectsAPI.h>
#include <ECDb/ECDbApi.h>
#include <Logging/bentleylogging.h>
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_LOGGING
USING_DGNDB_UNIT_TESTS_NAMESPACE

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"Performance"))

extern int GetSeedInstances (ECInstanceKeyMultiMap& instanceKeyMap, DgnDbR project, ECClassId ecClassId, Utf8CP ecSqlToGetInstanceIds);

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Performance, Relationship_Retrieval_LinkTables)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm (L"BGRSubset.i.idgndb"); // ConstructSim i-model - Note that this version is published as part of a Windows build
    DgnDbR project = *tdm.GetDgnProjectP();
    ECDbStoreCR ecDbStore = project.GetEC();

    ECClassP groupingComponentClass = NULL;
    ecDbStore.Schemas().GetECClass (groupingComponentClass, "CSimProductData", "GroupingComponent");
    ASSERT_TRUE (groupingComponentClass != NULL);

    // First run
    StopWatch sw (L"", false);
    DgnECPersistence dgnECPersistence (project);
    bset<ElementId> elementIds;
    sw.Start();
    ECInstanceKeyMultiMap seedInstanceKeyMap;
    GetSeedInstances (seedInstanceKeyMap, project, groupingComponentClass->GetId(), "SELECT ECInstanceId FROM CSimProductData.GroupingComponent WHERE Spool_Fabrication == 'Spool Shipped'");
    dgnECPersistence.FindElements (elementIds, seedInstanceKeyMap, UINT8_MAX);
    sw.Stop();
    size_t expectedSize = 721;
    ASSERT_EQ (expectedSize, elementIds.size());
    LOG.tracev (L"First run time taken: %lf, Number of children: %d", sw.GetElapsedSeconds(), elementIds.size());

    // Second run
    sw.Start();
    dgnECPersistence.FindElements (elementIds, seedInstanceKeyMap, UINT8_MAX);
    sw.Stop();
    ASSERT_EQ (expectedSize, elementIds.size());
    LOG.tracev (L"Second run time taken: %lf, Number of children: %d", sw.GetElapsedSeconds(), elementIds.size());

    // Average through more runs
    int numRuns = 100;
    sw.Start();
    for (int ii=0; ii<numRuns; ii++)
        {
        dgnECPersistence.FindElements (elementIds, seedInstanceKeyMap, UINT8_MAX);
        }
    sw.Stop();
    ASSERT_EQ (expectedSize, elementIds.size());
    double averageTime = sw.GetElapsedSeconds() / double (numRuns);
    LOG.tracev (L"Average time (over %d runs): %lf, Number of children: %d", numRuns, averageTime, elementIds.size());

    // Log previous best times
#ifndef NDEBUG
    double previousBestTime = 0.120672;
    LOG.tracev (L"Previous best time in DEBUG build (over %d runs): %lf, Number of children: %d", numRuns, previousBestTime, expectedSize);
#else
    double previousBestTime = 0.054084;
    LOG.tracev (L"Previous best time in RELEASE build (over %d runs): %lf, Number of children: %d", numRuns, previousBestTime, expectedSize);
#endif
    }
#endif

#if defined (DGNPLATFORM_HAVE_DGN_IMPORTER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Performance, Relationship_Import_LinkTables)
    {
    // Measure time taken to import
    ScopedDgnHost autoDgnHost;
    DgnDbPtr project;
    StopWatch sw (L"", false);
    sw.Start();
    DgnDbTestDgnManager::CreateProjectFromDgn (project, DgnDbTestDgnManager::GetOutputFilePath (L"BGRSubset.idgndb"), 
        DgnDbTestDgnManager::GetSeedFilePath (L"BGRSubset.i.dgn.v8")); // ConstructSim i-model
    sw.Stop();
    ASSERT_TRUE( project != NULL);
    double actualTime = sw.GetElapsedSeconds();
#ifndef NDEBUG
    LOG.tracev (L"Time to import in DEBUG build: %lf", actualTime);
    double previousBestTime = 14.231158;
    LOG.tracev (L"Historic time in DEBUG build (with minimal logging for performance): %lf", previousBestTime);
#else
    LOG.tracev (L"Time to import in RELEASE build: %lf", actualTime);
    double previousBestTime = 9.881749;
    LOG.tracev (L"Previous best time in RELEASE build (with minimal logging for performance): %lf", previousBestTime);
#endif

    // Validate imported dgndb
    ECClassP groupingComponentClass = NULL;
    project->Schemas().GetECClass (groupingComponentClass, "CSimProductData", "GroupingComponent");
    ECInstanceKeyMultiMap seedInstanceKeyMap;
    GetSeedInstances (seedInstanceKeyMap, project, groupingComponentClass->GetId(), "SELECT ECInstanceId FROM CSimProductData.GroupingComponent WHERE Spool_Fabrication == 'Spool Shipped'");
    DgnECPersistence dgnECPersistence (*project);
    bset<ElementId> elementIds;
    dgnECPersistence.FindElements (elementIds, seedInstanceKeyMap, UINT8_MAX);
    ASSERT_EQ (721, elementIds.size());
    }

#endif

#define TEST_WITH_LARGE_FILES 0
/* These tests have been commented out by default since their test files are just 
 * too large to include in CVS, and the tests are only run on local machines. 
 * Seems useful to keep the source in CVS to occasionally monitor for performance 
 * and regressions.*/

#ifdef TEST_WITH_LARGE_FILES

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Performance, Relationship_ConstructSim)
    {
    ScopedDgnHost host;
    DgnDbTestDgnManager tdm (L"d:\\temp\\Graphite\\TestCases\\ByDiscipline\\ConstructSim\\BGRStatus.i.idgndb"); 

    DgnDbR project = *tdm.GetDgnProjectP();
    ECDbStoreCR ecDbStore = project.GetEC();

    // Prepare query
    ECClassP systemComponentClass = NULL;
    ecDbStore.Schemas().GetECClass (systemComponentClass, "CSimProductData", "SystemComponent");
    ASSERT_TRUE (systemComponentClass != NULL);

    // First run
    StopWatch sw (L"", false);
    DgnECPersistence dgnECPersistence (project);
    bset<ElementId> elementIds;
    sw.Start();
    ECInstanceKeyMultiMap seedInstanceKeyMap;
    GetSeedInstances (seedInstanceKeyMap, project, systemComponentClass->GetId(), "SELECT ECInstanceId FROM CSimProductData.SystemComponent WHERE Isometric_Release == 'ISO_IFC'");
    dgnECPersistence.FindElements (elementIds, seedInstanceKeyMap, UINT8_MAX);
    sw.Stop();
    LOG.tracev (L"First run time taken: %lf, Number of children: %d", sw.GetElapsedSeconds(), elementIds.size());
    ASSERT_EQ (16683, elementIds.size());

    // Second run
    //printf ("Start profiling. Press a key\n"); getchar();
    sw.Start();
    dgnECPersistence.FindElements (elementIds, seedInstanceKeyMap, UINT8_MAX);
    sw.Stop();
#ifndef NDEBUG
    LOG.tracev (L"Second run time taken: %lf, Number of children: %d", sw.GetElapsedSeconds(), elementIds.size());
    double previousBestTime = 2.011434;
    LOG.tracev (L"Previous best time in DEBUG build (with minimal logging for performance): %lf", previousBestTime);
#else
    LOG.tracev (L"Second run time taken: %lf, Number of children: %d", sw.GetElapsedSeconds(), elementIds.size());
    double previousBestTime = 1.110041;
    LOG.tracev (L"Previous best time in RELEASE build (with minimal logging for performance): %lf", previousBestTime);
#endif
    //printf ("End profiling. Press a key\n"); getchar();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Performance, Relationship_OpenPlant)
    {
    ScopedDgnHost host;
    DgnDbTestDgnManager tdm (L"d:\\temp\\Graphite\\TestCases\\ByDiscipline\\DeliveredSamples\\Hydrotreater Expansion.i.idgndb"); 

    DgnDbR project = *tdm.GetDgnProjectP();
    ECDbStoreCR ecDbStore = project.GetEC();

    // Prepare query
    ECClassP pipeClass = NULL;
    ecDbStore.Schemas().GetECClass (pipeClass, "OpenPlant_3D", "PIPE");
    ASSERT_TRUE (pipeClass != NULL);
    ECInstanceKeyMultiMap seedInstanceKeyMap;
    GetSeedInstances (seedInstanceKeyMap, project, pipeClass->GetId(), "SELECT ECInstanceId FROM OpenPlant_3D.PIPE WHERE LINENUMBER == '01-HCL-L3106-mEX-OPM'");
    
    // Get elements
    StopWatch sw (L"", false);
    DgnECPersistence dgnECPersistence (project);
    bset<ElementId> elementIds;
    sw.Start();
    BentleyStatus status = dgnECPersistence.FindElements (elementIds, seedInstanceKeyMap, UINT8_MAX);
    sw.Stop();
    LOG.tracev (L"First run time taken: %lf, Number of children: %d", sw.GetElapsedSeconds(), elementIds.size());
    ASSERT_EQ (7, elementIds.size());
    ASSERT_EQ (SUCCESS, status);
    }
#endif

#endif
