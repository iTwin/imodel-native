/*--------------------------------------------------------------------------------------+                                                          
|
|     $Source: DgnCore/ProxyDisplayCore.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

typedef bvector<byte>                   T_ByteVector;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CachedVisibleEdgeHandlerId::CachedVisibleEdgeHandlerId () : XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_CachedVisibleEdgeHandler) { }
CachedVisibleEdgeCacheId::CachedVisibleEdgeCacheId () : XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_CachedVisibleEdgeCache) { }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewHandlerPass::operator == (ViewHandlerPassCR rhs) const
    {
    if (m_pass == rhs.m_pass)
        {
        if (ClipVolumePass::None == m_pass)
            return true;
        else
            return m_clipPlaneIndex == rhs.m_clipPlaneIndex;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewHandlerPass::operator < (ViewHandlerPassCR rhs) const
    {
    if (m_pass != rhs.m_pass)
        return m_pass < rhs.m_pass;

    if (ClipVolumePass::None == m_pass)
        return false;


    return m_clipPlaneIndex < rhs.m_clipPlaneIndex;
    }

ViewHandlerPass::ViewHandlerPass (ViewContextR viewContext)  { Init (viewContext);     }


///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool    ProxyDisplayPath::IsSamePath (ProxyDisplayPathCR other) const
//    {
//    if (m_rootModel != other.m_rootModel || m_path.size() != other.m_path.size())
//        return false;
//
//    for (size_t i=0; i<m_path.size(); i++)
//        if (m_path[i] != other.m_path[i])
//            return false;
//
//    return true;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      01/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyDisplayPath::ProxyDisplayPath (DisplayPath const& displayPath)
//    {
//    m_rootModel = displayPath.GetRoot();
//
//    for (int i=0, count = displayPath.GetCount(); i<count; i++)
//        m_path.push_back (displayPath.GetPathElem(i)->GetElementId());
//    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewHandlerPass::Init (ViewContextR viewContext)
    {
//    if (viewContext.GetDynamicViewStateStack().empty())
        m_pass = ClipVolumePass::None;
//    else
//        m_pass = viewContext.GetDynamicViewStateStack().back().GetPass();
//
//    ICutPlaneP          cutPlane;
//    if (ClipVolumePass::Cut == m_pass && NULL != (cutPlane = viewContext.GetCuttingPlane()))
//        m_clipPlaneIndex = cutPlane->_GetIndex();
//    else
        m_clipPlaneIndex = 0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProxyHLEdgeId::operator < (ProxyHLEdgeIdCR rhs) const
    {
    if (! (m_id == rhs.m_id))
        return m_id < rhs.m_id;

    return m_flags < rhs.m_flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProxyHLEdgeSegmentId::operator < (ProxyHLEdgeSegmentIdCR rhs) const
    {
    if (! (m_edgeId == rhs.m_edgeId))
        return m_edgeId < rhs.m_edgeId;

    if (m_startParam != rhs.m_startParam)
        return m_startParam < rhs.m_endParam;

    return m_endParam < rhs.m_endParam;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyGraphicsType   ProxyHLEdgeId::GetProxyGraphicsType() const
    {
    if (IsVisible())
        return IsEdge() ? ProxyGraphicsType_VisibleEdge : ProxyGraphicsType_VisibleWire;
    else
        return IsEdge() ? ProxyGraphicsType_HiddenEdge : ProxyGraphicsType_HiddenWire;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ProxyGraphicsFlags::ProxyGraphicsFlags (bool constructionClass, bool solidFillEdge, bool isPrecalculated) : m_value (0)
    {
    if (constructionClass)
        m_value |= ProxyGraphicsFlags_ConstructionClass;

    if (solidFillEdge)
        m_value |= ProxyGraphicsFlags_SolidFillEdge;

    if (isPrecalculated)
        m_value |= ProxyGraphicsFlags_Precalculated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
ProxyHLEdgeSegmentId::ProxyHLEdgeSegmentId (ProxyHLEdgeIdCR id, GPArrayIntervalCR interval) : m_edgeId (id)
    {
    m_startParam = (float) interval.m_start.GetValue();
    m_endParam   = (float) interval.m_end.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
WString ProxyHLEdgeSegmentId::ToString() const
    {
    return m_edgeId.ToString() + WPrintfString (L" Interval: (%f - %f)", m_startParam, m_endParam);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
void ProxyHLEdgeId::Init (void const* data, size_t dataBytes)
    {
    if (dataBytes < sizeof (m_flags))
        {
        BeAssert(false);
        return;
        }
    byte const*     pData = (byte*) data;

    memcpy (&m_flags, pData, sizeof (m_flags));

    m_id.Init (pData + sizeof(m_flags), dataBytes - sizeof (m_flags));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
void    ProxyHLEdgeId::Save (struct ProxyDataBuffer& buffer) const
    {
    buffer.Write (m_flags);

    T_ByteVector    packed;

    m_id.Pack (packed);

    if (!packed.empty())
        buffer.Write (&packed.at(0), packed.size());
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
WString ProxyHLEdgeId::ToString () const
    {
    return WPrintfString (L" ID: %ls Flags: %x\n", m_id.GetDebugString().c_str(), m_flags);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProxyHLEdgeId::IsAssociable () const
    {
    return !IsIntersection() && !IsGeometryMap();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
bool    ProxyEdgeIdData::Matches (void const* edgeData, size_t edgeDataBytes) const
    {
    return m_data.size() == edgeDataBytes && 0 == memcmp (edgeData, &m_data[0], edgeDataBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyEdgeIdData::Init (void const* edgeData, size_t edgeDataBytes)
    {
    m_data.resize (edgeDataBytes);
    memcpy (&m_data[0], edgeData, edgeDataBytes);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyHLEdgeSegmentId::Init (ProxyEdgeIdDataCR edgeId) { return Init (edgeId.GetData(), edgeId.GetSize()); }
StatusInt   ProxyHLEdgeSegmentId::Init (CurvePrimitiveIdCR id) { return InitFromCurveIdData (id.PeekId(), id.GetIdSize()); }
StatusInt   ProxyHLEdgeSegmentId::Init (void const* data, size_t nBytes)
    {
    CurvePrimitiveId::Type const*     edgeId = (CurvePrimitiveId::Type const*) data;

    if (*edgeId != CurvePrimitiveId::Type_CachedEdge)
       return ERROR;
    
    return InitFromCurveIdData ((byte const*) data + sizeof (CurvePrimitiveId::Type), nBytes - sizeof (CurvePrimitiveId::Type));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyHLEdgeSegmentId::InitFromCurveIdData (void const* data, size_t nBytes)
    {
    byte const*     pData    = (byte const*) data;
    byte const*     pDataEnd = (byte const*) data + nBytes;
    size_t          intervalBytes = *pData++;

    switch (intervalBytes)
        {
        case 0:
            m_startParam = 0.0f;
            m_endParam   = 1.0f;
            break;

        case 4:
            m_startParam = 0.0f;
            memcpy (&m_endParam, pData, sizeof (float));
            break;

        case 8:
            memcpy (&m_startParam, pData, sizeof (float));
            memcpy (&m_endParam, pData + sizeof (float), sizeof (float));
            break;
        }

    pData += intervalBytes;
    m_edgeId.Init (pData, pDataEnd  - pData);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
GPArrayInterval    ProxyHLEdgeSegmentId::GetInterval () const
    {
    return GPArrayInterval (GPArrayParam ((double) m_startParam), GPArrayParam ((double) m_endParam), NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
void    ProxyHLEdgeSegmentId::Save (ProxyEdgeIdDataR data) const
    {
    ProxyDataBuffer     buffer;

    buffer.Write ((byte) CurvePrimitiveId::Type_CachedEdge);
    // byte 0.  - Interval size
    if (0.0f == m_startParam)
        {
        if (1.0f == m_endParam)
            {
            buffer.Write ((byte) 0);
            }
        else
            {
            buffer.Write ((byte) sizeof (float));
            buffer.Write ((float) m_endParam);
            }
        }
    else
        {
        buffer.Write ((byte) (2 * sizeof (float)));
        buffer.Write ((float) m_startParam);
        buffer.Write ((float) m_endParam);
        }
    m_edgeId.Save (buffer);

    data.Init (buffer.GetData(), buffer.GetDataSize());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/201
+--------------+---------------+---------------+---------------+---------------+------*/
void            ProxyEdgeIdData::Store (DataExternalizer& sink) const
    {
    sink.put ((UInt32) m_data.size());
    sink.put (&m_data[0], m_data.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+--------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProxyEdgeIdData::Load (DataLoader& source)
    {
    UInt32      size;

    source.get (&size);
    m_data.resize (size);
    source.get (&m_data[0], size);

    return SUCCESS;
    }

typedef struct DisplayPath const& DisplayPathCR;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ProxyDataBuffer::Write (const void* data, size_t dataSize)
    {
    size_t      currentSize = m_buffer.size();

    m_buffer.resize (currentSize + dataSize);

    memcpy (&m_buffer[currentSize], data, dataSize);

    return currentSize;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ProxyDataBuffer::Write (WStringCR string)
    {
    size_t      currentSize = m_buffer.size();

    if (0 == string.size())
        {
        Write ((UInt32) 0);
        }
    else
        {
        Write ((UInt32) (1 + string.size()));
        Write (string.c_str(), string.size()*sizeof(WChar));
        Write ((WChar) 0);
        }

    return currentSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    ProxyDataBuffer::UpdateBytesToFollow (size_t location)
    {
    BeAssert (location < m_buffer.size());
    UInt32      size = ((UInt32) (m_buffer.size() - location)) - sizeof (UInt32);

    memcpy (&m_buffer[location], &size, sizeof (size));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyDataBuffer::SaveToXAttributes (ElementRefP elemRef, XAttributeHandlerId xAttrHandlerId, UInt32 xAttributeIndex)
    {
    XAttributeHandle        it (elemRef, xAttrHandlerId, xAttributeIndex);

    if (it.IsValid())
        return elemRef->GetDgnProject()->GetTxnManager().GetCurrentTxn().ReplaceXAttributeData (it, GetData(), (UInt32) GetDataSize());
    else
        return elemRef->GetDgnProject()->GetTxnManager().GetCurrentTxn().AddXAttribute (elemRef, xAttrHandlerId, xAttributeIndex, GetData(), (UInt32) GetDataSize());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyDataBuffer::SaveCompressedToXAttributes (ElementRefP elemRef, XAttributeHandlerId xAttrHandlerId, UInt32 xAttributeIndex)
    {
    XAttributeHandle        it (elemRef, xAttrHandlerId, xAttributeIndex);

    if (it.IsValid ())
        return  CompressedXAttribute::Replace (it, GetData(), GetDataSize(), COMPRESSION_THRESHOLD_MinimumAccess);
    else
        return  CompressedXAttribute::Add (elemRef, xAttrHandlerId, xAttributeIndex,  GetData(), GetDataSize(), COMPRESSION_THRESHOLD_MinimumAccess, NULL);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//DgnModelP          ProxyDisplayCacheBase::GetParentDgnModelP () const
//    {
//    return (NULL == m_rootModel) ? NULL : m_rootModel->GetParentDgnModelP();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//DgnProjectP          ProxyDisplayCacheBase::GetParentDgnFile () const
//    {
//    DgnModelP        parentModel;
//
//    return (NULL == (parentModel = GetParentDgnModelP())) ? NULL : parentModel->GetDgnProject();
//    }
//
//static ProxyDgnAttachmentHandlerManager       s_proxyDgnAttachmentHandlerManager;
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley       10/2010
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyDgnAttachmentHandlerManagerR ProxyDgnAttachmentHandlerManager::GetManager ()                                           { return s_proxyDgnAttachmentHandlerManager; }
//void ProxyDgnAttachmentHandlerManager::RegisterHandler (ProxyDgnAttachmentHandlerCR handler, XAttributeHandlerId id)        { m_map[id] = &handler; }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley       10/2010
//+---------------+---------------+---------------+---------------+---------------+------*/
//ProxyDgnAttachmentHandlerCP  ProxyDgnAttachmentHandlerManager::GetHandler (XAttributeHandlerId id)
//    {
//    T_ProxyDgnAttachmentHandlerMap::iterator   found;
//    return ((found = m_map.find (id)) == m_map.end()) ? NULL : found->second;
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Barry.Bentley                   08/11
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool            ProxyDisplayCacheBase::IsValidForViewport (ViewportR viewport) const
//    {
//    return  ProxyCacheStatus::UpToDate == this->GetCacheStatusForViewport (&viewport);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      08/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool            ProxyDisplayCacheBase::UsableForViewport (ViewportR viewport) const 
//    {
//    switch (GetCacheStatusForViewport(&viewport))
//        {
//        case ProxyCacheStatus::NotCached:
//        case ProxyCacheStatus::ObsoleteVersion:
//        case ProxyCacheStatus::NotUsedInView:
//            return false;
//
//        case ProxyCacheStatus::UpToDate:
//            return true;
//
//        default:
//        case ProxyCacheStatus::ModelChanged:
//        case ProxyCacheStatus::ViewChanged:
//        case ProxyCacheStatus::AttachmentChanged:
//            return true;
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+--------------+---------------+---------------+---------------+---------------+------*/
//bool ProxyDisplayCacheBase::IsValidForViewContext (ViewContextR viewContext, bool isTransientDisplay) // Static.
//    {
//    switch (viewContext.GetDrawPurpose())
//        {
//        case DrawPurpose::CutXGraphicsCreate:
//        case DrawPurpose::InterferenceDetection:
//        case DrawPurpose::VisibilityCalculation:
//        case DrawPurpose::RangeCalculation:
//        case DrawPurpose::ComputeDgnModelRange:
//            return false;
//
//        case DrawPurpose::FitView:
//            return  (NULL == dynamic_cast <IViewContextIgnoreCallouts*> (&viewContext));    // Ick.  TR# 325226, TR# 328200  - When getting callout range need true fit not CVE extent.
//
//         case DrawPurpose::Measure:
//            return isTransientDisplay;
//
//        default:
//            return true;
//        }
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool     ProxyDisplayCacheBase::OwnsXAttribute (XAttributeHandlerId handlerId, UInt32 xAttrId)
//    {
//    return  handlerId == CachedVisibleEdgeCacheId() ||
//            handlerId == XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, DisplayStyleHandlerSubID_Unused1);      // Was occlusion map.
//    }
//
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    RayBentley      06/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//DgnModelP ProxyRestoreUtil::FindModelChild (DgnModelP parent, ElementId childId)
//    {
//    DgnAttachmentArrayP        children;
//
//    if (NULL != parent &&
//        NULL != (children = parent->GetDgnAttachmentsP()))
//        {
//        for (size_t i=0; i<children->size(); i++)
//            {
//            DgnModelP        child;
//            DgnAttachmentP      refFile;
//
//            if (NULL != (child = children->at(i)) &&
//                NULL != (refFile = child->AsDgnAttachmentP()) && refFile->GetElementId() == childId)
//                return child;
//            }
//        }
//    return NULL;
//    }
//
//  
//
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyRestoreUtil::CopyData (void* destination, size_t size, byte const*& dataP, byte const* dataEndP)
    {
    if (dataP + size > dataEndP)
        throw ProxyRestoreUtil::ReadError();;

    memcpy (destination, dataP, size);
    dataP += size;

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ProxyRestoreUtil::CopyString (WStringR string, byte const*& dataP, byte const* dataEndP)
    {
    UInt32          size;
    StatusInt       status;
    static UInt32   s_maxStringSize = 4096;

    if (SUCCESS != (status = ProxyRestoreUtil::CopyData (size, dataP, dataEndP)))
        return status;

    size_t          charSize = size * sizeof (WChar);

    if (charSize > s_maxStringSize || dataP + charSize > dataEndP)
        return ERROR;

    string = 0 == size ? WString () : WString ((WCharP) dataP);

    dataP += charSize;

    return SUCCESS;
    }


