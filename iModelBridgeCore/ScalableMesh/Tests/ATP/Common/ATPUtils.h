#pragma once
#include <Bentley/Bentley.h>
#include <Bentley\BeStringUtilities.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>
#include <Bentley/bvector.h>
#include <BeXml/BeXml.h>
#include <Geom/GeomApi.h>
#include <Mtg/MtgStructs.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <TerrainModel/TerrainModel.h>

#ifdef VANCOUVER_API
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#endif

using namespace std;

typedef enum
    {
    TEST_GENERATION = 0,
    TEST_PARTIAL_UPDATE,
    TEST_QUALITY_MESH,
    TEST_QUALITY_STITCH,
    TEST_DRAPE_LINE,
    TEST_VOLUME,
    TEST_SELF_CONTAINED_IMPORTER,
    TEST_IMPORT_FEATURES_GIS,
    TEST_GROUND_DETECTION_BASELINE,
    TEST_OPTIMIZE_GROUND_PARAMETERS,
    TEST_ADD_NODES,
    TEST_LOADING,
    TEST_DRAPE_BASELINE,
    TEST_CONSTRAINTS,
    TEST_SDK_MESH,
    TEST_STREAMING,
    TEST_SCM_TO_CLOUD,
    TEST_RANDOM_DRAPE,
    EXPORT_LINE,
    EXPORT_VOLUME,
    IMPORT_VOLUME,
    TEST_GROUP_NODE_HEADERS,
    ADD_TEXTURES_TO_MESH,
	EXPORT_TO_UNITY,
    } TestType;


bool RunTestPlan(BeFileName& testPlanPath);
bool RemoveStmFiles(BeFileName& inputFileName);
void ReadLinesFile(std::ifstream& file, bvector<bvector<DPoint3d>>& lineDefs);
void ReadLinesFromDgnFile(WString& fileName, bvector<bvector<DPoint3d>>& lineDefs);
void ReadFeatureFile(std::ifstream& file, std::vector<std::pair<std::vector<DPoint3d>, DTMFeatureType>>& features);
bool DPoint3dEqualityTest(const DPoint3d& point1, const DPoint3d& point2);