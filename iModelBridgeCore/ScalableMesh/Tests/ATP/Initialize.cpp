#include <ScalableMeshATPPch.h>

#include "Initialize.h"

using namespace std;
#include <DgnPlatform\IAuxCoordSys.h>
/*#include <DgnPlatform/DelegatedElementECEnabler.h>
#include <DgnPlatform\ITransactionHandler.h>
#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\PointCloudHandler.h>
#include <DgnGeoCoord\DgnGeoCoord.h>
#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudDisplayHandler.h>*/
/*#include <RasterCore/DgnRaster.h>
#include <RasterCore/RasterCore.h>
#include <RasterCore\RasterCoreLib.h>*/
//#include <RmgrTools\Tools\RscFileManager.h>
#include <TerrainModel\TerrainModel.h>
#include <ScalableMesh\ScalableMeshDefs.h>
#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh\IScalableMeshStream.h>
#include <ScalableMesh\IScalableMeshURL.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnView/DgnViewAPI.h>
//#include <RmgrTools/Tools/RscFileManager.h>
#include <ScalableMesh/IScalableMeshMoniker.h>
#include <ScalableMesh/IScalableMeshStream.h>
#include <ScalableMesh/IScalableMeshURL.h>
/*#include <ScalableTerrainModel\IMrDTMMoniker.h>
#include <ScalableTerrainModel\IMrDTMStream.h>
#include <ScalableTerrainModel\IMrDTMURL.h>*/
#include <DgnView/DgnViewLib.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnGeoCoord.h>

#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>

//#include <DgnPlatform\Tools\ConfigurationManager.h>

//USING_NAMESPACE_RASTER

//#include <RasterCore/RasterCoreAPI.h>
//#include <RasterCore/HIERasterSourceObserver.h>

//#include <RasterCore/DgnRaster.h>

//#include <RasterCore/RasterCoreLib.h>



USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN
USING_NAMESPACE_BENTLEY_DGNPLATFORM
//USING_NAMESPACE_BENTLEY_MRDTM
using namespace BENTLEY_NAMESPACE_NAME::DgnPlatform;
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

namespace ScalableMeshATPexe
{
struct AppViewManager : ViewManager
    {
    protected:
        virtual DgnDisplay::QvSystemContextP _GetQvSystemContext() override { return nullptr; }
        virtual bool                _DoesHostHaveFocus()        override { return true; }
        virtual IndexedViewSetR     _GetActiveViewSet()         override { return *(IndexedViewSetP)nullptr; }
        virtual bool                _ForceSoftwareRendering()  override;
        virtual int                 _GetDynamicsStopInterval()  override { return 200; }

    public:
        AppViewManager() {}
        ~AppViewManager() {}
    };
/*    struct AppViewManager : BentleyApi::Dgn::IViewManager
        {
        private:
            IndexedViewSet* m_activeViewSet;
            HWND                                  m_topWindow;

            virtual DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override {return (DgnDisplayCoreTypes::WindowP)m_topWindow;}
            virtual bool                                               _DoesHostHaveFocus() {return false;}
            virtual IndexedViewSet&              _GetActiveViewSet() override {assert(!"Not expect to be call in offline mode"); return *m_activeViewSet;}
            virtual int             _GetCurrentViewNumber() override {return 0;}
            virtual HUDManagerP     _GetHUDManager () {return NULL;}

        public:

            //void SetActiveDgn (AppViewSet* newDgn) {m_activeViewSet = newDgn;}
            //void SetActiveDgn (DemoViewSet* newDgn) {m_activeViewSet = newDgn;}
            AppViewManager() {m_activeViewSet = NULL; m_topWindow = NULL;}
        };
        */
        //=======================================================================================
        // @bsiclass                                                    Keith.Bentley   01/10
        //=======================================================================================
class AppHost : public DgnViewLib::Host
    {
    //            AppViewManager   m_viewManager;

