/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSources.cpp $
|    $RCSfile: ScalableMeshSources.cpp,v $
|   $Revision: 1.27 $
|       $Date: 2012/02/23 01:54:03 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>

#include "ScalableMeshSources.h"
#include <ScalableMesh/IScalableMeshSourceVisitor.h>
#include <ScalableMesh/IScalableMeshStream.h>

#include "ScalableMeshTime.h"
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>

#include "ScalableMeshEditListener.h"

#include <ScalableMesh/Import/ImportSequence.h>
#include <ScalableMesh/Import/Command/All.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>
#include <ScalableMesh/Type/IScalableMeshTIN.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DocumentEnv::Impl
    {
    WString             m_currentDir;

    explicit            Impl                           (const WChar* currentDir)
        :   m_currentDir(currentDir)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentEnv::DocumentEnv (const WChar* currentDir)
    :   m_implP(new Impl(currentDir))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentEnv::~DocumentEnv ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentEnv::DocumentEnv (const DocumentEnv& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentEnv& DocumentEnv::operator=  (const DocumentEnv& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DocumentEnv::GetCurrentDir () const
    {
    return m_implP->m_currentDir;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DocumentEnv::GetCurrentDirCStr () const
    {
    return m_implP->m_currentDir.c_str();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void EditListener::NotifyOfPublicEdit ()
    {
    _NotifyOfPublicEdit();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void EditListener::NotifyOfLastEditUpdate (Time updatedLastEditTime)
    {
    return _NotifyOfLastEditUpdate(updatedLastEditTime);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSource::IDTMSource (Impl* implP)
    :   m_implP(implP)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IDTMSource::~IDTMSource ()
    {
    }

/*----------------------------------------------------------------------------+
|IDTMSource::IsReachable
+----------------------------------------------------------------------------*/ 
bool IDTMSource::IsReachable () const
    {
    return m_implP->_IsReachable();
    }

/*----------------------------------------------------------------------------+
|IDTMSource::GetMoniker
+----------------------------------------------------------------------------*/ 
const IMoniker& IDTMSource::GetMoniker () const
    {
    assert(0 != m_implP->m_monikerPtr.get());
    return *m_implP->m_monikerPtr;
    }

/*----------------------------------------------------------------------------+
|IDTMSource::GetSourceDataType
+----------------------------------------------------------------------------*/
DTMSourceDataType IDTMSource::GetSourceType () const
    {
    return m_implP->m_sourceDataType;
    }

/*----------------------------------------------------------------------------+
|IDTMSource::GetLastModified
+----------------------------------------------------------------------------*/
Time IDTMSource::GetLastModified () const
    {
    return m_implP->GetLastModified();
    }

Time IDTMSource::GetLastModifiedCheckTime () const
    {
    return m_implP->GetLastModifiedCheckTime();
    }

/*----------------------------------------------------------------------------+
|IDTMSource::SetLastModified
+----------------------------------------------------------------------------*/
void IDTMSource::SetLastModified (const Time& time)
    {
    m_implP->SetLastModified(time);
    }

StatusInt IDTMSource::InternalUpdateLastModified ()
    {
    return m_implP->_UpdateLastModified(); 
    }

/*----------------------------------------------------------------------------+
|IDTMSource::UpdateLastModified
+----------------------------------------------------------------------------*/
StatusInt IDTMSource::UpdateLastModified ()
    {
    StatusInt status = m_implP->_UpdateLastModified(); 
    if (BSISUCCESS != status)
        return status;

    if (0 != m_implP->m_editListenerP)
        m_implP->m_editListenerP->NotifyOfLastEditUpdate(m_implP->m_lastModified);

    return BSISUCCESS;
    }

/*----------------------------------------------------------------------------+
|IDTMSource::ResetLastModified
+----------------------------------------------------------------------------*/
void IDTMSource::ResetLastModified ()
    {
    m_implP->ResetLastModified();
    }

/*----------------------------------------------------------------------------+
|IDTMSource::EditConfig
+----------------------------------------------------------------------------*/
SourceImportConfig& IDTMSource::EditConfig ()
    {
    return m_implP->m_config;
    }

/*----------------------------------------------------------------------------+
|IDTMSource::GetConfig
+----------------------------------------------------------------------------*/
const SourceImportConfig& IDTMSource::GetConfig () const
    {
    return m_implP->m_config;
    }

/*----------------------------------------------------------------------------+
|IDTMSource::Accept
+----------------------------------------------------------------------------*/
void IDTMSource::Accept (IDTMSourceVisitor& visitor) const
    {
    _Accept(visitor);
    }

/*----------------------------------------------------------------------------+
|IDTMSource::Clone
+----------------------------------------------------------------------------*/
IDTMSourcePtr IDTMSource::Clone () const
    {
    return _Clone();
    }

namespace {

/*----------------------------------------------------------------------------+
|DoImportSourcePoints
+----------------------------------------------------------------------------*/   
bool DoImportSourcePoints (DTMSourceDataType type)
    {
    return DTM_SOURCE_DATA_DTM == type || 
           DTM_SOURCE_DATA_POINT == type;
    }

/*----------------------------------------------------------------------------+
|DoImportSourceLinear
+----------------------------------------------------------------------------*/     
bool DoImportSourceLinear (DTMSourceDataType type)
    {
    return DTM_SOURCE_DATA_DTM == type || 
           DTM_SOURCE_DATA_BREAKLINE == type ||
           DTM_SOURCE_DATA_CLIP == type ||
           DTM_SOURCE_DATA_MASK == type;
    }


ImportSequence CreateImportDTMSequence ()
    {
    ImportSequence sequence;
    sequence.push_back(ImportTypeCommand(PointTypeFamilyCreator().Create()));
    sequence.push_back(ImportTypeCommand(LinearTypeFamilyCreator().Create()));
    return sequence;
    }

ImportSequence CreateImportPointsSequence ()
    {
    ImportSequence sequence;
    sequence.push_back(ImportTypeCommand(PointTypeFamilyCreator().Create()));
    return sequence;
    }

ImportSequence CreateImportLinearsSequence ()
    {
    ImportSequence sequence;
    sequence.push_back(ImportTypeCommand(LinearTypeFamilyCreator().Create()));
    return sequence;
    }

ImportSequence CreateImportClipMasksSequence ()
    {
    ImportSequence sequence;
    sequence.push_back(ImportTypeCommand(LinearTypeFamilyCreator().Create()));
    return sequence;
    }



const ImportSequence& GetImportSequenceFor (DTMSourceDataType type)
    {
    static const ImportSequence IMPORT_DTM_SEQUENCE(CreateImportDTMSequence());
    static const ImportSequence IMPORT_POINTS_SEQUENCE(CreateImportPointsSequence());
    static const ImportSequence IMPORT_LINEARS_SEQUENCE(CreateImportLinearsSequence());
    static const ImportSequence IMPORT_CLIPMASKS_SEQUENCE(CreateImportClipMasksSequence());
    static const ImportSequence IMPORT_NOTHING_SEQUENCE;

    switch (type)
        {
    case DTM_SOURCE_DATA_DTM:
        return IMPORT_DTM_SEQUENCE;
    case DTM_SOURCE_DATA_POINT:
        return IMPORT_POINTS_SEQUENCE;
    case DTM_SOURCE_DATA_BREAKLINE:
        return IMPORT_LINEARS_SEQUENCE;
    case DTM_SOURCE_DATA_CLIP:
    case DTM_SOURCE_DATA_MASK:
        return IMPORT_CLIPMASKS_SEQUENCE;
    default:
        return IMPORT_NOTHING_SEQUENCE;
        }
    }
}





/*----------------------------------------------------------------------------+
|DTMSource class
+----------------------------------------------------------------------------*/                
IDTMSource::Impl::Impl (DTMSourceDataType       sourceDataType, 
                            const IMoniker*         monikerP)
    :   m_editListenerP(0),
        m_lastModified(CreateUnknownModificationTime()),
        m_config(GetImportSequenceFor(sourceDataType))
    {
    m_monikerPtr = const_cast<IMoniker*>(monikerP);    
    m_sourceDataType = sourceDataType;

    m_config.RegisterEditListener(*this);

    }
 
IDTMSource::Impl::Impl (const Impl& rhs)
    :   m_editListenerP(0),
        m_sourceDataType(rhs.m_sourceDataType),      
        m_lastModified(rhs.m_lastModified),
        m_config(rhs.m_config),
        m_monikerPtr(rhs.m_monikerPtr)
    {
    m_config.RegisterEditListener(*this);
    }

IDTMSource::Impl::~Impl()
    {
    m_config.UnregisterEditListener(*this);
    }



bool IDTMSource::IsPartOfCollection () const
    {
    return m_implP->IsPartOfCollection();
    }

void IDTMSource::RegisterEditListener (EditListener& listener)
    {
    assert(0 == m_implP->m_editListenerP);
    m_implP->m_editListenerP = &listener;
    }

void IDTMSource::UnregisterEditListener (const EditListener& listener)
    {
   // assert(m_implP->m_editListenerP == &listener);
    m_implP->m_editListenerP = 0;
    }


void IDTMSource::Impl::_NotifyOfPublicEdit ()
    {
    OnPublicEdit();
    }

void IDTMSource::Impl::_NotifyOfLastEditUpdate (Time updatedLastEditTime)
    {
    using namespace std::rel_ops;

    if (m_lastModified >= updatedLastEditTime)
        return;

    m_lastModified = updatedLastEditTime;

    if (0 != m_editListenerP)
        m_editListenerP->NotifyOfLastEditUpdate(m_lastModified);
    }

/*----------------------------------------------------------------------------+
|IDTMSource::_IsReachable
+----------------------------------------------------------------------------*/ 
bool IDTMSource::Impl::_IsReachable () const
    {
    assert(0 != m_monikerPtr.get());
    return m_monikerPtr->IsTargetReachable();
    }


void IDTMSource::Impl::OnPublicEdit ()
    {
    m_lastModified = Time::CreateActual();

    if (0 != m_editListenerP)
        m_editListenerP->NotifyOfPublicEdit();
    }

Time IDTMSource::Impl::GetLastModified () const
    {
    return m_lastModified;
    }

Time IDTMSource::Impl::GetLastModifiedCheckTime  () const
    {
    assert(!"Implement!");
    return CreateUnknownModificationTime();
    }

void IDTMSource::Impl::SetLastModified (const Time& time)
    {
    m_lastModified = time;
    }

void IDTMSource::Impl::ResetLastModified ()
    {
    m_lastModified = CreateUnknownModificationTime();
    }


IDTMLocalFileSource::IDTMLocalFileSource (Impl* implP)
    :   IDTMSource(implP),
        m_impl(*implP)
    {

    }


IDTMLocalFileSource::~IDTMLocalFileSource ()
    {

    }

void IDTMLocalFileSource::_Accept (IDTMSourceVisitor& visitor) const
    {
    visitor._Visit(*this);
    }

IDTMSource* IDTMLocalFileSource::_Clone () const 
    {
    return new IDTMLocalFileSource(new Impl(m_impl));
    }

LocalFileURL IDTMLocalFileSource::GetURL () const
    {
    StatusInt defaultStatus;
    return m_impl.GetURL(defaultStatus);
    }

LocalFileURL IDTMLocalFileSource::GetURL (StatusInt& status) const
    {
    return m_impl.GetURL(status);
    }

const WChar* IDTMLocalFileSource::GetPath () const
    {
    static StatusInt STATUS;
    return m_impl.GetPath(STATUS).c_str();
    }

const WChar* IDTMLocalFileSource::GetPath (StatusInt& status) const
    {
    return m_impl.GetPath(status).c_str();
    }

/*----------------------------------------------------------------------------+
|static IDTMLocalFileSource::Create
+----------------------------------------------------------------------------*/
IDTMLocalFileSourcePtr IDTMLocalFileSource::Create (DTMSourceDataType           sourceDataType, 
                                                            const ILocalFileMonikerPtr& localFileMonikerPtr)
    {
    return new IDTMLocalFileSource(new Impl(sourceDataType, localFileMonikerPtr.get())); 
    }



/*----------------------------------------------------------------------------+
|DTMLocalFileSource class
+----------------------------------------------------------------------------*/                
IDTMLocalFileSource::Impl::Impl(DTMSourceDataType   sourceDataType, 
                                    const IMoniker*     monikerP)
    :   IDTMSource::Impl(sourceDataType, monikerP),
        m_fileFound(false)
    {    
    }

IDTMLocalFileSource::Impl::~Impl()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileURL IDTMLocalFileSource::Impl::GetURL (StatusInt& status) const
    {
    assert(HasMoniker() && (dynamic_cast<const ILocalFileMoniker*>(&GetMoniker()) != 0));
    return static_cast<const ILocalFileMoniker&>(GetMoniker()).GetURL(status);
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& IDTMLocalFileSource::Impl::GetPath (StatusInt& status) const
    {
    status = BSISUCCESS;

    if (!m_fileFound)
        {
        const LocalFileURL url(GetURL(status));

        m_localURL.assign(url.GetPath());
        if (BSISUCCESS == status)
            {
            m_fileFound = true;
            }
        }

    return m_localURL;
    }



/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IDTMLocalFileSource::Impl::_UpdateLastModified ()
    {
    StatusInt status = BSISUCCESS;
        
    const WString& path = GetPath(status);

    SetLastModified(BSISUCCESS == status ? GetFileLastModificationTimeFor(path.c_str()) : CreateUnknownModificationTime());
    return status;
    }


IDTMDgnModelSource::IDTMDgnModelSource (Impl* implP)
    :   IDTMLocalFileSource(implP),
        m_impl(*implP)
    {

    }


IDTMDgnModelSource::~IDTMDgnModelSource ()
    {

    }

void IDTMDgnModelSource::_Accept (IDTMSourceVisitor& visitor) const
    {
    visitor._Visit(*this);
    }

IDTMSource* IDTMDgnModelSource::_Clone () const 
    {
    return new IDTMDgnModelSource(new Impl(m_impl));
    }

uint32_t IDTMDgnModelSource::GetModelID () const
    {
    return m_impl.GetModelID();
    }

const WChar* IDTMDgnModelSource::GetModelName () const
    {
    return m_impl.GetModelName().c_str();
    }

void IDTMDgnModelSource::UpdateModelName (const WChar* name) const
    {
    assert(!"TODO");
    }



/*----------------------------------------------------------------------------+
|DTMDgnSource class
+----------------------------------------------------------------------------*/                
IDTMDgnModelSource::Impl::Impl (DTMSourceDataType               sourceDataType, 
                                const IMoniker*                 monikerP,                                                  
                                uint32_t                          modelID, 
                                const WChar*                  modelName)
    :   IDTMLocalFileSource::Impl(sourceDataType, monikerP),
        m_modelID(modelID),
        m_modelName(modelName)
    {    
    }

IDTMDgnModelSource::Impl::~Impl()
    {
    }






IDTMDgnReferenceSource::IDTMDgnReferenceSource (Impl* implP)
    :   IDTMDgnModelSource(implP),
        m_impl(*implP)
    {

    }


IDTMDgnReferenceSource::~IDTMDgnReferenceSource ()
    {

    }

void IDTMDgnReferenceSource::_Accept (IDTMSourceVisitor& visitor) const
    {
    visitor._Visit(*this);
    }

IDTMSource* IDTMDgnReferenceSource::_Clone () const 
    {
    return new IDTMDgnReferenceSource(new Impl(m_impl));
    }

const WChar* IDTMDgnReferenceSource::GetRootToRefPersistentPath () const
    {
    return m_impl.GetRootToRefPersistentPath().c_str();
    }


const WChar* IDTMDgnReferenceSource::GetReferenceName () const
    {
    return m_impl.GetReferenceName().c_str();
    }

void IDTMDgnReferenceSource::UpdateReferenceName (const WChar* name) const
    {
    assert(!"TODO");
    }

const WChar* IDTMDgnReferenceSource::GetReferenceModelName  () const
    {
    return m_impl.GetReferenceModelName().c_str();
    }

void IDTMDgnReferenceSource::UpdateReferenceModelName (const WChar* name) const
    {
    assert(!"TODO");
    }


IDTMDgnReferenceSource::Impl::Impl (DTMSourceDataType               sourceDataType, 
                                    const IMoniker*                 monikerP,                                                  
                                    uint32_t                          modelID, 
                                    const WChar*                  modelName,
                                    const WChar*                     rootToRefPersistentPath,
                                    const WChar*                  referenceName,
                                    const WChar*                  referenceModelName)
    :   IDTMDgnModelSource::Impl(sourceDataType, monikerP, modelID, modelName),
        m_rootToRefPersistentPath(rootToRefPersistentPath),
        m_referenceName(referenceName),
        m_referenceModelName(referenceModelName)
    {    
    }

IDTMDgnReferenceSource::Impl::~Impl()
    {
    }






IDTMDgnLevelSourcePtr IDTMDgnLevelSource::Create   (DTMSourceDataType           sourceDataType, 
                                                    const ILocalFileMonikerPtr& dgnFileMonikerPtr,
                                                    uint32_t                      modelID, 
                                                    const WChar*              modelName,
                                                    uint32_t                      levelID,
                                                    const WChar*              levelName)
    {
    return new IDTMDgnLevelSource(new Impl(sourceDataType, 
                                           dgnFileMonikerPtr.get(), 
                                           modelID, 
                                           modelName, 
                                           levelID, 
                                           levelName));
    }


IDTMDgnLevelSource::IDTMDgnLevelSource (Impl* implP)
    :   IDTMDgnModelSource(implP),
        m_impl(*implP)
    {

    }


IDTMDgnLevelSource::~IDTMDgnLevelSource ()
    {

    }


uint32_t IDTMDgnLevelSource::GetLevelID () const
    {
    return m_impl.GetLevelID();
    }

const WChar* IDTMDgnLevelSource::GetLevelName () const
    {
    return m_impl.GetLevelName().c_str();
    }


void IDTMDgnLevelSource::UpdateLevelName (const WChar* name) const
    {
    assert(!"TODO");
    }

void IDTMDgnLevelSource::_Accept (IDTMSourceVisitor& visitor) const
    {
    visitor._Visit(*this);
    }

IDTMSource* IDTMDgnLevelSource::_Clone () const 
    {
    return new IDTMDgnLevelSource(new Impl(m_impl));
    }

/*----------------------------------------------------------------------------+
|DTMDgnLevelSource class
+----------------------------------------------------------------------------*/                
IDTMDgnLevelSource::Impl::Impl         (DTMSourceDataType               sourceDataType, 
                                        const IMoniker*                 monikerP,                                                       
                                        uint32_t                          modelID, 
                                        const WChar*                  modelName,
                                        uint32_t                          levelID,
                                        const WChar*                  levelName)
    :   IDTMDgnModelSource::Impl(sourceDataType, monikerP, modelID, modelName),
        m_levelID(levelID),
        m_levelName(levelName)
    {    
    }

IDTMDgnLevelSource::Impl::~Impl()
    {
    }





IDTMDgnReferenceLevelSourcePtr IDTMDgnReferenceLevelSource::Create (DTMSourceDataType           sourceDataType, 
                                                                    const ILocalFileMonikerPtr& rootDgnFileMonikerPtr,
                                                                    uint32_t                      rootModelID, 
                                                                    const WChar*              rootModelName,
                                                                    const WChar*                 rootToRefPersistentPath,
                                                                    const WChar*              referenceName,
                                                                    const WChar*              referenceModelName,
                                                                    uint32_t                      referenceLevelID,
                                                                    const WChar*              referenceLevelName)
    {
    assert(0 != rootToRefPersistentPath);
    return new IDTMDgnReferenceLevelSource(new Impl(sourceDataType, 
                                                    rootDgnFileMonikerPtr.get(), 
                                                    rootModelID, 
                                                    rootModelName, 
                                                    rootToRefPersistentPath, 
                                                    referenceName,
                                                    referenceModelName,
                                                    referenceLevelID, 
                                                    referenceLevelName));
    }



IDTMDgnReferenceLevelSource::IDTMDgnReferenceLevelSource (Impl* implP)
    :   IDTMDgnReferenceSource(implP),
        m_impl(*implP)
    {

    }


IDTMDgnReferenceLevelSource::~IDTMDgnReferenceLevelSource ()
    {

    }


uint32_t IDTMDgnReferenceLevelSource::GetLevelID () const
    {
    return m_impl.GetLevelID();
    }

const WChar* IDTMDgnReferenceLevelSource::GetLevelName () const
    {
    return m_impl.GetLevelName().c_str();
    }


void IDTMDgnReferenceLevelSource::UpdateLevelName (const WChar* name) const
    {
    assert(!"TODO");
    }

void IDTMDgnReferenceLevelSource::_Accept (IDTMSourceVisitor& visitor) const
    {
    visitor._Visit(*this);
    }

IDTMSource* IDTMDgnReferenceLevelSource::_Clone () const 
    {
    return new IDTMDgnReferenceLevelSource(new Impl(m_impl));
    }


/*----------------------------------------------------------------------------+
|DTMDgnLevelSource class
+----------------------------------------------------------------------------*/                
IDTMDgnReferenceLevelSource::Impl::Impl(DTMSourceDataType               sourceDataType, 
                                        const IMoniker*                 monikerP,                                                       
                                        uint32_t                          modelID, 
                                        const WChar*                  modelName,
                                        const WChar*                     rootToRefPersistentPath,
                                        const WChar*                  referenceName,
                                        const WChar*                  referenceModelName,
                                        uint32_t                          levelID,
                                        const WChar*                  levelName)
    :   IDTMDgnReferenceSource::Impl(sourceDataType, monikerP, modelID, modelName, rootToRefPersistentPath, referenceName, referenceModelName),
        m_levelID(levelID),
        m_levelName(levelName)
    {    
    }

IDTMDgnReferenceLevelSource::Impl::~Impl()
    {
    }




/*----------------------------------------------------------------------------+
|IDTMSourceGroup::Create
+----------------------------------------------------------------------------*/
IDTMSourceGroupPtr IDTMSourceGroup::Create()
    {
    return new IDTMSourceGroup(new Impl); 
    }




IDTMSourceGroup::IDTMSourceGroup (Impl* implP)
    :   IDTMSource(implP),
        m_impl(*implP)
    {

    }


IDTMSourceGroup::~IDTMSourceGroup ()
    {

    }

void IDTMSourceGroup::_Accept (IDTMSourceVisitor& visitor) const
    {
    visitor._Visit(*this);
    }

IDTMSource* IDTMSourceGroup::_Clone () const 
    {
    return new IDTMSourceGroup(new Impl(m_impl));
    }


const IDTMSourceCollection& IDTMSourceGroup::GetSources () const
    {
    return m_impl.GetSources();
    }

IDTMSourceCollection& IDTMSourceGroup::EditSources ()
    {
    return m_impl.GetSources();
    }

/*----------------------------------------------------------------------------+
|DTMSourceGroup class
+----------------------------------------------------------------------------*/                        
IDTMSourceGroup::Impl::Impl() 
    :   IDTMSource::Impl(DTM_SOURCE_DATA_MIX, 0)
    {
    m_sources.RegisterEditListener(*this);
    }


IDTMSourceGroup::Impl::~Impl()
    {
    m_sources.UnregisterEditListener(*this);
    }
        

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IDTMSourceGroup::Impl::_IsReachable () const
    {
    // Reachable only if all sources are reachable
    return m_sources.End() == std::find_if(m_sources.Begin(), m_sources.End(), not1(mem_fun_ref(&IDTMSource::IsReachable)));
    }

/*---------------------------------------------------------------------------------**//**
* @description 
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IDTMSourceGroup::Impl::_UpdateLastModified ()
    {
    StatusInt status = BSISUCCESS;

    Time lastModified = Time::CreateSmallestPossible();

    for (IDTMSourceCollection::iterator sourceIt = m_sources.BeginEditInternal(), sourceEnd = m_sources.EndEditInternal(); sourceIt != sourceEnd; ++sourceIt)
        {
        StatusInt sourceStatus = sourceIt->InternalUpdateLastModified();

        if (BSISUCCESS != sourceStatus)
            {
            status = BSIERROR;
            lastModified = CreateUnknownModificationTime();
            }
        else
            {
            lastModified = (std::max) (lastModified, sourceIt->GetLastModified());
            }
        }

    SetLastModified(lastModified);

    return status;
    }



END_BENTLEY_SCALABLEMESH_NAMESPACE
