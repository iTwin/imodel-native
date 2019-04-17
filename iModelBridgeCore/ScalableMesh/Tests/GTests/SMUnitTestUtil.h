/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/DgnPlatform.h>

#include <Bentley/BeFileName.h>
#include <json/json.h>

#ifndef SM_DATA_PATH
#define SM_DATA_PATH L"SMData"
#define SM_LISTING_FILE_NAME L"list.txt"
#endif

#ifndef SM_DATA_SOURCE_PATH
#define SM_DATA_SOURCE_PATH L"SMDataSource"
#endif

#ifndef SM_GROUND_DETECTION_PATH
#define SM_GROUND_DETECTION_PATH L"SMGroundDetectionData"
#endif

#ifndef SM_DISPLAY_QUERY_TEST_CASES
#define SM_DISPLAY_QUERY_TEST_CASES L"displayQueryTestCases.txt"
#endif

#ifndef SM_GROUND_DETECTION_TEST_CASES
#define SM_GROUND_DETECTION_TEST_CASES L"groundDetectionTestCases.txt"
#endif

//#define VANCOUVER_API
#ifndef VANCOUVER_API   
#include <DgnView/ViewManager.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DesktopTools/KnownDesktopLocationsAdmin.h>
#define VIEWMANAGER ViewManager
#else
#define VIEWMANAGER IViewManager
#include <DgnGeoCoord/DgnGeoCoord.h>

using namespace Bentley::GeoCoordinates;
#endif

#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh/IScalableMeshProgress.h>
//#include <iostream>

namespace ScalableMeshGTestUtil
    {
    bool GetDataPath(BeFileName& dataPath);
        
    BeFileName GetModuleFilePath();

    BeFileName GetModuleFileDirectory();

    BeFileName GetUserSMTempDir();

    BeFileName GetTempPathFromProjectPath(const BeFileName& path);

    enum class SMMeshType
        {
        TYPE_3SM,
        TYPE_3DTILES_TILESET,
        TYPE_3DTILES_B3DM,
        TYPE_3SM_SOURCE,
        TYPE_UNKNOWN
        };
    
    bvector<BeFileName> GetFiles(BeFileName dataPath, bool wantSource = false);

    size_t GetFileCount(BeFileName dataPath);

    Json::Value GetGroundTruthJsonFile(BeFileName dataPath);

	bvector<std::tuple<BeFileName, DMatrix4d, bvector<DPoint3d>, bvector<DPoint3d>>> GetListOfValues(BeFileName listingFile);

    bvector<std::tuple<BeFileName, DMatrix4d, bvector<DPoint4d>, bvector<double>>>   GetListOfDisplayQueryValues(BeFileName listingFile);

    bvector<std::tuple<BeFileName, bvector<DPoint3d>, uint64_t>>                     GetListOfGroundDetectionValues(BeFileName listingFile);
    
    SMMeshType GetFileType(BeFileName file);

    bool FilterEntry(BeFileName& entry, bool isDir, bool wantSource = false);

#ifdef VANCOUVER_API
    struct ScalableMeshModule : DgnViewLib::Host
#else
    struct ScalableMeshModule : DgnPlatformLib::Host
#endif
        {
        protected:

#ifndef VANCOUVER_API   
            virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() 
                override 
                { 
                return *new BentleyApi::Dgn::KnownDesktopLocationsAdmin(); 
                }
            virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }
            virtual void _SupplyProductName(Utf8StringR name) override { name.assign("ScalableMeshUnitTests"); }
#else
            virtual void _SupplyProductName(WStringR name) override { name.assign(L"ScalableMeshUnitTests"); }
#endif

#ifdef VANCOUVER_API   
            virtual VIEWMANAGER& _SupplyViewManager() override;
#endif

            virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        public:
            BentleyStatus Initialize();
        };

    bool InitScalableMesh();

    }
