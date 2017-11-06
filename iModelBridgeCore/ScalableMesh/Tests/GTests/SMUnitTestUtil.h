/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestUtil.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Windows.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnView/DgnViewLib.h>
#include <Bentley/BeFileName.h>

#ifndef SM_DATA_PATH
#define SM_DATA_PATH L"SMData"
#endif

#define VANCOUVER_API
#ifndef VANCOUVER_API   
#include <DgnView/ViewManager.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
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
    
    SMMeshType GetFileType(BeFileName file);

    bool FilterEntry(BeFileName& entry, bool isDir);

    struct ScalableMeshModule : DgnViewLib::Host
        {
        protected:

#ifndef VANCOUVER_API   
            virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new BentleyApi::Dgn::WindowsKnownLocationsAdmin(); }
            virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }
            virtual void _SupplyProductName(Utf8StringR name) override { name.assign("ScalableMeshUnitTests"); }
#else
            virtual void _SupplyProductName(WStringR name) override { name.assign(L"ScalableMeshUnitTests"); }
#endif

            virtual VIEWMANAGER& _SupplyViewManager() override;

            virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        public:
            BentleyStatus Initialize();
        };

    bool InitScalableMesh();
    }