    protected:

        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override { return *new BentleyApi::Dgn::WindowsKnownLocationsAdmin(); }
        virtual DgnPlatformLib::Host::NotificationAdmin&  _SupplyNotificationAdmin() override;
        virtual void                                      _SupplyProductName(Utf8StringR name) override;
        //            virtual DgnFileIOLib::Host::DigitalRightsManager& _SupplyDigitalRightsManager() override;     
                    //virtual GraphicsAdmin&                                                 _SupplyGraphicsAdmin() override;            
                    //virtual ViewStateAdmin&                                                _SupplyViewStateAdmin() override;           
                    //virtual ToolAdmin&                                                     _SupplyToolAdmin() override;                
        //            virtual IViewManager&                             _SupplyViewManager() override;              
                    //virtual SolidsKernelAdmin&                                             _SupplySolidsKernelAdmin() override;        
        virtual DgnPlatformLib::Host::RasterAttachmentAdmin&      _SupplyRasterAttachmentAdmin() override;
        virtual DgnPlatformLib::Host::PointCloudAdmin&            _SupplyPointCloudAdmin() override;
        //virtual FontAdmin&                                                     _SupplyFontAdmin() override;                
        //virtual MaterialAdmin&                                                 _SupplyMaterialAdmin();                     
        //virtual ProgressiveDisplayManager&                                     _SupplyProgressiveDisplayManager() override;
        virtual DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() override;
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() { return BeSQLite::L10N::SqlangFiles(BeFileName()); }
        virtual ViewManager& _SupplyViewManager() override { return *new AppViewManager(); }

    public:
        void Startup(/*HWND*/);

        void Terminate();

        //DemoViewManager& GetDemoViewManager(){return m_viewManager;}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
struct AppNotificationAdmin : DgnPlatformLib::Host::NotificationAdmin
    {
    virtual StatusInt _OutputMessage(BentleyApi::Dgn::NotifyMessageDetails const& msg) override;
    virtual void      _OutputPrompt(Utf8CP) override;
    virtual NotificationManager::MessageBoxValue _OpenMessageBox(NotificationManager::MessageBoxType, Utf8CP, NotificationManager::MessageBoxIconType) override;
    };

/*=================================================================================**//**
* DgnViewDemo only displays the contents of a file on the screen. It does not EXPORT
* engineering data. Therefore, it is safe for DgnViewDemo to open rights-restricted DGN files.
* @bsiclass                                     Sam.Wilson                      06/2010
+===============+===============+===============+===============+===============+======*/
/*        struct ReadOnlyDigitalRightsManager : DgnFileIOLib::Host::DigitalRightsManager
        {
        //    virtual StatusInt _OnEnterRestrictedMode (bool assertKeys, BENTLEY_NAMESPACE_NAME::DgnFileProtection::KeyMaterialWithDescription* keylist, uint32_t nkeys, BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileP file, uint32_t rights) override {return SUCCESS;}
        };
        */


        /*=================================================================================**//**
        * @bsiclass
        +===============+===============+===============+===============+===============+======*/
        /*struct AppRasterCoreAdmin : BentleyApi::Dgn::Raster::RasterCoreAdmin
        {
        virtual bool                    _IsProgressiveDisplayEnabled() const    {return true;}
        };*/

        /*=================================================================================**//**
        * @bsiclass                                     		Marc.Bedard     02/2011
        +===============+===============+===============+===============+===============+======*/
        /*struct AppRasterCoreLibHost : Raster::RasterCoreLib::Host
        {
        virtual Raster::RasterCoreAdmin& _SupplyRasterCoreAdmin() override {return *new AppRasterCoreAdmin();}
        }; // RasterCoreLib::Host
        */
void AppHost::Startup(/*HWND*/)
    {
    //InitMonikerFactories();

    //IScalableMeshAdmin::SetCanImportPODFile(true);    

    //Init GDAL path.    
//    WString gdalDataPath(L".\\Gdal_Data\\");        
//    ConfigurationManager::DefineVariable (L"_USTN_RASTERGDALDATA", gdalDataPath.c_str());

//    RscFileManager::StaticInitialize(L"en");

    DgnViewLib::Initialize(*this, true);

    //Application needs to initialize PdfLibInitializer dll if it wants support for PDF raster attachment.
    //BENTLEY_NAMESPACE_NAME::PdfLibInitializer::Initialize(*new ViewDemoPdfLibInitializerHost());

//    Raster::RasterCoreLib::Initialize (*new AppRasterCoreLibHost());
//    BeAssert (Raster::RasterCoreLib::IsInitialized());

    //Ensure basegeocoord is initialized.
    _SupplyGeoCoordinationAdmin()._GetServices();

    //Required for reading TM element
    //BENTLEY_NAMESPACE_NAME::TerrainModel::Element::DTMElementHandlerManager::InitializeForOfflineTmImport();

    //m_viewManager.SetTopWindow(topWindow);

    // Setup location to find parasolid schema files (schema directory was created under exe location)...
    /*
    WString baseDir;
    getBaseDirOfExecutingModule (baseDir);

    ConfigurationManager::DefineVariable (L"MS_SMARTSOLID", baseDir.c_str ());
    */
    }

void AppHost::Terminate()
    {
    //call rasterCore cleanup code
//    Raster::RasterCoreLib::GetHost().Terminate(true);

    //call PdfLibInitializer dll cleanup code.
    //BENTLEY_NAMESPACE_NAME::PdfLibInitializer::GetHost().Terminate(true);
    }

void                                                                   AppHost::_SupplyProductName(Utf8StringR name) { name.assign("DgnView Demo"); }
DgnPlatformLib::Host::NotificationAdmin&         AppHost::_SupplyNotificationAdmin() { return *new AppNotificationAdmin(); }
//DgnFileIOLib::Host::DigitalRightsManager&        AppHost::_SupplyDigitalRightsManager()       {return *new ReadOnlyDigitalRightsManager;}
//GraphicsAdmin&             AppHost::_SupplyGraphicsAdmin()              {return *new DgnViewGraphicsAdmin();}
//ViewStateAdmin&            AppHost::_SupplyViewStateAdmin()             {return *new DemoViewStateAdmin();}
//ToolAdmin&                 AppHost::_SupplyToolAdmin()                  {return *new DemoToolAdmin();}
//IViewManager&                                    AppHost::_SupplyViewManager()                {return m_viewManager; }
//SolidsKernelAdmin&         AppHost::_SupplySolidsKernelAdmin()          {return *new AppSolidKernelAdmin();}
//DgnPlatformLib::Host::RasterAttachmentAdmin&     AppHost::_SupplyRasterAttachmentAdmin()      {return Raster::RasterCoreLib::GetDefaultRasterAttachmentAdmin();}
//DgnPlatformLib::Host::PointCloudAdmin&           AppHost::_SupplyPointCloudAdmin()            {return *new PointCloudDisplayAdmin();}
//FontAdmin&                 AppHost::_SupplyFontAdmin()                  {return *new DemoFontAdmin();}
//MaterialAdmin&             AppHost::_SupplyMaterialAdmin()              {return *new DemoMaterialAdmin(); }
//ProgressiveDisplayManager& AppHost::_SupplyProgressiveDisplayManager()  {return *new DemoProgressiveDisplayManager(); }
DgnPlatformLib::Host::GeoCoordinationAdmin&      AppHost::_SupplyGeoCoordinationAdmin()
    {

    //WString geocoordinateDataPath(L".\\GeoCoordinateData\\");
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");

    //return *DgnGeoCoordinationAdmin::Create (geocoordinateDataPath.c_str(), IACSManager::GetManager()); 
    return *DgnGeoCoordinationAdmin::Create(geocoordinateDataPath);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+-****/
StatusInt AppNotificationAdmin::_OutputMessage(NotifyMessageDetails const& msg)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      AppNotificationAdmin::_OutputPrompt(Utf8CP prompt)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationManager::MessageBoxValue AppNotificationAdmin::_OpenMessageBox(NotificationManager::MessageBoxType, Utf8CP message, NotificationManager::MessageBoxIconType iconType)
    {
    return NotificationManager::MESSAGEBOX_VALUE_Ok;
    }



struct ExeAdmin : BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshAdmin
    {
    DgnModel* s_activeDgnModelRefP;
    ExeAdmin() :s_activeDgnModelRefP(0) {};

    ~ExeAdmin() {};

    bool _CanImportPODfile() const
        {
        return true;
        }

    void SetActiveModelRef(DgnModel* activeDgnModelRefP)
        {
        s_activeDgnModelRefP = activeDgnModelRefP;
        }

    DgnModel* _GetActiveModelRef() const
        {
        return s_activeDgnModelRefP;
        }

    };

struct ExeHost : BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshLib::Host
    {

    ExeHost()
        {
        BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshLib::Initialize(*this);
        }
    BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        return *new ExeAdmin();
        };
    };


    
    
/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*class MyMSDocumentMoniker : public BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMoniker
    {
private:    

    BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocumentMoniker> m_mrdtmMonikerPtr;

    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::LocalFileURL                _GetURL                        (StatusInt&                          status) const override
        {
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));
        
        if (BSISUCCESS != status)
            fileName = m_mrdtmMonikerPtr->GetPortableName(); // File not found. Return s_dgnFile name with configuration variable.

