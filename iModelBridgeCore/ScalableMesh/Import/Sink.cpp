/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include "Sink.h"

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Sink::Impl
    {
    ContentDescriptor           m_descriptor;
    bool                        m_isDescriptorUpToDate;
    size_t      m_expectedNSources;

    explicit                    Impl           ()
        :   m_descriptor(L""),
            m_isDescriptorUpToDate(false),
            m_expectedNSources(0)
        {
        
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Sink::Sink ()
    :   m_pImpl(new Impl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Sink::~Sink ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ContentDescriptor& Sink::GetDescriptor () const
    {
    if (!m_pImpl->m_isDescriptorUpToDate)
        {
        m_pImpl->m_descriptor = _CreateDescriptor();
        }

    return m_pImpl->m_descriptor;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BackInserterPtr Sink::CreateBackInserterFor(const PacketGroup&      src,
                                            uint32_t                    layerID,
                                            const DataType&         type,
                                            Log&             log) const
    {
    BackInserterPtr inserterPtr = _CreateBackInserterFor(layerID, type, log);
    if (0 == inserterPtr.get())
        return inserterPtr;

    inserterPtr->_Assign(src);
    return inserterPtr;
    }

void                       Sink::SetTotalNumberOfExpectedSources(size_t nSources)
    {
    m_pImpl->m_expectedNSources = nSources;
    }


size_t                       Sink::GetTotalNumberOfExpectedSources() const
    {
    return m_pImpl->m_expectedNSources;
    }
/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BackInserter::BackInserter ()    
    : m_is3dData(false)
    {        
    }

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BackInserter::~BackInserter ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Mathieu.St-Pierre  02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BackInserter::SetIs3dData(bool is3dData)
    {
    m_is3dData = is3dData;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
*
* @bsimethod                                                  Elenie.Godzaridis  11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BackInserter::SetIsGridData(bool isGridData)
    {
    m_isGridData = isGridData;
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
