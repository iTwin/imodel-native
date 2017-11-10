/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestUtil.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/DgnPlatform.h>

#include <Bentley/BeFileName.h>

#ifndef SM_DATA_PATH
#define SM_DATA_PATH L"SMData"
#define SM_LISTING_FILE_NAME L"list.txt"
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

namespace ScalableMeshGTestUtil
    {
    bool GetDataPath(BeFileName& dataPath);
        
    BeFileName GetModuleFilePath();

    BeFileName GetModuleFileDirectory();

    BeFileName GetUserSMTempDir();

    enum class SMMeshType
        {
        TYPE_3SM,
        TYPE_3DTILES,
        TYPE_UNKNOWN
        };

    bvector<BeFileName> GetFiles(BeFileName dataPath);

	bvector<std::tuple<BeFileName, DMatrix4d, bvector<DPoint3d>, bvector<DPoint3d>>> GetListOfValues(BeFileName listingFile);
    
    SMMeshType GetFileType(BeFileName file);

    bool FilterEntry(BeFileName& entry, bool isDir);

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
