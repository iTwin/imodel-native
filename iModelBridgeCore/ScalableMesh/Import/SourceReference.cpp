/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/SourceReference.cpp $
|    $RCSfile: SourceReference.cpp,v $
|   $Revision: 1.11 $
|       $Date: 2011/10/21 17:32:16 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/SourceReference.h>
#include <ScalableMesh/Import/SourceReferenceVisitor.h>

#include <ScalableMesh/Import/Plugin/SourceReferenceV0.h>

#include <STMInternal/Foundations/PrivateStringTools.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BaseT>
typename SourceRefMixinBase<BaseT>::ClassID  SourceRefMixinBase<BaseT>::s_GetClassID ()
    {
    static ClassID CLASS_ID = &typeid(UniqueTokenType());
    return CLASS_ID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BaseT>
SourceRefMixinBase<BaseT>::SourceRefMixinBase () 
    {
    }

template<typename BaseT>
SourceRefMixinBase<BaseT>::SourceRefMixinBase(const SourceRefMixinBase& rhs)
    : SourceRefBase(rhs)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BaseT>
SourceRefMixinBase<BaseT>::~SourceRefMixinBase () 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BaseT>
typename SourceRefMixinBase<BaseT>::ClassID SourceRefMixinBase<BaseT>::_GetClassID () const
    {
    return s_GetClassID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BaseT>
void SourceRefMixinBase<BaseT>::_Accept (ISourceRefVisitor& visitor) const
    {
    visitor._Visit(static_cast<const BaseT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BaseT>
SourceRefBase* SourceRefMixinBase<BaseT>::_Clone () const
    {
    return new BaseT(static_cast<const BaseT&>(*this));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef::SourceRef (const SourceRefBase* baseP)
    :   m_basePtr(const_cast<SourceRefBase*>(baseP)),
        m_classID(baseP->_GetClassID())
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef::SourceRef (const SourceRefBase& sourceRef)
    :   m_basePtr(sourceRef._Clone()),
        m_classID(sourceRef._GetClassID())        
    {        
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef::~SourceRef ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef::SourceRef (const SourceRef& rhs)
    :   m_basePtr(rhs.m_basePtr),
        m_classID(rhs.m_classID)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef& SourceRef::operator= (const SourceRef& rhs)
    {
    m_basePtr = rhs.m_basePtr;
    m_classID = rhs.m_classID;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceRef::Accept (ISourceRefVisitor& pi_rVisitor) const
    {
    m_basePtr->_Accept(pi_rVisitor);
    }

void SourceRef::SetDtmSource(const IDTMSourcePtr&        dtmSourcePtr)
    {
    m_basePtr->SetDtmSource(dtmSourcePtr);
    }

const IDTMSourcePtr SourceRef::GetDtmSource() const
    {
    return m_basePtr->GetDtmSource();
    }

struct SourceRefBase::Impl
    {
    IDTMSourcePtr               m_dtmSourcePtr;

    explicit                    Impl                   (const IDTMSourcePtr dtmSourcePtr) : m_dtmSourcePtr(dtmSourcePtr) {}
    Impl(){}
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRefBase::SourceRefBase ()
    :   m_pImpl(new Impl())
    {
    }

SourceRefBase::SourceRefBase(const SourceRefBase& rhs)
    {
    if(rhs.m_pImpl.get())
        if(rhs.m_pImpl->m_dtmSourcePtr != 0)
            m_pImpl.reset(new Impl(rhs.m_pImpl->m_dtmSourcePtr->Clone()));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRefBase::~SourceRefBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef SourceRefBase::CreateFromBase (const SourceRefBase* pi_sourceRefP)
    {
    return SourceRef(pi_sourceRefP);
    }

void SourceRefBase::SetDtmSource(const IDTMSourcePtr& dtmSourcePtr)
    {
    if(!m_pImpl.get())
        m_pImpl.reset(new Impl());
    m_pImpl->m_dtmSourcePtr = dtmSourcePtr;
    }

const IDTMSourcePtr SourceRefBase::GetDtmSource() const
    {
    return m_pImpl->m_dtmSourcePtr;
    }

LocalFileSourceRef::ClassID LocalFileSourceRef::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileSourceRef::Impl
    {
    WString                  m_path;

    explicit                    Impl                   (const WChar* pi_rPath) : m_path(pi_rPath) {}
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef LocalFileSourceRef::CreateFrom (const WChar* pi_rPath)
    {
    return CreateFromBase(new LocalFileSourceRef(pi_rPath));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceRef::LocalFileSourceRef (const WChar* path)
    :   m_pImpl(new Impl(path))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceRef::LocalFileSourceRef (const LocalFileSourceRef& rhs)
    : SourceRefMixinBase<LocalFileSourceRef>(rhs),
    m_pImpl(new Impl(*rhs.m_pImpl))  
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileSourceRef::~LocalFileSourceRef ()
    {
    
    }   

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& LocalFileSourceRef::GetPath () const
    {
    return m_pImpl->m_path;
    }   

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* LocalFileSourceRef::GetPathCStr () const
    {
    return m_pImpl->m_path.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalFileSourceRef::HasExtension (const WString& pi_rExtension) const
    {
    return equal(pi_rExtension.rbegin(), pi_rExtension.rend(), m_pImpl->m_path.rbegin(), 
                 GetDefaultCaseInsensitiveCharTools<WChar>().GetEqualTo());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LocalFileSourceRef::HasExtension (const WChar* pi_rExtension) const
    {
    return HasExtension(WString(pi_rExtension));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator==    (const LocalFileSourceRef&   lhs,
                    const LocalFileSourceRef&   rhs)
    {
    const WString& lPath = lhs.m_pImpl->m_path;
    const WString& rPath = rhs.m_pImpl->m_path;

    return std::equal(lPath.begin(), lPath.end(), rPath.begin(), 
                      GetDefaultCaseInsensitiveCharTools<WChar>().GetEqualTo());
    }


DGNLevelByNameSourceRef::ClassID DGNLevelByNameSourceRef::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNLevelByNameSourceRef::Impl
    {
    WString                  m_path;
    WString                  m_modelName;
    WString                  m_levelName;

    explicit                    Impl                   (const WChar*     path,
                                                        const WChar*     modelName,
                                                        const WChar*     levelName) 
        :   m_path(path), m_modelName(modelName), m_levelName(levelName) {}

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef DGNLevelByNameSourceRef::CreateFrom    (const WChar*      path,
                                            const WChar*      modelName,
                                            const WChar*      levelName)

    {
    return CreateFromBase(new DGNLevelByNameSourceRef(path, modelName ,levelName));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNLevelByNameSourceRef::DGNLevelByNameSourceRef   (const WChar*  dgnPath,
                                        const WChar*  modelName,
                                        const WChar*  levelName)
    :   m_pImpl(new Impl(dgnPath, modelName, levelName))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNLevelByNameSourceRef::DGNLevelByNameSourceRef (const DGNLevelByNameSourceRef& rhs)
    :   m_pImpl(new Impl(*rhs.m_pImpl))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNLevelByNameSourceRef::~DGNLevelByNameSourceRef ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNLevelByNameSourceRef::GetDGNPath () const
    {
    return m_pImpl->m_path;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNLevelByNameSourceRef::GetDGNPathCStr () const
    {
    return m_pImpl->m_path.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNLevelByNameSourceRef::GetModelName () const
    {
    return m_pImpl->m_modelName;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNLevelByNameSourceRef::GetModelNameCStr () const
    {
    return m_pImpl->m_modelName.c_str();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNLevelByNameSourceRef::GetLevelName () const
    {
    return m_pImpl->m_levelName;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNLevelByNameSourceRef::GetLevelNameCStr () const
    {
    return m_pImpl->m_levelName.c_str();
    }

DGNReferenceLevelByNameSourceRef::ClassID DGNReferenceLevelByNameSourceRef::s_GetClassID () { return super_class::s_GetClassID(); }

struct DGNReferenceLevelByNameSourceRef::Impl
    {
    WString                  m_path;
    WString                  m_rootModelName;
    WString                 m_rootToRefPersistantPath;
    WString                  m_levelName;

    explicit                    Impl                   (const WChar*     path,
                                                        const WChar*     rootModelName,
                                                        const WChar*    rootToRefPersistentPath,
                                                        const WChar*     referenceLevelName) 
        :   m_path(path), m_rootModelName(rootModelName), m_rootToRefPersistantPath(rootToRefPersistentPath), m_levelName(referenceLevelName) {}
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef DGNReferenceLevelByNameSourceRef::CreateFrom (const WChar*  dgnPath,
                                                        const WChar*  rootModelName,
                                                        const WChar*     rootToRefPersistentPath,
                                                        const WChar*  referenceLevelName)
    {
    return CreateFromBase(new DGNReferenceLevelByNameSourceRef(dgnPath, rootModelName ,rootToRefPersistentPath, referenceLevelName));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNReferenceLevelByNameSourceRef::DGNReferenceLevelByNameSourceRef (const WChar*  dgnPath,
                                                                    const WChar*  rootModelName,
                                                                    const WChar*   rootToRefPersistentPath,
                                                                    const WChar*  referenceLevelName)
    :   m_pImpl(new Impl(dgnPath, 
                         rootModelName, 
                         rootToRefPersistentPath,
                         referenceLevelName))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNReferenceLevelByNameSourceRef::DGNReferenceLevelByNameSourceRef (const DGNReferenceLevelByNameSourceRef& rhs)
    :   m_pImpl(new Impl(*rhs.m_pImpl))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNReferenceLevelByNameSourceRef::~DGNReferenceLevelByNameSourceRef ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNReferenceLevelByNameSourceRef::GetDGNPath () const
    {
    return m_pImpl->m_path;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNReferenceLevelByNameSourceRef::GetDGNPathCStr () const
    {
    return m_pImpl->m_path.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNReferenceLevelByNameSourceRef::GetRootModelName () const
    {
    return m_pImpl->m_rootModelName;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNReferenceLevelByNameSourceRef::GetRootModelNameCStr () const
    {
    return m_pImpl->m_rootModelName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNReferenceLevelByNameSourceRef::GetRootToRefPersistentPath () const
    {
    return m_pImpl->m_rootToRefPersistantPath;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNReferenceLevelByNameSourceRef::GetRootToRefPersistentPathCStr () const
    {
    return m_pImpl->m_rootToRefPersistantPath.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNReferenceLevelByNameSourceRef::GetLevelName () const
    {
    return m_pImpl->m_levelName;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNReferenceLevelByNameSourceRef::GetLevelNameCStr () const
    {
    return m_pImpl->m_levelName.c_str();
    }


DGNLevelByIDSourceRef::ClassID DGNLevelByIDSourceRef::s_GetClassID () { return super_class::s_GetClassID(); }

struct DGNLevelByIDSourceRef::Impl
    {
    WString                  m_path;
    uint32_t                      m_modelID;
    uint32_t                      m_levelID;

    explicit                    Impl                   (const WChar*     path,
                                                        uint32_t              modelID,
                                                        uint32_t              levelID) 
        :   m_path(path), m_modelID(modelID), m_levelID(levelID) {}
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef DGNLevelByIDSourceRef::CreateFrom(const WChar*  dgnPath,
                                            uint32_t          modelID,
                                            uint32_t          levelID)
    {
    return CreateFromBase(new DGNLevelByIDSourceRef(dgnPath, modelID ,levelID));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNLevelByIDSourceRef::DGNLevelByIDSourceRef   (const WChar*  dgnPath,
                                                uint32_t          modelID,
                                                uint32_t          levelID)
    :   m_pImpl(new Impl(dgnPath, 
                         modelID, 
                         levelID))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNLevelByIDSourceRef::DGNLevelByIDSourceRef (const DGNLevelByIDSourceRef& rhs)
    :   m_pImpl(new Impl(*rhs.m_pImpl))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNLevelByIDSourceRef::~DGNLevelByIDSourceRef ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNLevelByIDSourceRef::GetDGNPath () const
    {
    return m_pImpl->m_path;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNLevelByIDSourceRef::GetDGNPathCStr () const
    {
    return m_pImpl->m_path.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DGNLevelByIDSourceRef::GetModelID () const
    {
    return m_pImpl->m_modelID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DGNLevelByIDSourceRef::GetLevelID () const
    {
    return m_pImpl->m_levelID;
    }


DGNReferenceLevelByIDSourceRef::ClassID DGNReferenceLevelByIDSourceRef::s_GetClassID () { return super_class::s_GetClassID(); }

struct DGNReferenceLevelByIDSourceRef::Impl
    {
    WString                  m_path;
    uint32_t                      m_rootModelID;
    WString                 m_rootToRefPersistantPath;
    uint32_t                      m_levelID;

    explicit                    Impl                   (const WChar*     path,
                                                        uint32_t              rootModelID,
                                                        const WChar*         rootToRefPersistentPath,
                                                        uint32_t              referenceLevelID) 
        :   m_path(path), m_rootModelID(rootModelID), m_rootToRefPersistantPath(rootToRefPersistentPath), m_levelID(referenceLevelID) {}
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef DGNReferenceLevelByIDSourceRef::CreateFrom   (const WChar*  dgnPath,
                                                        uint32_t          rootModelID,
                                                        const WChar*     rootToRefPersistentPath,
                                                        uint32_t          referenceLevelID)
    {
    return CreateFromBase(new DGNReferenceLevelByIDSourceRef(dgnPath, rootModelID, rootToRefPersistentPath, referenceLevelID));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNReferenceLevelByIDSourceRef::DGNReferenceLevelByIDSourceRef (const WChar*  dgnPath,
                                                                uint32_t          rootModelID,
                                                                const WChar*     rootToRefPersistentPath,
                                                                uint32_t          referenceLevelID)
    :   m_pImpl(new Impl(dgnPath, 
                         rootModelID, 
                         rootToRefPersistentPath,
                         referenceLevelID))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNReferenceLevelByIDSourceRef::DGNReferenceLevelByIDSourceRef (const DGNReferenceLevelByIDSourceRef& rhs)
    :   m_pImpl(new Impl(*rhs.m_pImpl))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNReferenceLevelByIDSourceRef::~DGNReferenceLevelByIDSourceRef ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNReferenceLevelByIDSourceRef::GetDGNPath () const
    {
    return m_pImpl->m_path;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNReferenceLevelByIDSourceRef::GetDGNPathCStr () const
    {
    return m_pImpl->m_path.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DGNReferenceLevelByIDSourceRef::GetRootModelID () const
    {
    return m_pImpl->m_rootModelID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DGNReferenceLevelByIDSourceRef::GetRootToRefPersistentPath () const
    {
    return m_pImpl->m_rootToRefPersistantPath;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DGNReferenceLevelByIDSourceRef::GetRootToRefPersistentPathCStr () const
    {
    return m_pImpl->m_rootToRefPersistantPath.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t DGNReferenceLevelByIDSourceRef::GetLevelID () const
    {
    return m_pImpl->m_levelID;
    }


DGNElementSourceRef::ClassID DGNElementSourceRef::s_GetClassID () { return super_class::s_GetClassID(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceRef DGNElementSourceRef::CreateFrom  (Base* baseP)
    {
    return CreateFromBase(new DGNElementSourceRef(baseP));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElementSourceRef::DGNElementSourceRef (Base* baseP)
    :   m_baseP(baseP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElementSourceRef::DGNElementSourceRef (const DGNElementSourceRef& rhs)
    :   m_baseP(rhs.m_baseP->_Clone())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DGNElementSourceRef::~DGNElementSourceRef()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt DGNElementSourceRef::GetElementType () const
    {
    return m_baseP->_GetElementType();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt DGNElementSourceRef::GetElementHandlerID () const
    {
    return m_baseP->_GetElementHandlerID();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ElementReferenceP DGNElementSourceRef::GetElementRef () const
    {
    return m_baseP->_GetElementRef();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                Jean-Francois.Cote   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelReferenceP DGNElementSourceRef::GetModelRef() const
    {
    return m_baseP->_GetModelRef();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const LocalFileSourceRef* DGNElementSourceRef::GetLocalFileP () const
    {
    return m_baseP->_GetLocalFileP();
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
