/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/Sink.cpp $
|    $RCSfile: Sink.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/08/26 18:46:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

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

    explicit                    Impl           ()
        :   m_descriptor(L""),
            m_isDescriptorUpToDate(false)
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
                                            UInt                    layerID,
                                            const DataType&         type,
                                            Log&             log) const
    {
    BackInserterPtr inserterPtr = _CreateBackInserterFor(layerID, type, log);
    if (0 == inserterPtr.get())
        return inserterPtr;

    inserterPtr->_Assign(src);
    return inserterPtr;
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

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
