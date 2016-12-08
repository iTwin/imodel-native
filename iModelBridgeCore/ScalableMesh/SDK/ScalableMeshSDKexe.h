#pragma once

#include <list>
#include <iostream>
#include <windows.h> //used for pipes
#include <DgnPlatform/DgnPlatform.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ImagePP\h\ImageppAPI.h>
#include <ScalableMesh\IScalableMeshSourceImporter.h>
#include <Bentley/BeFileListIterator.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\PointCloudHandler.h>
#include <DgnGeoCoord\DgnGeoCoord.h>
#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudDisplayHandler.h>
#include <RasterCore/RasterCoreLib.h>
#include <DgnGeoCoord\DgnGeoCoord.h>
#include <BeXml/BeXml.h>
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

USING_NAMESPACE_BENTLEY_SCALABLEMESH
namespace ScalableMeshSDKexe
    {


    struct ImportParameters
        {
        bool decimateInput;
        WString tempPath;
        uint32_t maximumNbOfPoints;
        WString gcsKeyName;
        BeXmlNodeP pRootNode;
        };

    struct ExeViewManager : IViewManager
        {
        protected:
            virtual bool _DoesHostHaveFocus() override { return true; }
            virtual IndexedViewSet& _GetActiveViewSet() override { return *(IndexedViewSet*)nullptr; }
            virtual int _GetDynamicsStopInterval() override { return 200; }
            virtual DgnDisplayCoreTypes::Window* _GetTopWindow(int) override { return nullptr; }
            virtual int _GetCurrentViewNumber() override { return 0; }
            virtual HUDManager* _GetHUDManager() override { return nullptr; }
        };

    struct ScalableMeshSDKexe : DgnViewLib::Host
        {
        protected:
            enum class ParseStatus { Success, Error, NotRecognized };

        virtual void _SupplyProductName(BENTLEY_NAMESPACE_NAME::WStringR name) override { name.assign(L"ScalableMeshSDKexe"); }
        virtual IViewManager& _SupplyViewManager() override { return *new ExeViewManager(); }
        virtual DgnPlatformLib::Host::RasterAttachmentAdmin&  _SupplyRasterAttachmentAdmin() { return BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::GetDefaultRasterAttachmentAdmin(); }
        virtual DgnPlatformLib::Host::PointCloudAdmin&    _SupplyPointCloudAdmin() { return *new BENTLEY_NAMESPACE_NAME::DgnPlatform::PointCloudDisplayAdmin(); }
        virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() { return *GeoCoordinates::DgnGeoCoordinationAdmin::Create(NULL, *m_acsManager); }

            BeFileName          m_inputFileName;
            BeXmlDomPtr         m_pImportDefinitionXmlDom;
            IScalableMeshPtr    m_sMesh;
            HANDLE m_pipe;

            WString GetArgValueW(WCharCP arg);
            Utf8String GetArgValue(WCharCP arg);
            void OpenPipe();

        public:
            BeFileName          m_outputName;
            ScalableMeshSDKexe() : m_sMesh(nullptr), m_pipe(NULL){}
            int PrintUsage(WCharCP programName);
            int ParseCommandLine(int argc, WCharP argv[]);
            BentleyStatus Initialize(int argc, WCharP argv[]);
            BentleyStatus OpenScalableMesh(WCharCP path);
            size_t CountPoints();
            BentleyStatus DoImportSTM(const ImportParameters& params);
            BentleyStatus DoImportDirect(const ImportParameters& params);
            BentleyStatus ParseImportDefinition(BeXmlNodeP pTestNode, ImportParameters& params);
            bool OpenXmlImportFile();
            void Start();
            void Import();



        };
        


   

    };