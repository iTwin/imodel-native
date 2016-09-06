/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/DGNModelUtilities.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include "DGNModelUtilities.h"

#pragma warning( disable : 4273 )

USING_NAMESPACE_BENTLEY_DGNPLATFORM
//BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

////FROM UstnDgnFile.cpp//////////////////////////////////////////////////
//NEEDS_WORK_SM_IMPORTER : Copied from UstnDgnFile.cpp to avoid dependency with MstnPlatform
/**=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   08/07
+===============+===============+===============+===============+===============+======*/
struct TmpLoadContext
{
    DgnFileLoadContext      m_context;
    DgnFileSupplyRightsPtr  m_rights;

    TmpLoadContext (DgnFileSupplyRightsP userContext, DgnFileOpenParams::DecryptionOptions decrypt)
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

    DgnFileLoadContextP GetContext () {return &m_context;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFileOpenParams::DgnFileOpenParams (WCharCP name, bool readonly, DgnFilePurpose filePurpose)
    {
    Init (name, readonly);
    m_document      = DgnDocument::CreateForLocalFile (name);
    m_filePurpose   = filePurpose;
    }

DgnFileOpenParams::~DgnFileOpenParams ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnFileOpenParams::Init (WCharCP name, bool readonly)
    {
    m_openStatus         = DGNFILE_STATUS_UnknownError;
    m_openForWriteStatus = DGNFILE_STATUS_UnknownError;
    m_errorDetails       = NULL;
    m_fileName           = name;
    m_readonly           = readonly;
    m_alsoTryReadonly    = true;        // only means anything if readonly == false
    m_allowShare         = true;
    m_ownerChangeAlert   = !readonly;
    m_readonlyAlert      = false;
    m_loadContext        = NULL;
    m_decrypt            = DECRYPT_MasterfileOrReference;
    m_dontShareDWG       = false;
    m_foreignUnitMode    = -1;
    }
/*---------------------------------------------------------------------------------**//**
* This function exists only because you cannot use SEH in any function that has an object with a destructor.
* @bsimethod                                    Keith.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnFileStatus loadFromEmbeddedFileSafe (DgnFileR dgnFile, DgnFileR containerDgn, int32_t embedId, DgnFileLoadContextP loadContext)
    {
    __try
        {
        return (DgnFileStatus) dgnFile.LoadFromContainerFile (containerDgn, embedId, loadContext, true);
        }    
    __except (EXCEPTION_EXECUTE_HANDLER /*mstnException_filterException (GetExceptionInformation(), false)*/)
        {
        }
    
    return DGNOPEN_STATUS_BAD_FILE;
    }

