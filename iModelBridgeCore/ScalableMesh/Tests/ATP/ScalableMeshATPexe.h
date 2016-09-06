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
#include <DgnView/ViewManager.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
#include <BeXml/BeXml.h>
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

USING_NAMESPACE_BENTLEY_SCALABLEMESH
namespace ScalableMeshATPexe
{

struct ExeViewManager : ViewManager
    {
    protected:
        virtual DgnDisplay::QvSystemContextP _GetQvSystemContext() override { return nullptr; }
        virtual bool                _DoesHostHaveFocus()        override { return true; }
        virtual IndexedViewSetR     _GetActiveViewSet()         override { return *(IndexedViewSetP)nullptr; }
        virtual int                 _GetDynamicsStopInterval()  override { return 200; }

    public:
        ExeViewManager() {}
        ~ExeViewManager() {}
    };

struct ScalableMeshATPexe : DgnViewLib::Host
    {
    protected:
        enum class ParseStatus { Success, Error, NotRecognized };

        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new BentleyApi::Dgn::WindowsKnownLocationsAdmin(); }
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }

        virtual void _SupplyProductName(Utf8StringR name) override { name.assign("ScalableMeshATPexe"); }
        virtual ViewManager& _SupplyViewManager() override { return *new ExeViewManager(); }
        virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin();

        BeFileName          m_inputFileName;
        bool                m_optionClean;
        IScalableMeshPtr    m_sMesh;
        HANDLE m_pipe;

        WString GetArgValueW(WCharCP arg);
        Utf8String GetArgValue(WCharCP arg);
        void OpenPipe();

    public:
        BeFileName          m_outputName;
        ScalableMeshATPexe() : m_sMesh(nullptr), m_pipe(NULL) {}
        int PrintUsage(WCharCP programName);
        int ParseCommandLine(int argc, WCharP argv[]);
        BentleyStatus Initialize(int argc, WCharP argv[]);
        BentleyStatus OpenScalableMesh(WCharCP path);
        size_t CountPoints();
        BentleyStatus ParseImportDefinition(BeXmlNodeP pTestNode);
        void Start();
        void Import();



    };




};