

#include <windows.h>
#include "Initialize.h"

using namespace std;
#include <Bentley\BeFileListIterator.h>
#include <DgnPlatform\IAuxCoordSys.h>
#include <DgnPlatform/DelegatedElementECEnabler.h>
#include <DgnPlatform\ITransactionHandler.h>
#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\PointCloudHandler.h>
#include <DgnGeoCoord\DgnGeoCoord.h>
#include <GeoCoord\GCSLibrary.h>
#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudDisplayHandler.h>
#include <RasterCore\RasterCoreLib.h>
#include <RmgrTools\Tools\RscFileManager.h>
#undef LAT
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
using namespace BENTLEY_NAMESPACE_NAME::DgnPlatform;
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

namespace ScalableMeshSDKexe
    {
    struct AppViewManager : BENTLEY_NAMESPACE_NAME::DgnPlatform::IViewManager
        {
        private:
            BENTLEY_NAMESPACE_NAME::DgnPlatform::IndexedViewSet* m_activeViewSet;
            HWND                                  m_topWindow;
    
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnDisplayCoreTypes::WindowP _GetTopWindow(int) override {return (BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnDisplayCoreTypes::WindowP)m_topWindow;}
            virtual bool                                               _DoesHostHaveFocus() {return false;}
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::IndexedViewSet&              _GetActiveViewSet() override {assert(!"Not expect to be call in offline mode"); return *m_activeViewSet;}
            virtual int             _GetCurrentViewNumber() override {return 0;}
            virtual HUDManagerP     _GetHUDManager () {return NULL;}

        public:

            AppViewManager() {m_activeViewSet = NULL; m_topWindow = NULL;}
        };

        //=======================================================================================
        // @bsiclass                                                    Keith.Bentley   01/10
        //=======================================================================================
        class AppHost : public BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnViewLib::Host
        {
            AppViewManager m_viewManager;
            BeFileName     m_systemDtyPath;
            BeFileName     m_customDtyPath;

        protected:

            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin&  _SupplyNotificationAdmin() override;
            virtual void                                                            _SupplyProductName(WStringR name) override;
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager& _SupplyDigitalRightsManager() override;
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::IViewManager&                             _SupplyViewManager() override;
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::RasterAttachmentAdmin&      _SupplyRasterAttachmentAdmin() override;
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::PointCloudAdmin&            _SupplyPointCloudAdmin() override;
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::GeoCoordinationAdmin& _SupplyGeoCoordinationAdmin() override;

        public:
            void Startup (BeFileName& systemDtyPath, BeFileName& customDtyPath);
        
            void Terminate ();
        };

        //=======================================================================================
        // @bsiclass                                                    Keith.Bentley   01/10
        //=======================================================================================
        struct AppNotificationAdmin : BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin
        {
            virtual StatusInt _OutputMessage (BENTLEY_NAMESPACE_NAME::DgnPlatform::NotifyMessageDetails const& msg) override;
            virtual void      _OutputPrompt (WCharCP) override;
            virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::NotificationManager::MessageBoxValue _OpenMessageBox (BENTLEY_NAMESPACE_NAME::DgnPlatform::NotificationManager::MessageBoxType, WCharCP, BENTLEY_NAMESPACE_NAME::DgnPlatform::NotificationManager::MessageBoxIconType) override;
        };

        /*=================================================================================**//**
        * DgnViewDemo only displays the contents of a file on the screen. It does not EXPORT
        * engineering data. Therefore, it is safe for DgnViewDemo to open rights-restricted DGN files.
        * @bsiclass                                     Sam.Wilson                      06/2010
        +===============+===============+===============+===============+===============+======*/
        struct ReadOnlyDigitalRightsManager : BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager
        {
        //    virtual StatusInt _OnEnterRestrictedMode (bool assertKeys, BENTLEY_NAMESPACE_NAME::DgnFileProtection::KeyMaterialWithDescription* keylist, uint32_t nkeys, BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileP file, uint32_t rights) override {return SUCCESS;}
        };



/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AppRasterCoreAdmin : BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreAdmin
{
virtual bool                    _IsProgressiveDisplayEnabled() const    {return true;}
};

/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     02/2011
+===============+===============+===============+===============+===============+======*/
struct AppRasterCoreLibHost : BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::Host 
{
virtual BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreAdmin& _SupplyRasterCoreAdmin() override {return *new AppRasterCoreAdmin();}
}; // RasterCoreLib::Host

void AppHost::Startup (BeFileName& systemDtyPath, BeFileName& customDtyPath)
    {           
    m_systemDtyPath = systemDtyPath;
    m_customDtyPath = customDtyPath;

    //Init GDAL path.    
    WString gdalDataPath(L".\\Gdal_Data\\");        
    ConfigurationManager::DefineVariable (L"_USTN_RASTERGDALDATA", gdalDataPath.c_str());

    RscFileManager::StaticInitialize(L"en");
        
    DgnViewLib::Initialize (*this, true);

    //Application needs to initialize PdfLibInitializer dll if it wants support for PDF raster attachment.

    BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::Initialize (*new AppRasterCoreLibHost());
    BeAssert (BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::IsInitialized());

    //Ensure basegeocoord is initialized.
    _SupplyGeoCoordinationAdmin()._GetServices();

    if (BeFileName::DoesPathExist(m_customDtyPath.c_str()))
        {
        BeFileName customDtySearchPath(m_customDtyPath);
        customDtySearchPath.AppendToPath(L"*.dty");

        BeFileListIterator iterDir(customDtySearchPath, false);
        BeFileName currentFile;
        while (BSISUCCESS == iterDir.GetNextFileName(currentFile))
            {
            GeoCoordinates::LibraryManager::Instance()->AddUserLibrary(currentFile.GetName(), nullptr);
            }        
        }   
    }

void AppHost::Terminate ()
    {
    //call rasterCore cleanup code
    BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::GetHost().Terminate(true);
    }

void                                                                   AppHost::_SupplyProductName(WStringR name)   {name.assign(L"DgnView Demo");}
BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::NotificationAdmin&         AppHost::_SupplyNotificationAdmin()          {return *new AppNotificationAdmin();}
BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnFileIOLib::Host::DigitalRightsManager&        AppHost::_SupplyDigitalRightsManager()       {return *new ReadOnlyDigitalRightsManager;}
BENTLEY_NAMESPACE_NAME::DgnPlatform::IViewManager&                                    AppHost::_SupplyViewManager()                {return m_viewManager; }
BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::RasterAttachmentAdmin&     AppHost::_SupplyRasterAttachmentAdmin()      {return BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::GetDefaultRasterAttachmentAdmin();}
BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::PointCloudAdmin&           AppHost::_SupplyPointCloudAdmin()            {return *new BENTLEY_NAMESPACE_NAME::DgnPlatform::PointCloudDisplayAdmin();}
BENTLEY_NAMESPACE_NAME::DgnPlatform::DgnPlatformLib::Host::GeoCoordinationAdmin&      AppHost::_SupplyGeoCoordinationAdmin()
    {        
    WString geocoordinateDataPath;

    if (BeFileName::DoesPathExist(m_systemDtyPath.c_str()))
        {
        geocoordinateDataPath = WString(m_systemDtyPath.c_str());
        }
    else
        {
        geocoordinateDataPath = WString(L".\\GeoCoordinateData\\");
        }
       
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



  struct ExeAdmin : BENTLEY_NAMESPACE_NAME::ScalableMesh::ScalableMeshAdmin
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
class MyMSDocumentMoniker : public BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMoniker
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

        //const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
        //if (!WriteStringW(stream, monikerString.c_str()))
        //    return BSIERROR;

        return BSISUCCESS;
        }

    explicit                            MyMSDocumentMoniker        (const DgnDocumentMonikerPtr&         monikerPtr)
        :   m_mrdtmMonikerPtr(monikerPtr)
        {
        assert(0 != m_mrdtmMonikerPtr.get());
        }

public:
    
    static BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr Create(const DgnDocumentMonikerPtr&         monikerPtr)
        {
        return new MyMSDocumentMoniker(monikerPtr);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileMonikerCreator : public BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerCreator
    {
private:
     
    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::ILocalFileMonikerPtr         _Create                        (const DgnDocumentMonikerPtr&         msMoniker,
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


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct MonikerBinStreamCreator : public BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerBinStreamCreator
    {
private :
    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::DTMSourceMonikerType        _GetSupportedType              () const override
        {
        return BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
        }


    virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerPtr                 _Create                        (BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::SourceDataSQLite&                      sourceData,
                                                                                               const BENTLEY_NAMESPACE_NAME::ScalableMesh::DocumentEnv&                  env,
                                                                                               StatusInt&                          status) const override
        {
        WString monikerString = sourceData.GetMonikerString();
        /*if (!ReadStringW(stream, monikerString))
            {
            status = BSIERROR;
            return 0;
            }*/

        const WChar* basePath = env.GetCurrentDirCStr();

        BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocumentMoniker> documentMonikerPtr
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
        
    /*        virtual BENTLEY_NAMESPACE_NAME::ScalableMesh::IMonikerPtr                 _Create                        (BinaryIStream&                      stream,
                                                                                               const BENTLEY_NAMESPACE_NAME::ScalableMesh::DocumentEnv&                  env,
                                                                                               StatusInt&                          status) const override
        {
        WString monikerString;
        if (!ReadStringW(stream, monikerString))
            {
            status = BSIERROR;
            return 0;
            }

        const WChar* basePath = env.GetCurrentDirCStr();

        BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocumentMoniker> documentMonikerPtr
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
        }*/
    };


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

  /*  virtual StatusInt                   _Serialize                     (ScalableMesh::Import::SourceDataSQLite&                      sourceData,//BinaryOStream&                      stream,
                                                                        const DocumentEnv&                  env) const override
        {
        // TDORAY: Recreate the moniker using new env prior to serializing it in order so
        // that relative path is correct on file moves...

       // const WString& monikerString(m_mrdtmMonikerPtr->Externalize());
       // if (!WriteStringW(stream, monikerString.c_str()))
       //     return BSIERROR;

        return BSISUCCESS;
        }*/
        
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


  /*  virtual IMonikerPtr                 _Create                        (ScalableMesh::Import::SourceDataSQLite&                      sourceData,
                                                                        const DocumentEnv&                  env,
                                                                        StatusInt&                          status) const override
        {
       WString monikerString = sourceData.GetMonikerString();

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
        }*/
        
        
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


BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnDocument> docPtr = nullptr;
BENTLEY_NAMESPACE_NAME::RefCountedPtr<DgnFile> file = nullptr;

void InitializeSDK(BeFileName& systemDtyPath, BeFileName& customDtyPath)
        {
        static AppHost appHost;
        appHost.Startup (systemDtyPath, customDtyPath);

        DependencyManager::SetTrackingDisabled(true);
        DependencyManager::SetProcessingDisabled(true);
        DgnFileStatus status;
        BeFileName name;
        BeFileNameStatus beStatus = BeFileName::BeGetTempPath(name);
        assert(BeFileNameStatus::Success == beStatus);
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
        BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::Initialize(*new AppRasterCoreLibHost());
        BeAssert(BENTLEY_NAMESPACE_NAME::DgnPlatform::Raster::RasterCoreLib::IsInitialized()); 
        }

void CloseSDK()
    {
    WString dgnFileName(file->GetFileName ());

    file = nullptr;    
    docPtr = nullptr;
    _wremove(dgnFileName.c_str());
    }
    }

