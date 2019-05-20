#include "DgnInterface.h"
#if defined(BEGIN_BENTLEY_NAMESPACE)
    #error This should be the first code that includes BeAssert.h
#endif
#pragma warning(disable:4312)
#pragma warning(disable:4091)
#pragma warning(disable:4273)
//  The Vancouver header files will not compile unless the there is a "using namespace Bentley".  Therefore we
//  have to disallow "using namespace BentleyG06".
#define NO_USING_NAMESPACE_BENTLEY 1

#include <DgnV8Api/Bentley/BeAssert.h>
#undef BeAssert
#undef BeDataAssert
#undef BeAssertOnce
#undef BeDataAssertOnce
#include <Bentley/BeAssert.h>

#define DGNV8_WSTRING_LEGACY_SUPPORT

#include <DgnV8Api/DgnPlatform/DgnCoreAPI.h>
#include <DgnV8Api/DgnPlatform/DgnPlatform.h>
#include <DgnV8Api/DgnPlatform/DgnFile.h>
#include <DgnV8Api/DgnPlatform/DgnPlatformLib.h>
#include <DgnV8Api/DgnPlatform/DgnFileIO/BentleyDgn.h>
#include <DgnV8Api/DgnPlatform/DgnFileIO/DgnElements.h>
#include <DgnV8Api/DgnPlatform/DgnFileIO/ElementRefBase.h>
#include <DgnV8Api/DgnPlatform/DgnECManager.h>
#include <DgnV8Api/RasterCore/RasterDEMFilters.h>

namespace DgnV8Api    = Bentley::DgnPlatform;
namespace Bentley
    {

    namespace DgnPlatform
        {
        typedef DgnFile&          DgnFileR;
        typedef DgnFile const&    DgnFileCR;
        typedef DgnFile*          DgnFileP;
        typedef RefCountedPtr<DgnFile>          DgnFilePtr;
        typedef DgnFile const*    DgnFileCP;
        typedef DgnModel&         DgnModelR;
        typedef DgnModel const&   DgnModelCR;
        typedef DgnModel*         DgnModelP;
        typedef DgnModel const*  DgnModelCP;
        typedef DgnModelRef&         DgnModelRefR;
        typedef DgnModelRef const&   DgnModelRefCR;
        typedef DgnModelRef*         DgnModelRefP;
        typedef DgnModelRef const*   DgnModelRefCP;
        typedef RefCountedPtr<DgnModel>          DgnModelPtr;
        typedef DgnFileLoadContext*          DgnFileLoadContextP;
        typedef DgnFileSupplyRights*          DgnFileSupplyRightsP;
        typedef MSElementDescr&          MSElementDescrR;
        typedef MSElementDescr const&    MSElementDescrCR;
        typedef MSElementDescr*          MSElementDescrP;
        typedef MSElementDescr const*    MSElementDescrCP;
        typedef ElementRefBase*          ElementRefP;
        }
    }

#include "ElementType.h"

int testFunction()
{
return 0;
}


struct TmpLoadContext
{
    DgnV8Api::DgnFileLoadContext      m_context;
    DgnV8Api::DgnFileSupplyRightsPtr  m_rights;

    TmpLoadContext (DgnV8Api::DgnFileSupplyRightsP userContext, DgnV8Api::DgnFileOpenParams::DecryptionOptions decrypt)
        {
        assert(userContext == NULL);
        /*if (userContext == NULL)
            {
            if ((decrypt & DgnFileOpenParams::DECRYPT_UseSuppliedPassword) && getCommandLinePassword())
                {
                m_rights = suppliedPassword_create (getCommandLinePassword());
                m_context.SetDgnFileSupplyRights(m_rights.get());
                }
            else
                {
                bool noPrompt = (decrypt & DgnFileOpenParams::DECRYPT_NoPrompt) || ISessionMgr::InBatchModeOrBatchProcessing ();
                m_rights = passwordPrompt_create (noPrompt);
                m_context.SetDgnFileSupplyRights(m_rights.get());
                }
            }
        else*/
            m_context.SetDgnFileSupplyRights (userContext);
        }

