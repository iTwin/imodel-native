/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"



#include <ScalableMesh/IScalableMeshMoniker.h>
#include <STMInternal/Foundations/PrivateStringTools.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*----------------------------------------------------------------------------+
|IMoniker::IMoniker
+----------------------------------------------------------------------------*/
IMoniker::IMoniker ()
    :   m_implP(0)
    {
    }

/*----------------------------------------------------------------------------+
|IMoniker::~IMoniker
+----------------------------------------------------------------------------*/
IMoniker::~IMoniker ()
    {
    }

/*----------------------------------------------------------------------------+
|IMoniker::GetType
+----------------------------------------------------------------------------*/
DTMSourceMonikerType IMoniker::GetType() const
    {
    return _GetType();
    }

/*----------------------------------------------------------------------------+
|IMoniker::IsReachable
+----------------------------------------------------------------------------*/
bool IMoniker::IsTargetReachable () const
    {
    return _IsTargetReachable();
    }

/*----------------------------------------------------------------------------+
|IMoniker::Serialize
+----------------------------------------------------------------------------*/
StatusInt IMoniker::Serialize(Import::SourceDataSQLite&        sourceData,
    const DocumentEnv&    env) const
{
    const byte monikerTypeField = static_cast<byte>(GetType());
    sourceData.SetMonikerType(monikerTypeField);
    /*if (!stream.put(monikerTypeField).good())
        return BSIERROR;*/

    // SM_NEEDS_WORK : just ignore it for now
    return _Serialize(sourceData, env);
    //return BSISUCCESS;
}


/*----------------------------------------------------------------------------+
|IMoniker::Accept
+----------------------------------------------------------------------------*/
void IMoniker::Accept (IMonikerVisitor& visitor) const
    {
    _Accept(visitor);
    }

/*----------------------------------------------------------------------------+
|ILocalFileMoniker::ILocalFileMoniker
+----------------------------------------------------------------------------*/
ILocalFileMoniker::ILocalFileMoniker ()
    :   m_implP(0)
    {
    }

/*----------------------------------------------------------------------------+
|ILocalFileMoniker::~ILocalFileMoniker
+----------------------------------------------------------------------------*/
ILocalFileMoniker::~ILocalFileMoniker ()
    {
    }

/*----------------------------------------------------------------------------+
|ILocalFileMoniker::_Accept
+----------------------------------------------------------------------------*/
void ILocalFileMoniker::_Accept (IMonikerVisitor& visitor) const
    {
    visitor._Visit(*this);
    }

/*----------------------------------------------------------------------------+
|ILocalFileMoniker::GetURL
+----------------------------------------------------------------------------*/
LocalFileURL ILocalFileMoniker::GetURL (StatusInt& status) const
    {
    return _GetURL(status);
    }

/*----------------------------------------------------------------------------+
|ILocalFileMoniker::GetURL
+----------------------------------------------------------------------------*/
LocalFileURL ILocalFileMoniker::GetURL () const
    {
    StatusInt defaultStatus;
    return _GetURL(defaultStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerCreator::ILocalFileMonikerCreator ()
    :   m_implP(0)
    {   
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerCreator::~ILocalFileMonikerCreator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ILocalFileMonikerFactory::Impl
    {
    explicit                            Impl                       () : m_creatorP(0) {}

    const ILocalFileMonikerCreator*     m_creatorP;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerFactory::ILocalFileMonikerFactory       ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerFactory::~ILocalFileMonikerFactory      ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerFactory& ILocalFileMonikerFactory::GetInstance ()
    {
    static ILocalFileMonikerFactory SINGLETON;
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerFactory::CreatorID ILocalFileMonikerFactory::Register (const ILocalFileMonikerCreator& creator)
    {
    if (0 != m_implP->m_creatorP)
        {
        assert(!"Creator already registered");
        return 0;
        }

    m_implP->m_creatorP = &creator;
    return m_implP->m_creatorP;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ILocalFileMonikerFactory::Unregister (CreatorID id)
    {
    assert(0 != id);

    if (m_implP->m_creatorP == id)
        m_implP->m_creatorP = 0;
    else
        {
        assert(!"Creator not registered!");
        }
    }  

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerPtr ILocalFileMonikerFactory::Create  (const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&  msMoniker) const
    {
    if (0 == m_implP->m_creatorP)
        return 0;

    StatusInt dummyStatus;
    return m_implP->m_creatorP->_Create(msMoniker, dummyStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalFileMonikerPtr ILocalFileMonikerFactory::Create  (const WChar*  fullPath) const
    {
    if (0 == m_implP->m_creatorP)
        return 0;

    StatusInt dummyStatus;
    return m_implP->m_creatorP->_Create(fullPath, dummyStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IMonikerBinStreamCreator::IMonikerBinStreamCreator()
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IMonikerBinStreamCreator::~IMonikerBinStreamCreator()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IMonikerFactory::Impl
    {
    typedef vector<const IMonikerBinStreamCreator*> BinStreamCreatorMap;
    BinStreamCreatorMap                             m_binStreamCreators;
    
    explicit                                        Impl                               ()
        :   m_binStreamCreators(DTM_SOURCE_MONIKER_QTY, 0)
        {
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IMonikerFactory::IMonikerFactory ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IMonikerFactory::~IMonikerFactory ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IMonikerFactory& IMonikerFactory::GetInstance ()
    {
    static IMonikerFactory SINGLETON;
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IMonikerFactory::BinStreamCreatorID IMonikerFactory::Register (const IMonikerBinStreamCreator& creator)
    {
    const DTMSourceMonikerType type = creator._GetSupportedType();
    assert(type < DTM_SOURCE_MONIKER_QTY);

    Impl::BinStreamCreatorMap::value_type& creatorP = m_implP->m_binStreamCreators[type];

    if (0 != creatorP)
        {
        assert(!"Creator already registered");
        return 0;
        }

    creatorP = &creator;
    return creatorP;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void IMonikerFactory::Unregister (BinStreamCreatorID id)
    {
    if (0 == id)
        {
        assert(!"Invalid creator");
        return;
        }

    const DTMSourceMonikerType type = id->_GetSupportedType();
    if (DTM_SOURCE_MONIKER_QTY <= type)
        {
        assert(!"Invalid moniker type");
        return;
        }

    Impl::BinStreamCreatorMap::value_type& creatorP = m_implP->m_binStreamCreators[type];
    assert(0 != creatorP);
    assert(id == creatorP);

    if (id == creatorP)
        creatorP = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IMonikerPtr IMonikerFactory::Create(Import::SourceDataSQLite&      sourceData,
    const DocumentEnv&  env)
{
    const uint32_t typeField = sourceData.GetMonikerType();
    if (typeField >= DTM_SOURCE_MONIKER_QTY)
        return 0;

    const DTMSourceMonikerType type = static_cast<DTMSourceMonikerType>(typeField);

    const Impl::BinStreamCreatorMap::value_type creatorP = m_implP->m_binStreamCreators[type];
    if (0 == creatorP)
        return 0;

    //stream.get(); // Skip moniker type

    StatusInt dummyStatus;
    return creatorP->_Create(sourceData, env, dummyStatus);
}


END_BENTLEY_SCALABLEMESH_NAMESPACE
