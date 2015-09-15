

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
#include <ScalableMesh\IScalableMeshMoniker.h>
#include <ScalableMesh\IScalableMeshStream.h>
#include <ScalableMesh\IScalableMeshURL.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnHost.h>
#include <DgnView/DgnViewAPI.h>
#include <ScalableMesh\ScalableMeshAdmin.h>
#include <ScalableMesh\ScalableMeshLib.h>
#include <RmgrTools/Tools/RscFileManager.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH

namespace ScalableMeshSDKSample
    {
  struct SampleAdmin : Bentley::ScalableMesh::ScalableMeshAdmin
        {
        DgnModelRef* s_activeDgnModelRefP;
        SampleAdmin() :s_activeDgnModelRefP(0){};

        ~SampleAdmin(){};

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

    struct SampleHost : Bentley::ScalableMesh::ScalableMeshLib::Host
        {

        SampleHost()
            {
            Bentley::ScalableMesh::ScalableMeshLib::Initialize(*this);
            }
    Bentley::ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        return *new SampleAdmin();
        };
        };

    /*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class MyMSDocumentMoniker : public ILocalFileMoniker
    {
private:    

    Bentley::RefCountedPtr<DgnDocumentMoniker> m_mrdtmMonikerPtr;

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
struct LocalFileMonikerCreator : public ILocalFileMonikerCreator
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
struct MonikerBinStreamCreator : public IMonikerBinStreamCreator
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


void InitMonikerFactories()
    {   
    static const struct MonikerBinStreamCreator s_MonikerBinStreamCreator;
    static const struct LocalFileMonikerCreator s_LocalFileMonikerCreator;
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
}; 



Bentley::RefCountedPtr<DgnDocument> docPtr = nullptr;
Bentley::RefCountedPtr<DgnFile> file = nullptr;

void InitializeSDK(DgnPlatformLib::Host& host)
        {
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
        static SampleHost smHost;
        ((SampleAdmin&)smHost.GetScalableMeshAdmin()).SetActiveModelRef(model);
        InitMonikerFactories();
        Bentley::DgnPlatform::Raster::RasterCoreLib::Initialize(*new AppRasterCoreLibHost());
        BeAssert(Bentley::DgnPlatform::Raster::RasterCoreLib::IsInitialized()); 
        }

void CloseSDK()
    {
    file->SetAbandonChangesFlag();
    file->ProcessChanges(DgnPlatform::DgnSaveReason::ApplInitiated);
    file = nullptr;
    }
    }