/*---------------------------------------------------------------------------------**//**
* This function exists only because you cannot use SEH in any function that has an object with a destructor.
* @bsimethod                                    john.gooding                    01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnFileStatus loadFileSafe (DgnFileR dgnFile, DgnFileStatus* openForWriteStatus, DgnFileLoadContextP loadContext)
    {
    __try
        {
        return (DgnFileStatus) dgnFile.LoadFile ((StatusInt*) openForWriteStatus, loadContext);
        }
    __except (EXCEPTION_EXECUTE_HANDLER /*mstnException_filterException (GetExceptionInformation(), false)*/)
        {
        }

    return DGNOPEN_STATUS_BAD_FILE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFilePtr DgnFileOpenParams::CreateFileAndLoad ()
    {
    if (NULL != m_errorDetails)
        *m_errorDetails = L"";

    m_openStatus = DGNFILE_STATUS_UnknownError;

    if (m_document.IsNull())
        {
        m_openStatus = DGNOPEN_STATUS_FileNotFound;
        return NULL;
        }

    // if readonly and shareable, look for a shareable entry with same name.
    if (m_readonly && m_allowShare)
        {
        DgnFileP openFile = DgnFile::GetOpenedFileArray().GetOpenFileByName (m_fileName.c_str(), NULL, -1, true, false);
        if (NULL != openFile)
            {
            if (!m_dontShareDWG || openFile->GetTargetFormat() != DgnFileFormatType::DWG) // TR321301
                {
                m_openStatus = DGNFILE_STATUS_Success;
                return  openFile;
                }
            }
        }

    DgnFileOpenMode openMode = m_readonly ? DgnFileOpenMode::ReadOnly : DgnFileOpenMode::PreferablyReadWrite;
    //DgnFilePtr dgnFile = new UstnDgnFile (*m_document, openMode, m_filePurpose);
    DgnFilePtr dgnFile = DgnFile::Create (*m_document.get(), openMode);    
    
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
        if (SUCCESS == DgnFile::ParsePackagedName (&containerName, &embedId, NULL, dgnFile->GetFileName().c_str()))
            {
            DgnFileP containerDgn = DgnFile::GetOpenedFileArray().GetOpenFileByName (containerName.c_str(), NULL, -1, true, false);

            // only allow opening of an embedded file if it is readonly, shared, and its containing file is already open.
            m_openStatus = ((NULL == containerDgn) || !m_readonly || !m_allowShare) ? 
                                    DGNFILE_ERROR_InvalidOperationForNestedFile :
                                    loadFromEmbeddedFileSafe (*dgnFile, *containerDgn, embedId, tmpContext.GetContext());
            }
        else
            m_openStatus = loadFileSafe (*dgnFile, &m_openForWriteStatus, tmpContext.GetContext());
        }

    if (DGNFILE_STATUS_Success != m_openStatus)
        {
        /*  tell caller about the problem */
        if (DGNOPEN_STATUS_InsecureEnvironment==m_openStatus && NULL != m_errorDetails)
            dgnFileObj_getNonCompliantAppsLoadedReport (*m_errorDetails, dgnFile.get());
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

////FROM DgnFile.cpp END//////////////////////////////////////////////////
//END_BENTLEY_DGNPLATFORM_NAMESPACE


USING_NAMESPACE_BENTLEY_SCALABLEMESH

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool FindDGNLevelIDFromName    (DgnModelRefP    modelRefP,
                                const WChar*  levelName,
                                LevelId&        levelID)
    {
    FileLevelCacheP fileLevelCacheP = modelRefP->GetFileLevelCacheP();
    assert(fileLevelCacheP != 0);    
    levelID = fileLevelCacheP->GetNameDictionary().GetNameId(levelName);

    return levelID != LEVEL_DICT_NULL_ID;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNFileHolder OpenDGNFile  (const WChar*      pName,
                            StatusInt&        status)
    {
    bool                 readOnly = TRUE;
    uint32_t               rights = DGNFILE_RIGHT_Any;
    //MessageDestination   displayError = MESSAGE_DEST_None;    
    
    DgnFileOpenParams openParams (pName, TO_BOOL(readOnly), DgnPlatform::DgnFilePurpose::Unknown);    
           
    DGNFileHolder dgnFile(DGNFileHolder::CreateFromWorking(openParams.CreateFileAndLoad()));
    if (NULL == dgnFile.GetP())
        {
        status = openParams.GetOpenStatus();
        return dgnFile;
        } 
    
    StatusInt checkRightsStatus;
    if (SUCCESS != (checkRightsStatus = dgnFileObj_checkRights0 (dgnFile.GetP(), rights)))
        {
        dgnFile.Reset();
        status = checkRightsStatus;
        return dgnFile;        
        }

    // we need to make sure that as many dependencies as possible have been resolved in the dgnFileObj that we just opened here.    
    DependencyManager::ProcessAffected ();

    status = BSISUCCESS;
    return dgnFile;
    }


namespace {

struct WorkingModelReprojectionEnabler
    {
private:
    static WString      s_varName;

    bool                m_wasAlreadyDefined;
public:
    explicit            WorkingModelReprojectionEnabler                ()
        :   m_wasAlreadyDefined(SUCCESS == ConfigurationManager::IsVariableDefined (s_varName.c_str()))
        {
        if (!m_wasAlreadyDefined)
            ConfigurationManager::DefineVariable (s_varName.c_str(), L"1", ConfigurationVariableLevel::User);            
        }

                        ~WorkingModelReprojectionEnabler               ()
        {
        if (!m_wasAlreadyDefined)
            ConfigurationManager::UndefineVariable (s_varName.c_str());            
        }

    };
WString WorkingModelReprojectionEnabler::s_varName = L"MS_GEOCOORDINATE_OTFTRANSFORM_WORKINGMODELREF";

}


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNModelRefHolder FindDGNModel    (const DGNFileHolder&     dgnFile,
                                   ModelId                  modelID,
                                   StatusInt&               status)
    {    
    if (0 == dgnFile.GetP())
        {
        status = DGNOPEN_STATUS_BAD_FILE;
        return DGNModelRefHolder();
        }
   
    if (!dgnFile.GetP()->IsValidModelID(modelID))
        {
        status = DGNFILE_ERROR_NoSuchModel;
        return DGNModelRefHolder();
        }
         
    DgnModelPtr model = dgnFile.GetP()->LoadModelById (modelID);
    assert(model != NULL);        
    model->AddRef ();

    // Set the model so that we do not attempt to reproject or transform geographic attachments, for performance reasons.
    // Those references are marked as "not found" with the reason set to "missing geographic coordinate system".
    // Callers that want reprojection should call this function with loadRefs set to false, and upon return
    // call SetGeoAttachmentHandling (GeoAttachmentHandling::Default) followed by model->ReadAndLoadDgnAttachments.
    model->SetGeoAttachmentHandling (GeoAttachmentHandling::DoNotReproject);
    status = SUCCESS;    
    model->FillSections (DgnModelSections::Model);        
    

    //status = mdlModelRef_createWorking (&pInternalModelRef, dgnFile.GetP(), modelID, TRUE, FALSE);    

    DGNModelRefHolder modelRefHoler(DGNModelRefHolder::CreateFromWorking(dgnFile, model.get()));

    // Load references making sure reprojection is enabled for working models.
    // See geocoord\mstn\transformcache.cpp -> AllowReprojectionOfWorkingModelRefAttachment
    // for more informations.
        {
        WorkingModelReprojectionEnabler enableModelReprojection;

        DgnAttachmentLoadOptions opts (TRUE, FALSE, TRUE);

        if (BSISUCCESS != model.get()->ReadAndLoadDgnAttachments (opts))
            {
            status = BSIERROR;
            return DGNModelRefHolder();
            }      
        }

    return modelRefHoler;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNModelRefHolder FindDGNModel (const DGNFileHolder&    dgnFile,
                                const WChar*          modelName,
                                StatusInt&              status)
    {

    ModelId modelID = 0;

    if (0 == dgnFile.GetP())
        {
        status = DGNOPEN_STATUS_BAD_FILE;
        return DGNModelRefHolder();
        }

    DgnFileFormatType format = DgnFileFormatType::V8;
    dgnFileObj_getVersion (dgnFile.GetP(), &format, NULL, NULL);

    if ( (NULL == modelName) || ( (DgnFileFormatType::V7 == format) && (0 == wcsicmp (modelName, L"default")) ) )
        {
        modelID = dgnFile.GetP()->GetDefaultModelId();
        }
    else
        {
        if (SUCCESS != (status = dgnFileObj_findModelIDByName (dgnFile.GetP(), &modelID, modelName)))
            {
            status = DGNFILE_ERROR_NoSuchModel;
            return DGNModelRefHolder();
            }
        }

    return FindDGNModel(dgnFile, modelID, status);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNModelRefHolder FindDGNReferenceFromRootModel    (const DGNModelRefHolder&    rootModel,
                                                    const WChar*                rootToRefPersitentPath,
                                                    StatusInt&                  status)
    {
    DgnModelRefP rootModelRefP = rootModel.GetP();    

    PersistentElementPath rootToRefPersistantPath;
    if (BSISUCCESS != rootToRefPersistantPath.FromWString(rootToRefPersitentPath))
        {
        status = BSIERROR;
        return DGNModelRefHolder();
        }
    
    ElementHandle elH(rootToRefPersistantPath.EvaluateElement(rootModelRefP));
    EditElementHandle referenceElementHandle(elH.GetElementRef(), elH.GetModelRef());

    if (!referenceElementHandle.IsValid())
        {
        status = BSIERROR;
        return DGNModelRefHolder();
        }

    DgnModelRefP referenceModelRefP = INVALID_MODELREF;
    ElementId    elementId(referenceElementHandle.GetElementRef()->GetElementId());
           
    DgnAttachmentP refP(DgnAttachment::FindByElementId (referenceElementHandle.GetModelRef(), elementId));
    if (NULL != refP)
        {
        referenceModelRefP = refP;
        }
    else
        {
        status = BSIERROR;
        return DGNModelRefHolder();
        }

    // Ensure that reference cache is correctly initialized.
    assert(0 != referenceModelRefP->GetDgnFileP());

    status = BSISUCCESS;
    return DGNModelRefHolder::CreateFromReference(rootModel, referenceModelRefP);
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE

