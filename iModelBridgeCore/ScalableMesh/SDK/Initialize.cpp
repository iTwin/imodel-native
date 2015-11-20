

#include <windows.h>
#include "Initialize.h"

using namespace std;
#include <DgnPlatform\IAuxCoordSys.h>
#include <DgnPlatform/DelegatedElementECEnabler.h>
#include <DgnPlatform\ITransactionHandler.h>
#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\PointCloudHandler.h>
#include <DgnGeoCoord\DgnGeoCoord.h>
#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudDisplayHandler.h>
#include <RasterCore\RasterCoreLib.h>
#include <RmgrTools\Tools\RscFileManager.h>
#include <TerrainModel\TerrainModel.h>
#include <ScalableMesh\ScalableMeshDefs.h>
#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh\IScalableMeshStream.h>
#include <ScalableMesh\IScalableMeshURL.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnHost.h>
#include <DgnView/DgnViewAPI.h>
#include <ScalableMesh\ScalableMeshAdmin.h>
#include <ScalableMesh\ScalableMeshLib.h>
#include <RmgrTools/Tools/RscFileManager.h>
#include <ScalableTerrainModel\IMrDTMMoniker.h>
#include <ScalableTerrainModel\IMrDTMStream.h>
#include <ScalableTerrainModel\IMrDTMURL.h>


//USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_MRDTM
using namespace Bentley::DgnPlatform;
using namespace Bentley::GeoCoordinates;

namespace ScalableMeshSDKexe
    {
    struct AppViewManager : Bentley::DgnPlatform::IViewManager
        {
        private:
            Bentley::DgnPlatform::IndexedViewSet* m_activeViewSet;
            HWND                                  m_topWindow;
    
            virtual Bentley::DgnPlatform::DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override {return (Bentley::DgnPlatform::DgnDisplayCoreTypes::WindowP)m_topWindow;}
            virtual bool                                               _DoesHostHaveFocus() {return false;}
            virtual Bentley::DgnPlatform::IndexedViewSet&              _GetActiveViewSet() override {assert(!"Not expect to be call in offline mode"); return *m_activeViewSet;}
            virtual int             _GetCurrentViewNumber() override {return 0;}
            virtual HUDManagerP     _GetHUDManager () {return NULL;}

        public:
    
            //void SetActiveDgn (AppViewSet* newDgn) {m_activeViewSet = newDgn;}
            //void SetActiveDgn (DemoViewSet* newDgn) {m_activeViewSet = newDgn;}
            AppViewManager() {m_activeViewSet = NULL; m_topWindow = NULL;}
        };

        //=======================================================================================
        // @bsiclass                                                    Keith.Bentley   01/10
        //=======================================================================================
        class AppHost : public Bentley::DgnPlatform::DgnViewLib::Host
        {
            AppViewManager   m_viewManager;

        protected:

            virtual Bentley::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin&  _SupplyNotificationAdmin() override;        
            virtual void                                                            _SupplyProductName(WStringR name) override;     
            virtual Bentley::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager& _SupplyDigitalRightsManager() override;     
            //virtual GraphicsAdmin&                                                 _SupplyGraphicsAdmin() override;            
            //virtual ViewStateAdmin&                                                _SupplyViewStateAdmin() override;           
            //virtual ToolAdmin&                                                     _SupplyToolAdmin() override;                
            virtual Bentley::DgnPlatform::IViewManager&                             _SupplyViewManager() override;              
            //virtual SolidsKernelAdmin&                                             _SupplySolidsKernelAdmin() override;        
            virtual Bentley::DgnPlatform::DgnPlatformLib::Host::RasterAttachmentAdmin&      _SupplyRasterAttachmentAdmin() override;    
            virtual Bentley::DgnPlatform::DgnPlatformLib::Host::PointCloudAdmin&            _SupplyPointCloudAdmin() override;          
            //virtual FontAdmin&                                                     _SupplyFontAdmin() override;                
            //virtual MaterialAdmin&                                                 _SupplyMaterialAdmin();                     
            //virtual ProgressiveDisplayManager&                                     _SupplyProgressiveDisplayManager() override;
            virtual Bentley::DgnPlatform::DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() override;

        public:
            void Startup (/*HWND*/);
        
            void Terminate ();

            //DemoViewManager& GetDemoViewManager(){return m_viewManager;}
        };

        //=======================================================================================
        // @bsiclass                                                    Keith.Bentley   01/10
        //=======================================================================================
        struct AppNotificationAdmin : Bentley::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin
        {
            virtual StatusInt _OutputMessage (Bentley::DgnPlatform::NotifyMessageDetails const& msg) override;
            virtual void      _OutputPrompt (WCharCP) override;
            virtual Bentley::DgnPlatform::NotificationManager::MessageBoxValue _OpenMessageBox (Bentley::DgnPlatform::NotificationManager::MessageBoxType, WCharCP, Bentley::DgnPlatform::NotificationManager::MessageBoxIconType) override;
        };

        /*=================================================================================**//**
        * DgnViewDemo only displays the contents of a file on the screen. It does not EXPORT
        * engineering data. Therefore, it is safe for DgnViewDemo to open rights-restricted DGN files.
        * @bsiclass                                     Sam.Wilson                      06/2010
        +===============+===============+===============+===============+===============+======*/
        struct ReadOnlyDigitalRightsManager : Bentley::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager
        {
        //    virtual StatusInt _OnEnterRestrictedMode (bool assertKeys, Bentley::DgnFileProtection::KeyMaterialWithDescription* keylist, uint32_t nkeys, Bentley::DgnPlatform::DgnFileP file, uint32_t rights) override {return SUCCESS;}
        };



/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AppRasterCoreAdmin : Bentley::DgnPlatform::Raster::RasterCoreAdmin
{
virtual bool                    _IsProgressiveDisplayEnabled() const    {return true;}
};

/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     02/2011
+===============+===============+===============+===============+===============+======*/
struct AppRasterCoreLibHost : Bentley::DgnPlatform::Raster::RasterCoreLib::Host 
{
virtual Bentley::DgnPlatform::Raster::RasterCoreAdmin& _SupplyRasterCoreAdmin() override {return *new AppRasterCoreAdmin();}
}; // RasterCoreLib::Host

void AppHost::Startup (/*HWND*/)
    {       
    //InitMonikerFactories();

    //IScalableMeshAdmin::SetCanImportPODFile(true);    

    //Init GDAL path.    
    WString gdalDataPath(L".\\Gdal_Data\\");        
    ConfigurationManager::DefineVariable (L"_USTN_RASTERGDALDATA", gdalDataPath.c_str());

    RscFileManager::StaticInitialize(L"en");
        
    DgnViewLib::Initialize (*this, true);

    //Application needs to initialize PdfLibInitializer dll if it wants support for PDF raster attachment.
    //Bentley::PdfLibInitializer::Initialize(*new ViewDemoPdfLibInitializerHost());

    Bentley::DgnPlatform::Raster::RasterCoreLib::Initialize (*new AppRasterCoreLibHost());
    BeAssert (Bentley::DgnPlatform::Raster::RasterCoreLib::IsInitialized());

    //Ensure basegeocoord is initialized.
    _SupplyGeoCoordinationAdmin()._GetServices();
    
    //Required for reading TM element
    //Bentley::TerrainModel::Element::DTMElementHandlerManager::InitializeForOfflineTmImport();

    //m_viewManager.SetTopWindow(topWindow);

    // Setup location to find parasolid schema files (schema directory was created under exe location)...
    /*
    WString baseDir;
    getBaseDirOfExecutingModule (baseDir);
    
    ConfigurationManager::DefineVariable (L"MS_SMARTSOLID", baseDir.c_str ());
    */
    }

void AppHost::Terminate ()
    {
    //call rasterCore cleanup code
    Bentley::DgnPlatform::Raster::RasterCoreLib::GetHost().Terminate(true);

    //call PdfLibInitializer dll cleanup code.
    //Bentley::PdfLibInitializer::GetHost().Terminate(true);
    }

void                                                                   AppHost::_SupplyProductName(WStringR name)   {name.assign(L"DgnView Demo");}
Bentley::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin&         AppHost::_SupplyNotificationAdmin()          {return *new AppNotificationAdmin();}
Bentley::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager&        AppHost::_SupplyDigitalRightsManager()       {return *new ReadOnlyDigitalRightsManager;}
//GraphicsAdmin&             AppHost::_SupplyGraphicsAdmin()              {return *new DgnViewGraphicsAdmin();}
//ViewStateAdmin&            AppHost::_SupplyViewStateAdmin()             {return *new DemoViewStateAdmin();}
//ToolAdmin&                 AppHost::_SupplyToolAdmin()                  {return *new DemoToolAdmin();}
Bentley::DgnPlatform::IViewManager&                                    AppHost::_SupplyViewManager()                {return m_viewManager; }
//SolidsKernelAdmin&         AppHost::_SupplySolidsKernelAdmin()          {return *new AppSolidKernelAdmin();}
Bentley::DgnPlatform::DgnPlatformLib::Host::RasterAttachmentAdmin&     AppHost::_SupplyRasterAttachmentAdmin()      {return Bentley::DgnPlatform::Raster::RasterCoreLib::GetDefaultRasterAttachmentAdmin();}
Bentley::DgnPlatform::DgnPlatformLib::Host::PointCloudAdmin&           AppHost::_SupplyPointCloudAdmin()            {return *new Bentley::DgnPlatform::PointCloudDisplayAdmin();}
//FontAdmin&                 AppHost::_SupplyFontAdmin()                  {return *new DemoFontAdmin();}
//MaterialAdmin&             AppHost::_SupplyMaterialAdmin()              {return *new DemoMaterialAdmin(); }
//ProgressiveDisplayManager& AppHost::_SupplyProgressiveDisplayManager()  {return *new DemoProgressiveDisplayManager(); }
Bentley::DgnPlatform::DgnPlatformLib::Host::GeoCoordinationAdmin&      AppHost::_SupplyGeoCoordinationAdmin()       
    {        
    
    WString geocoordinateDataPath(L".\\GeoCoordinateData\\");    

    return *DgnGeoCoordinationAdmin::Create (geocoordinateDataPath.c_str(), IACSManager::GetManager()); 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+-****/
StatusInt AppNotificationAdmin::_OutputMessage (NotifyMessageDetails const& msg)
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      AppNotificationAdmin::_OutputPrompt (WCharCP prompt)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
NotificationManager::MessageBoxValue AppNotificationAdmin::_OpenMessageBox (NotificationManager::MessageBoxType, WCharCP message, NotificationManager::MessageBoxIconType iconType)
    {
    return NotificationManager::MESSAGEBOX_VALUE_Ok;
    }



  struct ExeAdmin : Bentley::ScalableMesh::ScalableMeshAdmin
        {
        DgnModelRef* s_activeDgnModelRefP;
        ExeAdmin() :s_activeDgnModelRefP(0){};

        ~ExeAdmin(){};

        bool _CanImportPODfile() const
            {
            return true;
            }

        void SetActiveModelRef(DgnModelRef* activeDgnModelRefP)
            {
            s_activeDgnModelRefP = activeDgnModelRefP;
            }

        DgnModelRef* _GetActiveModelRef() const
            {
            return s_activeDgnModelRefP;
            }

        };

    struct ExeHost : Bentley::ScalableMesh::ScalableMeshLib::Host
        {

        ExeHost()
            {
            Bentley::ScalableMesh::ScalableMeshLib::Initialize(*this);
            }
    Bentley::ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        return *new ExeAdmin();
        };
        };


    
    
/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class MyMSDocumentMoniker : public Bentley::ScalableMesh::ILocalFileMoniker
    {
private:    

    Bentley::RefCountedPtr<DgnDocumentMoniker> m_mrdtmMonikerPtr;

    virtual Bentley::ScalableMesh::LocalFileURL                _GetURL                        (StatusInt&                          status) const override
        {
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));
        
        if (BSISUCCESS != status)
            fileName = m_mrdtmMonikerPtr->GetPortableName(); // File not found. Return s_dgnFile name with configuration variable.

        return Bentley::ScalableMesh::LocalFileURL(fileName.c_str());
        }




    virtual Bentley::ScalableMesh::DTMSourceMonikerType        _GetType                       () const override
        {
        return Bentley::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


    virtual bool                        _IsTargetReachable             () const override
        {
        StatusInt status;
        m_mrdtmMonikerPtr->ResolveFileName(&status);
        return BSISUCCESS == status;
        }

    virtual StatusInt                   _Serialize                     (Bentley::ScalableMesh::BinaryOStream&                      stream,
                                                                        const Bentley::ScalableMesh::DocumentEnv&                  env) const override
        {
        // TDORAY: Recreate the moniker using new env prior to serializing it in order so
        // that relative path is correct on s_dgnFile moves...

        const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
        if (!WriteStringW(stream, monikerString.c_str()))
            return BSIERROR;

        return BSISUCCESS;
        }

    explicit                            MyMSDocumentMoniker        (const DgnDocumentMonikerPtr&         monikerPtr)
        :   m_mrdtmMonikerPtr(monikerPtr)
        {
        assert(0 != m_mrdtmMonikerPtr.get());
        }

public:
    
    static Bentley::ScalableMesh::ILocalFileMonikerPtr Create(const DgnDocumentMonikerPtr&         monikerPtr)
        {
        return new MyMSDocumentMoniker(monikerPtr);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileMonikerCreator : public Bentley::ScalableMesh::ILocalFileMonikerCreator
    {
private:
     
    virtual Bentley::ScalableMesh::ILocalFileMonikerPtr         _Create                        (const DgnDocumentMonikerPtr&         msMoniker,
                                                                                                StatusInt&                          status) const override
        {
        status = BSISUCCESS;        
        return MyMSDocumentMoniker::Create(msMoniker);
        }

    virtual Bentley::ScalableMesh::ILocalFileMonikerPtr        _Create                        (const WChar*                      fullPath,
                                                                                               StatusInt&                          status) const override
        {        
        DgnFileStatus openStatus = DGNFILE_STATUS_Success;
        Bentley::RefCountedPtr<DgnDocument> docPtr = DgnDocument::CreateFromFileName(openStatus, fullPath, 0, 0, DgnDocument::FetchMode::InfoOnly);

        if (DGNFILE_STATUS_Success != openStatus || 0 == docPtr.get())
            {
            status = BSIERROR;
            return 0;
            }

        status = BSISUCCESS;        
        return MyMSDocumentMoniker::Create(docPtr->GetMonikerPtr());        
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct MonikerBinStreamCreator : public Bentley::ScalableMesh::IMonikerBinStreamCreator
    {
private :
    virtual Bentley::ScalableMesh::DTMSourceMonikerType        _GetSupportedType              () const override
        {
        return Bentley::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


    virtual Bentley::ScalableMesh::IMonikerPtr                 _Create                        (Bentley::ScalableMesh::BinaryIStream&                      stream,
                                                                                               const Bentley::ScalableMesh::DocumentEnv&                  env,
                                                                                               StatusInt&                          status) const override
        {
        WString monikerString;
        if (!ReadStringW(stream, monikerString))
            {
            status = BSIERROR;
            return 0;
            }

        const WChar* basePath = env.GetCurrentDirCStr();

        Bentley::RefCountedPtr<DgnDocumentMoniker> documentMonikerPtr
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
        
        return MyMSDocumentMoniker::Create(documentMonikerPtr.get());
        }
    };


void InitScalableMeshMonikerFactories()
    {   
    static const struct MonikerBinStreamCreator s_MonikerBinStreamCreator;
    static const struct LocalFileMonikerCreator s_LocalFileMonikerCreator;
    const Bentley::ScalableMesh::ILocalFileMonikerFactory::CreatorID localFileCreatorID
        = Bentley::ScalableMesh::ILocalFileMonikerFactory::GetInstance().Register(s_LocalFileMonikerCreator);
    assert(localFileCreatorID == &s_LocalFileMonikerCreator);

    const Bentley::ScalableMesh::IMonikerFactory::BinStreamCreatorID binStreamCreator
        = Bentley::ScalableMesh::IMonikerFactory::GetInstance().Register(s_MonikerBinStreamCreator);
    assert(binStreamCreator == &s_MonikerBinStreamCreator);
    }


namespace {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class GeoDtmMSDocumentMoniker : public ILocalFileMoniker
    {
private:    

    DgnDocumentMonikerPtr m_mrdtmMonikerPtr;

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

    virtual StatusInt                   _Serialize                     (BinaryOStream&                      stream,
                                                                        const DocumentEnv&                  env) const override
        {
        // TDORAY: Recreate the moniker using new env prior to serializing it in order so
        // that relative path is correct on file moves...

        const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
        if (!WriteStringW(stream, monikerString.c_str()))
            return BSIERROR;

        return BSISUCCESS;
        }

    explicit                            GeoDtmMSDocumentMoniker        (const DgnDocumentMonikerPtr&         monikerPtr)
        :   m_mrdtmMonikerPtr(monikerPtr)
        {
        assert(0 != m_mrdtmMonikerPtr.get());
        }

public:
    
    static ILocalFileMonikerPtr Create(const DgnDocumentMonikerPtr&         monikerPtr)
        {
        return new GeoDtmMSDocumentMoniker(monikerPtr);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static const struct LocalFileMonikerCreator : public ILocalFileMonikerCreator
    {
private:
     
    virtual ILocalFileMonikerPtr         _Create                        (const DgnDocumentMonikerPtr&         msMoniker,
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


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static const struct MonikerBinStreamCreator : public IMonikerBinStreamCreator
    {
private :
    virtual DTMSourceMonikerType        _GetSupportedType              () const override
        {
        return DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


    virtual IMonikerPtr                 _Create                        (BinaryIStream&                      stream,
                                                                        const DocumentEnv&                  env,
                                                                        StatusInt&                          status) const override
        {
        WString monikerString;
        if (!ReadStringW(stream, monikerString))
            {
            status = BSIERROR;
            return 0;
            }

        const WChar* basePath = env.GetCurrentDirCStr();

        DgnDocumentMonikerPtr documentMonikerPtr
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


void InitMonikerFactories()
    {   
    const ILocalFileMonikerFactory::CreatorID localFileCreatorID
        = ILocalFileMonikerFactory::GetInstance().Register(s_LocalFileMonikerCreator);
    assert(localFileCreatorID == &s_LocalFileMonikerCreator);

    const IMonikerFactory::BinStreamCreatorID binStreamCreator
        = IMonikerFactory::GetInstance().Register(s_MonikerBinStreamCreator);
    assert(binStreamCreator == &s_MonikerBinStreamCreator);
    }
}


Bentley::RefCountedPtr<DgnDocument> docPtr = nullptr;
Bentley::RefCountedPtr<DgnFile> file = nullptr;

void InitializeSDK(DgnPlatformLib::Host& host)
        {
        static AppHost appHost;
        appHost.Startup ();

        DependencyManager::SetTrackingDisabled(true);
        DependencyManager::SetProcessingDisabled(true);
        DgnFileStatus status;
        BeFileName name;
        assert(BeFileNameStatus::Success == BeFileName::BeGetTempPath(name));
        name.AppendToPath(L"temp.dgn");
        
        docPtr = DgnDocument::CreateForNewFile(status, name.GetName(), NULL, DEFDGNFILE_ID, NULL, DgnDocument::OverwriteMode::Always, DgnDocument::CreateOptions::SupressFailureNotification);
        file = DgnFile::Create(*docPtr, DgnFileOpenMode::ReadWrite);
        DgnModelStatus createStatus;
        file->SetScratchFileFlag(true);
        DgnModel* model = file->CreateNewModel(&createStatus, L"Model",DgnModelType::Normal, true);
        
        RscFileManager::StaticInitialize(L"not-used");
        
        static ExeHost smHost;
        ((ExeAdmin&)smHost.GetScalableMeshAdmin()).SetActiveModelRef(model);
        
        InitMonikerFactories();
        InitScalableMeshMonikerFactories();
        Bentley::DgnPlatform::Raster::RasterCoreLib::Initialize(*new AppRasterCoreLibHost());
        BeAssert(Bentley::DgnPlatform::Raster::RasterCoreLib::IsInitialized()); 
        }

void CloseSDK()
    {
    WString dgnFileName(file->GetFileName ());

    file = nullptr;    
    docPtr = nullptr;
    _wremove(dgnFileName.c_str());
    }
    }

