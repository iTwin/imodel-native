/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMBinaryData.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "DTMBinaryData.h"
#include <DgnPlatform\TerrainModel\TMPersistentAppIDs.h>
//#include <interface/IXAttributeHandler.h>
//#include <interface/ITxnManager.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

#define XAttributeSize 65536
#define TXNMGR ITxnManager::GetCurrentTxn()

//=======================================================================================
// @bsiclass                                                     Sylvain.Pucci   09/05
//=======================================================================================
class BlobXAttributeAccessor
    {
    //=======================================================================================
    // Handler for my xattribute
    // @bsiclass                                                 Sylvain.Pucci   09/05
    //=======================================================================================
    class Handler : public XAttributeHandler
        {
        private:
            //  Implement Singleton pattern
            // don't allow instantiation outside of this class
            friend BlobXAttributeAccessor;
            Handler () {;}

            //=======================================================================================
            // @bsimethod                                                    Sylvain.Pucci   09/05
            //=======================================================================================
            //! Callback Invoked when the XAttribute is cloned
            //! @param replacer IN where to write the new version of the XAttribute
            //! @param xa   IN  the XAttribute that is being copied
            //! @param eh   IN  the element that holds the XAttribute
            //! @param cc   IN  the copy context
            //! @returns non-zero if XAttribute should \em not be copied
        public: StatusInt _OnPreprocessCopy
                    (
                    IReplaceXAttribute*         replacer,
                    XAttributeHandle const&     xa,
                    ElementHandle const&        eh,
                    ElementCopyContextP         cc
                    )
                    {
                    return SUCCESS;
                    }
        };

    //=======================================================================================
    // @bsimethod                                                    Sylvain.Pucci   09/05
    //=======================================================================================
    public: static XAttributeHandler* GetHandler()
                {
                // The single instance of BlobXAttributeAccessor.Handler is the XAttributes handler.
                return &s_handler;
                }

    public: static Handler  s_handler;

    }; // BlobXAttributeAccessor