        return BENTLEY_NAMESPACE_NAME::ScalableMesh::LocalFileURL(fileName.c_str());
        }




    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceMonikerType        _GetType                       () const override
        {
        return BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


    virtual bool                        _IsTargetReachable             () const override
        {
        StatusInt status;
        m_mrdtmMonikerPtr->ResolveFileName(&status);
        return BSISUCCESS == status;
        }

    virtual StatusInt                   _Serialize                     (BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::SourceDataSQLite&                      sourceData,
                                                                        const BENTLEY_NAMESPACE_NAME::ScalableMesh::DocumentEnv&                  env) const override
        {
        // TDORAY: Recreate the moniker using new env prior to serializing it in order so
        // that relative path is correct on s_dgnFile moves...

            const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
            sourceData.SetMonikerString(monikerString);

        return BSISUCCESS;
        }

    explicit                            MyMSDocumentMoniker        (const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         monikerPtr)
        :   m_mrdtmMonikerPtr(monikerPtr)
        {
        assert(0 != m_mrdtmMonikerPtr.get());
        }

public:
    
    static BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr Create(const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         monikerPtr)
        {
        return new MyMSDocumentMoniker(monikerPtr);
        }
    };
    */
/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*struct LocalFileMonikerCreator : public BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerCreator
    {
private:
     
    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr         _Create                        (const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         msMoniker,
                                                                                                StatusInt&                          status) const override
        {
        status = BSISUCCESS;        
        return MyMSDocumentMoniker::Create(msMoniker);
        }

    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr        _Create                        (const WChar*                      fullPath,
                                                                                               StatusInt&                          status) const override
        {        
        DgnFileStatus openStatus = DGNFILE_STATUS_Success;
        BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocument> docPtr = DgnDocument::CreateFromFileName(openStatus, fullPath, 0, 0, DgnDocument::FetchMode::InfoOnly);

        if (DGNFILE_STATUS_Success != openStatus || 0 == docPtr.get())
            {
            status = BSIERROR;
            return 0;
            }

        status = BSISUCCESS;        
        return MyMSDocumentMoniker::Create(docPtr->GetMonikerPtr());        
        }
    };
*/
/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*class GeoDtmMSDocumentMoniker : public ILocalFileMoniker
    {
private:    

    BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr m_mrdtmMonikerPtr;

    virtual LocalFileURL                _GetURL                        (StatusInt&                          status) const override
        {
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));
        
        if (BSISUCCESS != status)
            fileName = m_mrdtmMonikerPtr->GetPortableName(); // File not found. Return file name with configuration variable.

