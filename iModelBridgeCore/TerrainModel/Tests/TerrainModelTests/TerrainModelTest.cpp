/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TerrainModelTests/TerrainModelTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma warning(disable:4505) // unreferenced local function has been removed [in gtest-port.h]

#include "stdafx.h"
#pragma warning(disable:4189) // unreferenced local function has been removed [in gtest-port.h]

StackExaminer* g_stackExaminer = NULL;
BSIBaseGeomExaminer* g_bsiBaseGeomExaminer = NULL;

WString s_dllPath = L"";

static void* getDLLInstance()
    {
    MEMORY_BASIC_INFORMATION    mbi;
    if (VirtualQuery((void*)&getDLLInstance, &mbi, sizeof mbi))
        return mbi.AllocationBase;

    return 0;
    }

/*---------------------------------------------------------------------------------******
 * @bsimethod                                    Carole.MacDonald                08/2010
 +---------------+---------------+---------------+---------------+---------------+------*/
static WString GetDllPath()
    {
    if (s_dllPath.empty())
        {
        HINSTANCE ecobjectsHInstance = (HINSTANCE)getDLLInstance();
        wchar_t strExePath[MAX_PATH];
        if (0 == (GetModuleFileNameW(ecobjectsHInstance, strExePath, MAX_PATH)))
            return L"";

        wchar_t executingDirectory[_MAX_DIR];
        wchar_t executingDrive[_MAX_DRIVE];
        _wsplitpath(strExePath, executingDrive, executingDirectory, NULL, NULL);
        wchar_t filepath[MAX_PATH];
        _wmakepath(filepath, executingDrive, executingDirectory, NULL, NULL);
        s_dllPath = filepath;
        }
    return s_dllPath;
    }
/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                                                    Carole.MacDonald 02/10
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
WString TMHelpers::GetTestDataPath(WCharCP dataFile)
    {
    WString testData(GetDllPath());
    //testData.append(L"Data\\");
    testData.append(dataFile);
    return testData;
    }

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

BcDTMPtr TMHelpers::LoadTerrainModel(WCharCP filename, WCharCP name)
    {
    WString fullFilePath = GetTestDataPath(filename);

    printf("Loading File %ls\n", filename);
    TerrainImporterPtr reader = TerrainImporter::CreateImporter(fullFilePath.c_str());
    WString surfaceName;

    if (name == nullptr)
        {
        TerrainInfoList surfaceInfos = reader->GetTerrains();

        if (surfaceInfos.size() == 0)
            return nullptr;

        surfaceName = surfaceInfos[0].GetName();
        }
    else
        surfaceName = name;

    ImportedTerrain dtm = reader->ImportTerrain(surfaceName.GetWCharCP());

    return dtm.GetTerrain();
    }

bool TMHelpers::ValidateTM(BcDTMR dtm, const TMHelpers::ValidateParams& params)
    {
    if (dtm.CheckTriangulation() != SUCCESS)
        return false;

    if (dtm.GetDTMState() != DTMState::Tin)
        if (dtm.Triangulate() != SUCCESS)
            return false;

    int numFeatures = 0;
    DTMFeatureEnumeratorPtr featureEnum = DTMFeatureEnumerator::Create(dtm);

    for (const auto& featureInfo : *featureEnum)
        {
        numFeatures++;
        }
    featureEnum = nullptr;
    printf("Num of features %d\n", numFeatures);

    DTMMeshEnumeratorPtr meshEnum = DTMMeshEnumerator::Create(dtm);

    for (auto poly : *meshEnum)
        {
        }
    meshEnum = nullptr;

    int numContours = 0;
    dtm.BrowseContours(params.contourParams, params.fence, nullptr, [&numContours](DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg)
        {
        numContours++;
        return SUCCESS;
        });
    printf("Num of contours %d\n", numContours);

    double flatArea = 0, slopeArea = 0;
    dtm.CalculateSlopeArea(flatArea, slopeArea, nullptr, 0);
    printf("Num of flatArea %f, slopeArea %f\n", flatArea, slopeArea);


    BcDTMVolumeAreaResult result;
    dtm.ComputePlanarPrismoidalVolume(result, 0, nullptr, 0, nullptr, 0);

    printf("CutVolume %f FillVolume %f\n", result.cutVolume, result.fillVolume);

    return true;
    }

int main(int argc, char **argv)
    {
#if defined (_DEBUG)
    int dbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    /*
    * Set the debug-heap flag to no longer keep freed blocks in the
    * heap's linked list and turn on Debug type allocations (CLIENT)
    */
    dbgFlag |= _CRTDBG_ALLOC_MEM_DF;
    dbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;

    // Clear the upper 16 bits and OR in the desired freqency
    dbgFlag = (dbgFlag & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_1024_DF;

    _CrtSetDbgFlag(dbgFlag);
#endif

#ifdef argHandler
    ArgHandlerList handlers;
    Args arghandlers;
    arghandlers.AddArgsToList(handlers);

    CommandLineArgHandler args(handlers, argc, argv);

    int status = 0;
    if (args.ShouldRunTests())
        {
        ::testing::InitGoogleTest(args.GetCountP(), args.GetArgVP());

        g_stackExaminer = new StackExaminer;  // this must be alloced because gtest frees it up.
        ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
        listeners.Append(g_stackExaminer);

        status = RUN_ALL_TESTS();
        }
#else

    ::testing::InitGoogleTest(&argc, argv);

    g_stackExaminer = new StackExaminer;  // this must be alloced because gtest frees it up.
    g_bsiBaseGeomExaminer = new BSIBaseGeomExaminer();  // this must be alloced because gtest frees it up.
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(g_stackExaminer);
    listeners.Append(g_bsiBaseGeomExaminer);

#endif
    int stat = RUN_ALL_TESTS();
    g_stackExaminer->DumpStackInfo();

    printf(" BSIBaseGeom counters: (Malloc %lld) (Calloc %lld) (Realloc %lld) (Free %lld) (DIFF %lld)\n",
        BSIBaseGeom::GetNumMalloc(),
        BSIBaseGeom::GetNumCalloc(),
        BSIBaseGeom::GetNumRealloc(),
        BSIBaseGeom::GetNumFree(),
        BSIBaseGeom::GetAllocationDifference());

    return stat;
    }