    DgnV8Api::DgnFileLoadContextP GetContext () {return &m_context;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFileOpenParams::DgnFileOpenParams (WCharCP name, bool readonly, DgnV8Api::DgnFilePurpose filePurpose)
    {
    Init (name, readonly);
    m_document      = DgnV8Api::DgnDocument::CreateForLocalFile (name);
    m_filePurpose   = filePurpose;
    }

DgnV8Api::DgnFileOpenParams::~DgnFileOpenParams ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnV8Api::DgnFileOpenParams::Init (WCharCP name, bool readonly)
    {
    m_openStatus         = DgnV8Api::DGNFILE_STATUS_UnknownError;
    m_openForWriteStatus = DgnV8Api::DGNFILE_STATUS_UnknownError;
    m_errorDetails       = NULL;
    m_fileName           = name;
    m_readonly           = readonly;
    m_alsoTryReadonly    = true;        // only means anything if readonly == false
    m_allowShare         = true;
    m_ownerChangeAlert   = !readonly;
    m_readonlyAlert      = false;
    m_loadContext        = NULL;
    m_decrypt = DgnV8Api::DgnFileOpenParams::DecryptionOptions::DECRYPT_MasterfileOrReference;
    m_dontShareDWG       = false;
    m_foreignUnitMode    = -1;
    }
/*---------------------------------------------------------------------------------**//**
* This function exists only because you cannot use SEH in any function that has an object with a destructor.
* @bsimethod                                    Keith.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnV8Api::DgnFileStatus loadFromEmbeddedFileSafe (DgnV8Api::DgnFileR dgnFile, DgnV8Api::DgnFileR containerDgn, int32_t embedId, DgnV8Api::DgnFileLoadContextP loadContext)
    {

        return (DgnV8Api::DgnFileStatus) dgnFile.LoadFromContainerFile (containerDgn, embedId, loadContext, true);

    }

/*---------------------------------------------------------------------------------**//**
* This function exists only because you cannot use SEH in any function that has an object with a destructor.
* @bsimethod                                    john.gooding                    01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnV8Api::DgnFileStatus loadFileSafe (DgnV8Api::DgnFileR dgnFile, DgnV8Api::DgnFileStatus* openForWriteStatus, DgnV8Api::DgnFileLoadContextP loadContext)
    {

        {
        return (DgnV8Api::DgnFileStatus) dgnFile.LoadFile ((StatusInt*) openForWriteStatus, loadContext);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFilePtr DgnV8Api::DgnFileOpenParams::CreateFileAndLoad ()
    {
    if (NULL != m_errorDetails)
        *m_errorDetails = L"";

    m_openStatus = DgnV8Api::DGNFILE_STATUS_UnknownError;

    if (m_document.IsNull())
        {
        m_openStatus = DgnV8Api::DGNOPEN_STATUS_FileNotFound;
        return NULL;
        }

    // if readonly and shareable, look for a shareable entry with same name.
    if (m_readonly && m_allowShare)
        {
        DgnV8Api::DgnFileP openFile = DgnV8Api::DgnFile::GetOpenedFileArray().GetOpenFileByName (m_fileName.c_str(), NULL, -1, true, false);
        if (NULL != openFile)
            {
            if (!m_dontShareDWG || openFile->GetTargetFormat() != DgnV8Api::DgnFileFormatType::DWG) // TR321301
                {
                m_openStatus = DgnV8Api::DGNFILE_STATUS_Success;
                return  openFile;
                }
            }
        }

    DgnV8Api::DgnFileOpenMode openMode = m_readonly ? DgnV8Api::DgnFileOpenMode::ReadOnly : DgnV8Api::DgnFileOpenMode::PreferablyReadWrite;
    //DgnFilePtr dgnFile = new UstnDgnFile (*m_document, openMode, m_filePurpose);
    DgnV8Api::DgnFilePtr dgnFile = DgnV8Api::DgnFile::Create (*m_document.get(), openMode);    
    
    // Preserve "foreign unit" specification supplied in open params (dwg ref attach units)...
    if (m_foreignUnitMode > 0)
        dgnFile->SetForeignUnitMode (m_foreignUnitMode);


    //  Try to load the file
    if (true) // this exists to limit the scope of the TmpLoadContext object.
        {
        TmpLoadContext tmpContext (m_loadContext.GetDgnFileSupplyRights(), m_decrypt);

        // First check to see whether someone is attempting to directly open an embedded file. If so, look to see whether the
        // containing file is already opened and load from it. Otherwise, we can't load this file, just return an error.
        // [N.B. this is not the way we load embedded references - the reference file code does this directly.]

        WString containerName;
        int32_t embedId;
        if (SUCCESS == DgnV8Api::DgnFile::ParsePackagedName (&containerName, &embedId, NULL, dgnFile->GetFileName().c_str()))
            {
            DgnV8Api::DgnFileP containerDgn = DgnV8Api::DgnFile::GetOpenedFileArray().GetOpenFileByName (containerName.c_str(), NULL, -1, true, false);

            // only allow opening of an embedded file if it is readonly, shared, and its containing file is already open.
            m_openStatus = ((NULL == containerDgn) || !m_readonly || !m_allowShare) ? 
                                    DgnV8Api::DGNFILE_ERROR_InvalidOperationForNestedFile :
                                    loadFromEmbeddedFileSafe (*dgnFile, *containerDgn, embedId, tmpContext.GetContext());
            }
        else
            m_openStatus = loadFileSafe (*dgnFile, &m_openForWriteStatus, tmpContext.GetContext());
        }

    if (DGNFILE_STATUS_Success != m_openStatus)
        {
        }
    else
        {
        if (m_allowShare)
            dgnFile->SetShareFlag(true);
        }

    //ShowLockedFileAlert (*dgnFile);
    //ShowAuthorChangeAlert (*dgnFile);

    // if m_readonly is false, we always try to open the r/w, but then r/o. That's so we can read the file to determine if
    // another user is currently editing and get their name. But if "m_alsoTryReadonly" is false, then he doesn't want the readonly
    // file. Just set the openStatus to the error and we'll release the file below.
    if (!m_alsoTryReadonly && DidWriteAccessFail())
        m_openStatus = m_openForWriteStatus;
    else if (SUCCESS == m_openStatus)
        return  dgnFile;

    m_openStatus = m_openStatus;    // ???
    return  NULL;
    }

DgnFileHandle OpenDGNFile  (const wchar_t*      pName,
                            int&        status)
    {
                                bool                 readOnly = TRUE;
    //MessageDestination   displayError = MESSAGE_DEST_None;    
    
    DgnV8Api::DgnFileOpenParams openParams (pName, TO_BOOL(readOnly), DgnV8Api::DgnFilePurpose::Unknown);   
    DgnV8Api::DgnFileP file = openParams.CreateFileAndLoad().get();  
    if (NULL == file)
        status = openParams.GetOpenStatus();
    else file->AddRef();
    return (DgnFileHandle)file;
    }



namespace
    {

    struct WorkingModelReprojectionEnabler
        {
        private:
            static WString      s_varName;

            bool                m_wasAlreadyDefined;
        public:
            explicit            WorkingModelReprojectionEnabler()
                : m_wasAlreadyDefined(SUCCESS == DgnV8Api::ConfigurationManager::IsVariableDefined(s_varName.c_str()))
                {
                if (!m_wasAlreadyDefined)
                    DgnV8Api::ConfigurationManager::DefineVariable(s_varName.c_str(), L"1", DgnV8Api::ConfigurationVariableLevel::User);
                }

            ~WorkingModelReprojectionEnabler()
                {
                if (!m_wasAlreadyDefined)
                    DgnV8Api::ConfigurationManager::UndefineVariable(s_varName.c_str());
                }

        };
    WString WorkingModelReprojectionEnabler::s_varName = L"MS_GEOCOORDINATE_OTFTRANSFORM_WORKINGMODELREF";

    }

DgnModelHandle FindDGNModel(const DgnFileHandle&     dgnFile,
                               int                  modelID,
                               int&               status)
    {
    if (nullptr == dgnFile)
        {
        status = DgnV8Api::DGNOPEN_STATUS_BAD_FILE;
        return nullptr;
        }

    DgnV8Api::DgnFileP fileP = (DgnV8Api::DgnFileP)dgnFile;
    if (!fileP->IsValidModelID(modelID))
        {
        status = DgnV8Api::DGNFILE_ERROR_NoSuchModel;
        return nullptr;
        }

    DgnV8Api::DgnModelPtr model = fileP->LoadModelById(modelID);
    assert(model != NULL);
    model->AddRef();

    // Set the model so that we do not attempt to reproject or transform geographic attachments, for performance reasons.
    // Those references are marked as "not found" with the reason set to "missing geographic coordinate system".
    // Callers that want reprojection should call this function with loadRefs set to false, and upon return
    // call SetGeoAttachmentHandling (GeoAttachmentHandling::Default) followed by model->ReadAndLoadDgnAttachments.
    model->SetGeoAttachmentHandling(DgnV8Api::GeoAttachmentHandling::DoNotReproject);
    status = SUCCESS;
    model->FillSections(DgnV8Api::DgnModelSections::Model);


    //status = mdlModelRef_createWorking (&pInternalModelRef, dgnFile.GetP(), modelID, TRUE, FALSE);    


    // Load references making sure reprojection is enabled for working models.
    // See geocoord\mstn\transformcache.cpp -> AllowReprojectionOfWorkingModelRefAttachment
    // for more informations.
        {
        WorkingModelReprojectionEnabler enableModelReprojection;

        DgnV8Api::DgnAttachmentLoadOptions opts(TRUE, FALSE, TRUE);

        if (BSISUCCESS != model.get()->ReadAndLoadDgnAttachments(opts))
            {
            status = BSIERROR;
            return nullptr;
            }
        }

    return (DgnModelHandle)model.get();
    }

DgnModelHandle FindDGNModel(const DgnFileHandle&    dgnFile,
                               const wchar_t*          modelName,
                               int&              status)
    {

    DgnV8Api::ModelId modelID = 0;

    if (nullptr == dgnFile)
        {
        status = DgnV8Api::DGNOPEN_STATUS_BAD_FILE;
        return nullptr;
        }

    DgnV8Api::DgnFileP fileP = (DgnV8Api::DgnFileP)dgnFile;
    DgnV8Api::DgnFileFormatType format = DgnV8Api::DgnFileFormatType::V8;
    Bentley::dgnFileObj_getVersion(fileP, &format, NULL, NULL);

    if ((NULL == modelName) || ((DgnV8Api::DgnFileFormatType::V7 == format) && (0 == wcsicmp(modelName, L"default"))))
        {
        modelID = fileP->GetDefaultModelId();
        }
    else
        {
        if (SUCCESS != (status = Bentley::dgnFileObj_findModelIDByName(fileP, &modelID, modelName)))
            {
            status = DgnV8Api::DGNFILE_ERROR_NoSuchModel;
            return nullptr;
            }
        }

    return FindDGNModel(dgnFile, modelID, status);
    }

int                      FindLevel(const DgnModelHandle&                model,
                                              LevelId                                 levelID)
    {
    if (!((DgnV8Api::DgnModelRefP)model)->GetLevelCache().GetLevel(levelID, true).IsValid())
        return ERROR;

    return SUCCESS;
    }

bool FindDGNLevelIDFromName(DgnV8Api::DgnModelRefP    modelRefP,
                            const wchar_t*  levelName,
                            LevelId&        levelID)
    {
    DgnV8Api::FileLevelCache* fileLevelCacheP = modelRefP->GetFileLevelCacheP();
    assert(fileLevelCacheP != 0);
    levelID = fileLevelCacheP->GetNameDictionary().GetNameId(levelName);

    return levelID != LEVEL_DICT_NULL_ID;
    }

int                      FindLevel(const DgnModelHandle&              model,
                                              const wchar_t*                          levelName,
                                              LevelId&                      levelID)
    {
    if (!FindDGNLevelIDFromName(((DgnV8Api::DgnModelRefP)model), levelName, levelID))
        return ERROR;

    return SUCCESS;
    }

//Copied from MstnPlatform\PublicAPI\Mstn\MdlApi\scanner.h
#define ELEINVISIBLE 0x0080

struct ElementIterator
    {
    DgnV8Api::ScanCriteria* m_scanCriteria;

    explicit            ElementIterator(DgnV8Api::DgnModelRefP modelRefPtr, LevelId levelID)
        {
        m_scanCriteria = new DgnV8Api::ScanCriteria();
        m_scanCriteria->AddSingleLevelTest(levelID);
        m_scanCriteria->SetPropertiesTest(0, ELEINVISIBLE);
        m_scanCriteria->SetModelRef(modelRefPtr);
        m_scanCriteria->SetReturnType(DgnV8Api::MSSCANCRIT_ITERATE_ELMREF, false, false);
        }

    ~ElementIterator()
        {
        delete m_scanCriteria;
        }

    int                 AddSingleElementTypeTest(const DgnV8Api::MSElementTypes& type)
        {
        m_scanCriteria->AddSingleElementTypeTest(type);
        return SUCCESS;
        }

    int                 Scan(Bentley::PFScanElemRefCallback callbackFunc, void* userArgP)
        {
        m_scanCriteria->SetElemRefCallback(callbackFunc, userArgP);
        return m_scanCriteria->Scan(NULL, NULL, NULL, NULL);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int ComputeCountsCallback(DgnV8Api::ElementRefP elmRef, void* userArgP, DgnV8Api::ScanCriteria* scanCritP)
    {
    ElementStats& stats = *((ElementStats*)userArgP);
    
    DgnV8Api::EditElementHandle handle(elmRef, scanCritP->GetModelRef());
    DgnV8Api::MSElementDescrP edP = handle.GetElementDescrP();

    // Extract element type and compute count
    ElementPointExtractor::GetFor(edP).ComputeStats(edP, stats.m_point);
    ElementLinearExtractor::GetFor(edP).ComputeStats(edP, stats.m_linear);
    ElementMeshExtractor::GetFor(edP).ComputeStats(edP, stats.m_mesh);

    return SUCCESS;
    }
 
/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ComputeCounts(ElementStats& stats, DgnModelHandle    modelRefP, LevelId        levelID)
    {
    ElementIterator elementIt((DgnV8Api::DgnModelRefP)modelRefP, levelID);
    elementIt.AddSingleElementTypeTest(DgnV8Api::LINE_ELM);
    elementIt.AddSingleElementTypeTest(DgnV8Api::LINE_STRING_ELM);
    elementIt.AddSingleElementTypeTest(DgnV8Api::CMPLX_STRING_ELM);
    elementIt.AddSingleElementTypeTest(DgnV8Api::SHAPE_ELM);
    elementIt.AddSingleElementTypeTest(DgnV8Api::CMPLX_SHAPE_ELM);
    elementIt.AddSingleElementTypeTest(DgnV8Api::MESH_HEADER_ELM);
    elementIt.Scan(ComputeCountsCallback, &stats);
    }



#define PointCloudMinorId_Handler 1
int CreateSourceRefListCallback(DgnV8Api::ElementRefP elmRef, void* userArgP, DgnV8Api::ScanCriteria* scanCritP)
    {
    std::list<RefToModelAndElement>& ref = *((std::list<RefToModelAndElement>*)userArgP);

    DgnV8Api::EditElementHandle elHandle(elmRef);

    bool supported = false;

    // Raster element
    DgnV8Api::MSElementDescrP edP = elHandle.GetElementDescrP();

    if (DgnV8Api::RASTER_FRAME_ELM == edP->el.ehdr.type)
        {
        DgnV8Api::DgnRaster* dgnRasterP = 0;
        if (BSISUCCESS == DgnV8Api::mdlRaster_handleFromElementRefGet(&dgnRasterP, elHandle.GetElementRef(), elHandle.GetModelRef()) &&
            DgnV8Api::mdlRaster_HasDEMFilters(dgnRasterP))
            {
            RefToModelAndElement elem;
            elem.type = ElementType::RASTER_ELEM_TYPE;
            elem.targetModel = (DgnModelHandle)&elHandle.GetModelRef();
            elem.targetElement = (DgnElementHandle)&elHandle.GetElementRef();
            ref.push_back(elem);
            supported = true;
            }
        }
    else if (DgnV8Api::EXTENDED_ELM == edP->el.ehdr.type)
        {
        // DTM element        
        if (DgnV8Api::ElementHandlerManager::GetHandlerId(elHandle) == DgnV8Api::MrDTMDefaultElementHandler::GetElemHandlerId())
            {
            // STM element
            RefToModelAndElement elem;
            elem.type = ElementType::STM_ELEM_TYPE;
            elem.targetModel = (DgnModelHandle)&elHandle.GetModelRef();
            elem.targetElement = (DgnElementHandle)&elHandle.GetElementRef();
            ref.push_back(elem);
            supported = true;
            }
        else
            if (DgnV8Api::ElementHandlerManager::GetHandlerId(elHandle) == DgnV8Api::DTMElementHandler::GetElemHandlerId()
                || DgnV8Api::ElementHandlerManager::GetHandlerId(elHandle) == DgnV8Api::ElementHandlerId(DgnV8Api::XATTRIBUTEID_CIF, DgnV8Api::ELEMENTHANDLER_DTMOVERRIDESYMBOLOGY))
                {
                // Civil element
                RefToModelAndElement elem;
                elem.type = ElementType::CIVIL_ELEM_TYPE;
                elem.targetModel = (DgnModelHandle)&elHandle.GetModelRef();
                elem.targetElement = (DgnElementHandle)&elHandle.GetElementRef();
                ref.push_back(elem);
                supported = true;
                }
        // POD element        
            else if (DgnV8Api::ElementHandlerManager::GetHandlerId(elHandle) == DgnV8Api::ElementHandlerId(DgnV8Api::XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_Handler))
                {
                RefToModelAndElement elem;
                elem.type = ElementType::POD_ELEM_TYPE;
                elem.targetModel = (DgnModelHandle)&elHandle.GetModelRef();
                elem.targetElement = (DgnElementHandle)&elHandle.GetElementRef();
                ref.push_back(elem);
                supported = true;
                }
        }

    // Enable for debug purpose
    assert(supported);

    return BSISUCCESS;
    }

void ImportSourceRef(std::list<RefToModelAndElement>& refs, DgnModelHandle    modelRefP, LevelId        levelID)
    {
    ElementIterator elementIt((DgnV8Api::DgnModelRefP)modelRefP, levelID);
    elementIt.AddSingleElementTypeTest(RASTER_FRAME_ELM);
    elementIt.AddSingleElementTypeTest(EXTENDED_ELM);
    elementIt.Scan(CreateSourceRefListCallback, &refs);
    }