BlobXAttributeAccessor::Handler   BlobXAttributeAccessor::s_handler;

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
void DTMBinaryData::Initialize()
    {
    XAttributeHandlerId handlerId2 (TMElementMajorId, XATTRIBUTES_SUBID_DTM_FEATUREARRAY);
    XAttributeHandlerManager::RegisterHandler(handlerId2, BlobXAttributeAccessor::GetHandler());
    XAttributeHandlerId handlerId3 (TMElementMajorId, XATTRIBUTES_SUBID_DTM_POINTARRAY);
    XAttributeHandlerManager::RegisterHandler(handlerId3, BlobXAttributeAccessor::GetHandler());
    XAttributeHandlerId handlerId4 (TMElementMajorId, XATTRIBUTES_SUBID_DTM_NODEARRAY);
    XAttributeHandlerManager::RegisterHandler(handlerId4, BlobXAttributeAccessor::GetHandler());
    XAttributeHandlerId handlerId5 (TMElementMajorId, XATTRIBUTES_SUBID_DTM_CLISTARRAY);
    XAttributeHandlerManager::RegisterHandler(handlerId5, BlobXAttributeAccessor::GetHandler());
    XAttributeHandlerId handlerId6 (TMElementMajorId, XATTRIBUTES_SUBID_DTM_FLISTARRAY);
    XAttributeHandlerManager::RegisterHandler(handlerId6, BlobXAttributeAccessor::GetHandler());
    XAttributeHandlerId handlerId8 (TMElementMajorId, XATTRIBUTES_SUBID_DTM_FEATURETABLEMAP);
    XAttributeHandlerManager::RegisterHandler(handlerId8, BlobXAttributeAccessor::GetHandler());
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
bool DTMBinaryData::HasBinaryData(ElementRefP elRef, UInt16 xAttrSubId, UInt32 xAttrId)
    {
    XAttributeHandlerId handlerId (TMElementMajorId, xAttrSubId);
    return XAttributeHandle::HasAttribute(elRef, handlerId, xAttrId);
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
bool DTMBinaryData::HasBinaryData(EditElementHandleR element, UInt16 xAttrSubId, UInt32 xAttrId)
    {
    XAttributeHandlerId handlerId (TMElementMajorId, xAttrSubId);
    EditElementHandle::XAttributeIter xattrIter = element.GetXAttributeIter();
    return xattrIter.Search (handlerId, xAttrId);
    }

//=======================================================================================
// @bsimethod                                                    Mathieu.St-Pierre   04/10
//=======================================================================================
StatusInt DTMBinaryData::ScheduleReadData(EditElementHandleP element, UInt16 xAttrSubId, UInt32 xAttrId, void* data, UInt32 offset, UInt32 dataSize)
    {
    XAttributeHandlerId handlerId (TMElementMajorId, xAttrSubId);
    ElementRefP elRef = element->GetElementRef();

    StatusInt status = ERROR;
    
    if (elRef)
        {
        XAttributeHandle it (elRef, handlerId, xAttrId);
          if (it.IsValid() && (it.GetSize() >= offset + dataSize))
            {
            memcpy(data, ((byte*)it.PeekData()) + offset, dataSize);

            status = SUCCESS;
            }
        }   

    return status;
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
void DTMBinaryData::ScheduleWriteData (EditElementHandleP element, UInt16 xAttrSubId, UInt32 xAttrId, void const* data, UInt32 offset, UInt32 dataSize)
    {
    XAttributeHandlerId handlerId (TMElementMajorId, xAttrSubId);
    ElementRefP elRef = element->GetElementRef();

    StatusInt status = SUCCESS;
    if (elRef)
        {
        XAttributeHandle it (elRef, handlerId, xAttrId);
        if (it.IsValid() == false)            
            {
            unsigned char* pTempBuffer = (unsigned char*)_alloca(XAttributeSize);
            memcpy(pTempBuffer + offset, data, dataSize); 
            status = ITxnManager::GetCurrentTxn().AddXAttribute(elRef, handlerId, xAttrId, pTempBuffer, XAttributeSize);
            }
        else
            {
            XAttributeHandle handle(elRef, handlerId, xAttrId);
            char* xaData = (char*)handle.GetPtrForWrite();
            memcpy(xaData + offset, data, dataSize);
            status = SUCCESS;
            }
        }
    else
        status = element->ScheduleWriteXAttribute (handlerId, xAttrId, dataSize, data);
    BeAssert (status == SUCCESS);
    }


//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
void DTMBinaryData::ScheduleWriteData (EditElementHandleP element, UInt16 xAttrSubId, UInt32 xAttrId, void const* data, UInt32 dataSize)
    {
    XAttributeHandlerId handlerId (TMElementMajorId, xAttrSubId);
    ElementRefP elRef = element->GetElementRef();

    StatusInt status = SUCCESS;
    if (elRef)
        {
        XAttributeHandle it (elRef, handlerId, xAttrId);
        if (it.IsValid())
            status = ITxnManager::GetCurrentTxn().ReplaceXAttributeData (it, data, dataSize);
        else
            status = ITxnManager::GetCurrentTxn().AddXAttribute(elRef, handlerId, xAttrId, data, dataSize);
        }
    else
        status = element->ScheduleWriteXAttribute (handlerId, xAttrId, dataSize, data);
    BeAssert (status == SUCCESS);
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
void DTMBinaryData::ScheduleDeleteData (EditElementHandleR element, UInt16 xAttrSubId, UInt32 xAttrId)
    {
    XAttributeHandlerId handlerId (TMElementMajorId, xAttrSubId);
    ElementRefP elRef = element.GetElementRef();

    StatusInt status = SUCCESS;
    if (elRef)
        {
            XAttributeHandle it (elRef, handlerId, xAttrId);
            if (it.IsValid())
                status = ITxnManager::GetCurrentTxn().DeleteXAttribute(it);
        }
    else
        status = element.ScheduleDeleteXAttribute(handlerId, xAttrId);
    BeAssert (status == SUCCESS);
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
DTMBinaryDataIter::DTMBinaryDataIter(ElementHandle const& eh, UInt16 xAttrSubId)
    {
    m_elRef = eh.GetElementRef();
    m_xAttrIter = new ElementHandle::XAttributeIter(eh);
    m_isReset = true;
    m_xAttrSubId = xAttrSubId;
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
DTMBinaryDataIter::~DTMBinaryDataIter()
    {
    if (m_xAttrIter)
        delete m_xAttrIter;
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
void DTMBinaryDataIter::Reset()
    {
    m_isReset = true;
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
bool DTMBinaryDataIter::MoveNext()
    {
    bool retValue = false;
    XAttributeHandlerId handlerId (TMElementMajorId, m_xAttrSubId);

    if (m_isReset)
        {
        m_isReset = false;
        retValue = m_xAttrIter->Search(handlerId);
        if (retValue)GetCurrentData();
        }
    else
        {
        retValue = m_xAttrIter->SearchNext (handlerId);
        if (retValue)GetCurrentData();
        }
    return retValue;
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
Int32 DTMBinaryDataIter::GetCurrentSize()
    {
    return m_xAttrIter->GetSize();
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
void* DTMBinaryDataIter::GetCurrentData()
    {
    return (char*)m_xAttrIter->PeekData();
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  05/08
//=======================================================================================
void* DTMBinaryDataIter::GetWritableData()
    {
    if (m_xAttrIter->GetElementXAttributeIter())
        {
        XAttributeHandle handle(m_elRef, m_xAttrIter->GetHandlerId(), m_xAttrIter->GetId());
        ITxnManager::GetCurrentTxn().SaveXAttributeDataForDirectChange(handle, 0, handle.GetSize());
        return (char*)handle.GetPtrForWrite();
        }
    return (char*)m_xAttrIter->PeekData();
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
UInt32 DTMBinaryDataIter::GetCurrentAttrId()
    {
    return m_xAttrIter->GetId();
    }

//=======================================================================================
// @bsimethod                                                    Sylvain.Pucci   09/05
//=======================================================================================
void DTMBinaryDataIter::SetxAttrSubId(UInt16 xAttrSubId)
    {
    m_xAttrSubId = xAttrSubId;
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
