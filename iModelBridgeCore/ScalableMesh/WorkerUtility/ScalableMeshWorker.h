#pragma once

#include <list>
#include <iostream>
#include <windows.h> //used for pipes
#include <DgnPlatform/DgnPlatform.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ImagePP\h\ImageppAPI.h>
#include <Bentley/BeFileListIterator.h>
#include <DgnPlatform/DgnPlatformLib.h>

#ifndef VANCOUVER_API   
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform\DesktopTools\KnownDesktopLocationsAdmin.h>
#define VIEWMANAGER ViewManager
#else
#define VIEWMANAGER IViewManager
#include <DgnView\IViewManager.h>
#include <DgnGeoCoord/DgnGeoCoord.h>
#include <DgnView\DgnViewLib.h>

using namespace Bentley::GeoCoordinates;
#endif

#include <BeXml/BeXml.h>
USING_NAMESPACE_BENTLEY

#ifdef VANCOUVER_API
    USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
    USING_NAMESPACE_BENTLEY_DGN
#endif

#include "SMWorkerDefinitions.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH


BEGIN_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE

#ifdef VANCOUVER_API

struct ExeViewManager : VIEWMANAGER
    {
    protected:
#ifndef VANCOUVER_API   
        virtual DgnDisplay::QvSystemContextP _GetQvSystemContext() override { return nullptr; }
#else
        virtual Bentley::DgnPlatform::DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override { return nullptr; }
        virtual int                                                _GetCurrentViewNumber() override { return 0; }
        virtual HUDManager*                                        _GetHUDManager() { return nullptr; }
#endif

        virtual bool                _DoesHostHaveFocus()        override { return true; }
        virtual IndexedViewSetR     _GetActiveViewSet()         override { return *(IndexedViewSetP)nullptr; }
        virtual int                 _GetDynamicsStopInterval()  override { return 200; }

    public:
        ExeViewManager() {}
        ~ExeViewManager() {}
    };

#endif

#ifdef VANCOUVER_API
struct ScalableMeshWorker : DgnViewLib::Host
#else
struct ScalableMeshWorker : DgnPlatformLib::Host
#endif
    {
    protected:
        enum class ParseStatus { Success, Error, NotRecognized };

#ifndef VANCOUVER_API   
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new Dgn::KnownDesktopLocationsAdmin(); }
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }
        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("ScalableMeshWorker"); }
#else
        virtual void _SupplyProductName(WStringR name) override { name.assign(L"ScalableMeshWorker"); }
#endif

#ifdef VANCOUVER_API
        virtual VIEWMANAGER& _SupplyViewManager() override { return *new ExeViewManager(); }
#endif
        virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        BeFileName m_taskFolderName;
        BeFileName m_startingIndexTask;       
        uint16_t   m_nbExtraWorkers;
        Utf8String m_workerProcessName;
        bool       m_useGroupingStrategy;

        WString GetArgValueW(WCharCP arg);
        Utf8String GetArgValue(WCharCP arg);        

    public:
        BeFileName          m_outputName;

        ScalableMeshWorker()
            {
            m_nbExtraWorkers = 0;
            m_useGroupingStrategy = false;
            }
        
        int PrintUsage(WCharCP programName);
        int ParseCommandLine(int argc, WCharP argv[]);
        BentleyStatus Initialize(int argc, WCharP argv[]);
        BentleyStatus OpenScalableMesh(WCharCP path);
        size_t CountPoints();
        BentleyStatus ParseImportDefinition(BeXmlNodeP pTestNode);
        void Start();
        void Import();
    };

END_BENTLEY_SCALABLEMESH_WORKER_NAMESPACE