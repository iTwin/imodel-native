// StandAloneTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ios>
#include <list>

using namespace std;

#include "AppHost.h"
#include <DgnPlatform\IAuxCoordSys.h>
#include <DgnGeoCoord\DgnGeoCoord.h>
#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudDisplayHandler.h>
#include <RasterCore\RasterCoreLib.h>
#include <RmgrTools\Tools\RscFileManager.h>
#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh\IScalableMeshStream.h>
#include <ScalableMesh\IScalableMeshURL.h>
#include <TerrainModel\ElementHandler\DTMElementHandlerManager.h>
#include <TerrainModel\ElementHandler\IMrDTMHostAdmin.h>
#include <TerrainModel\ElementHandler\TerrainModelElementHandler.h>




USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT 
USING_NAMESPACE_RASTER
using namespace Bentley::GeoCoordinates;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class MyMSDocumentMoniker : public ILocalFileMoniker
    {
private:    

    DgnDocumentMonikerPtr m_mrdtmMonikerPtr;

    virtual LocalFileURL                _GetURL                        (StatusInt&                          status) const override
        {
        WString fileName(m_mrdtmMonikerPtr->ResolveFileName(&status));
        
        if (BSISUCCESS != status)
            fileName = m_mrdtmMonikerPtr->GetPortableName(); // File not found. Return file name with configuration variable.

        return Bentley::ScalableMesh::LocalFileURL(fileName.c_str());
        }




    virtual DTMSourceMonikerType        _GetType                       () const override
        {
        return Bentley::ScalableMesh::DTM_SOURCE_MONIKER_MSDOCUMENT;
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

    explicit                            MyMSDocumentMoniker        (const DgnDocumentMonikerPtr&         monikerPtr)
        :   m_mrdtmMonikerPtr(monikerPtr)
        {
        assert(0 != m_mrdtmMonikerPtr.get());
        }

public:
    
    static ILocalFileMonikerPtr Create(const DgnDocumentMonikerPtr&         monikerPtr)
        {
        return new MyMSDocumentMoniker(monikerPtr);
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
        return MyMSDocumentMoniker::Create(msMoniker);
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
        return MyMSDocumentMoniker::Create(docPtr->GetMonikerPtr());        
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
        
        return MyMSDocumentMoniker::Create(documentMonikerPtr.get());
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
    InitMonikerFactories();

    IMrDTMAdmin::SetCanImportPODFile(true);    

    //Init GDAL path.
    wchar_t* outRoot = _wgetenv(L"OutRoot");
    assert(outRoot != 0);
    WString gdalDataPath(outRoot);
    gdalDataPath +=  WString(L"Winx64\\Product\\TerrainDataImporterModule\\Gdal_Data\\");
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
    Bentley::TerrainModel::Element::DTMElementHandlerManager::InitializeForOfflineTmImport();

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
    wchar_t* outRoot = _wgetenv(L"OutRoot");
    assert(outRoot != 0);
    WString geocoordinateDataPath(outRoot);
    geocoordinateDataPath +=  WString(L"Winx64\\Product\\TerrainDataImporterModule\\GeoCoordinateData\\");

    return *DgnGeoCoordinationAdmin::Create (geocoordinateDataPath.c_str(), IACSManager::GetManager()); 
    }





/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/09
+---------------+---------------+---------------+---------------+---------------+-

-----*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*
BentleyStatus   AppSolidKernelAdmin::_RestoreEntityFromMemory
(
ISolidKernelEntityPtr&              entityOut,
void const*                         pBuffer, 
unsigned int                        bufferSize,
ISolidKernelEntity::SolidKernelType kernelType,
TransformCR                         transform
) const
    {
    if (ISolidKernelEntity::SolidKernel_PSolid == kernelType)
        return __super::_RestoreEntityFromMemory (entityOut, pBuffer, bufferSize, kernelType, transform);

    int         entityTag;
    Transform   entityTransform;

    PSolidKernelManager::StartSession (); // NOTE: Make sure parasolid is started...

    if (SUCCESS != PSolidAcisInterop::SATEntityToXMTEntity (entityTag, entityTransform, pBuffer, bufferSize, transform))
        return ERROR;

    entityOut = PSolidKernelManager::CreateEntityPtr (entityTag, entityTransform);

    return SUCCESS;
    }
    */