        return LocalFileURL(fileName.c_str());
        }




    virtual DTMSourceMonikerType        _GetType                       () const override
        {
        return DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


    virtual bool                        _IsTargetReachable             () const override
        {
        StatusInt status;
        m_mrdtmMonikerPtr->ResolveFileName(&status);
        return BSISUCCESS == status;
        }

        virtual StatusInt                   _Serialize(Import::SourceDataSQLite&                      sourceData,
                                                       const DocumentEnv&                  env) const override
            {
            // TDORAY: Recreate the moniker using new env prior to serializing it in order so
            // that relative path is correct on file moves...

            const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
            sourceData.SetMonikerString(monikerString);

        return BSISUCCESS;
        }

    explicit                            GeoDtmMSDocumentMoniker        (const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         monikerPtr)
        :   m_mrdtmMonikerPtr(monikerPtr)
        {
        assert(0 != m_mrdtmMonikerPtr.get());
        }

    public:

        static ILocalFileMonikerPtr Create(const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         monikerPtr)
            {
            return new GeoDtmMSDocumentMoniker(monikerPtr);
            }
    };
    */

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*struct MonikerBinStreamCreator : public BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerBinStreamCreator
    {
private :
    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceMonikerType        _GetSupportedType              () const override
        {
        return BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerPtr                 _Create(Import::SourceDataSQLite&                      sourceData,
                                                                           const DocumentEnv&                  env,
                                                                           StatusInt&                          status) const override
            {
            WString monikerString = sourceData.GetMonikerString();

            const WChar* basePath = env.GetCurrentDirCStr();

            BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr documentMonikerPtr
                (
                    DgnDocumentMoniker::Create(monikerString.GetWCharCP(),
                                               basePath,
                                               false)
                    );

            if (documentMonikerPtr == 0)
                {
                status = BSIERROR;
                return 0;
                }

            status = BSISUCCESS;

            return GeoDtmMSDocumentMoniker::Create(documentMonikerPtr.get());
            }
    };
*/
/*
void InitScalableMeshMonikerFactories()
    {   
    static const struct MonikerBinStreamCreator s_MonikerBinStreamCreator;
    static const struct LocalFileMonikerCreator s_LocalFileMonikerCreator;
    const BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerFactory::CreatorID localFileCreatorID
        = BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerFactory::GetInstance().Register(s_LocalFileMonikerCreator);
    assert(localFileCreatorID == &s_LocalFileMonikerCreator);

    const BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerFactory::BinStreamCreatorID binStreamCreator
        = BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerFactory::GetInstance().Register(s_MonikerBinStreamCreator);
    assert(binStreamCreator == &s_MonikerBinStreamCreator);
    }
*/

//namespace {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*static const struct LocalFileMonikerCreator : public ILocalFileMonikerCreator
    {
private:
     
    virtual ILocalFileMonikerPtr         _Create                        (const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         msMoniker,
                                                                        StatusInt&                          status) const override
        {
        status = BSISUCCESS;        
        return GeoDtmMSDocumentMoniker::Create(msMoniker);
        }

    virtual ILocalFileMonikerPtr        _Create                        (const WChar*                      fullPath,
                                                                        StatusInt&                          status) const override
        {        
        DgnFileStatus openStatus = DGNFILE_STATUS_Success;
        DgnDocumentPtr docPtr = DgnDocument::CreateFromFileName(openStatus, fullPath, 0, 0, DgnDocument::FetchMode::InfoOnly);

        if (DGNFILE_STATUS_Success != openStatus || 0 == docPtr.get())
            {
            status = BSIERROR;
            return 0;
            }

        status = BSISUCCESS;        
        return GeoDtmMSDocumentMoniker::Create(docPtr->GetMonikerPtr());        
        }
    } s_LocalFileMonikerCreator;
*/

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*static const struct MonikerBinStreamCreator : public IMonikerBinStreamCreator
    {
private :
    virtual DTMSourceMonikerType        _GetSupportedType              () const override
        {
        return DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


        virtual IMonikerPtr                 _Create(Import::SourceDataSQLite&                      sourceData,
                                                    const DocumentEnv&                  env,
                                                    StatusInt&                          status) const override
            {
            WString monikerString = sourceData.GetMonikerString();

        const WChar* basePath = env.GetCurrentDirCStr();

        BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr documentMonikerPtr
            (            
            DgnDocumentMoniker::Create(monikerString.GetWCharCP(),
                                       basePath,                                                                  
                                       false)
            );

        if (documentMonikerPtr == 0)
            {
            status = BSIERROR;
            return 0;
            }

        status = BSISUCCESS;
        
        return GeoDtmMSDocumentMoniker::Create(documentMonikerPtr.get());
        }
    } s_MonikerBinStreamCreator;
*/
/*
void InitMonikerFactories()
    {   
    const ILocalFileMonikerFactory::CreatorID localFileCreatorID
        = ILocalFileMonikerFactory::GetInstance().Register(s_LocalFileMonikerCreator);
    assert(localFileCreatorID == &s_LocalFileMonikerCreator);

    const IMonikerFactory::BinStreamCreatorID binStreamCreator
        = IMonikerFactory::GetInstance().Register(s_MonikerBinStreamCreator);
    assert(binStreamCreator == &s_MonikerBinStreamCreator);
    }
}*/


//BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocument> docPtr = nullptr;
//BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnFile> file = nullptr;

void InitializeATP(DgnPlatformLib::Host& host)
        {
        static AppHost appHost;
        appHost.Startup ();

//        DependencyManager::SetTrackingDisabled(true);
//        DependencyManager::SetProcessingDisabled(true);
//        DgnFileStatus status;
        BeFileName name;
        BeFileNameStatus beStatus = BeFileName::BeGetTempPath(name);
        assert(BeFileNameStatus::Success == beStatus);
        name.AppendToPath(L"temp.dgn");
        
//        docPtr = DgnDocument::CreateForNewFile(status, name.GetName(), NULL, DEFDGNFILE_ID, NULL, DgnDocument::OverwriteMode::Always, DgnDocument::CreateOptions::SupressFailureNotification);
//        file = DgnFile::Create(*docPtr, DgnFileOpenMode::ReadWrite);
//        DgnModelStatus createStatus;
//        file->SetScratchFileFlag(true);
//        DgnModel* model = file->CreateNewModel(&createStatus, L"Model",DgnModelType::Normal, true);
        
//        RscFileManager::StaticInitialize(L"not-used");
        
        static ExeHost smHost;
//        ((ExeAdmin&)smHost.GetScalableMeshAdmin()).SetActiveModelRef(model);
        
        //InitMonikerFactories();
        //InitScalableMeshMonikerFactories();
//        BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::Initialize(*new AppRasterCoreLibHost());
//        BeAssert(BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::IsInitialized()); 
        }

void CloseATP()
    {
//    WString dgnFileName(file->GetFileName ());

//    file = nullptr;    
//    docPtr = nullptr;
//    _wremove(dgnFileName.c_str());
    }
    }

