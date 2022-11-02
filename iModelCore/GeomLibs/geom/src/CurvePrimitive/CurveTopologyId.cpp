/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId::CurveTopologyId (CurveTopologyIdCR rhs) : m_type (rhs.m_type), m_ids (rhs.m_ids) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId::CurveTopologyId (uint32_t id, CurveTopologyIdCR rhs) : m_type (rhs.m_type)
    {
    m_ids.resize (rhs.GetCount() + 1);
    m_ids[0] = id;

    for (size_t i=1, count = m_ids.size(); i < count; i++)
        m_ids[i] = rhs.m_ids[i-1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId::CurveTopologyId (CurveTopologyIdCR lhs, uint32_t id) : m_type (lhs.m_type)
    {
    m_ids = lhs.m_ids;
    m_ids.push_back (id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId::CurveTopologyId (Type type, uint32_t id) : m_type (type)
    {
    m_ids.push_back (id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId::CurveTopologyId (Type type, uint32_t id0, uint32_t id1) : m_type (type)
    {
    m_ids.push_back (id0);
    m_ids.push_back (id1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId::CurveTopologyId (Type type, uint32_t id0, uint32_t id1, uint32_t id2) : m_type (type)
    {
    m_ids.push_back (id0);
    m_ids.push_back (id1);
    m_ids.push_back (id2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
void CurveTopologyId::Clear()
    {
    m_type = Type::Unknown;
    m_ids.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
bool CurveTopologyId::operator == (CurveTopologyIdCR rhs) const
    {
    return m_type == rhs.m_type && m_ids == rhs.m_ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
bool CurveTopologyId::operator < (CurveTopologyIdCR rhs) const
    {
    if (m_type != rhs.m_type)
        return m_type < rhs.m_type;

    if (m_ids.size() != rhs.m_ids.size())
        return m_ids.size() < rhs.m_ids.size();

    for (size_t i=0, count = m_ids.size(); i<count; i++)
        if (m_ids[i] != rhs.m_ids[i])
            return m_ids[i] < rhs.m_ids[i];

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::Init (Type type, uint32_t const* ids, size_t idCount)
    {
    m_type = type;

    m_ids.resize (idCount);
    memcpy (&m_ids.front(), ids, idCount * sizeof (uint32_t));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
enum PackedIdSize
    {
    IdSize_UInt8   = 0,
    IdSize_UInt16  = 1,
    IdSize_UInt32  = 2,
    IdSize_Invalid = 7,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
void CurveTopologyId::Pack (bvector<Byte>& packed) const
    {
    packed.clear();
    if (m_ids.empty())
        {
        packed.resize (2);
        packed.at(0) = (Byte)m_type;
        packed.at(1) = IdSize_UInt8;
        return;
        }

    size_t      count = m_ids.size();
    uint32_t    max = m_ids[0];

    for (size_t i=1; i<count; i++)
        if (m_ids[i] > max)
            max = m_ids[i];

    if (max <= 0xff)
        {
        packed.resize (2 + count);
        packed.at(0) = (Byte) m_type;
        packed.at(1) = (Byte) IdSize_UInt8;
        for (size_t i=0; i< count; i++)
            packed.at (i+2) = (Byte) m_ids[i];
        }
    else if (max <= 0xffff)
        {
        packed.resize (2 + 2 * count);
        packed.at(0) = (Byte) m_type;
        packed.at(1) = (Byte) IdSize_UInt16;

        for (size_t i=0; i<count; i++)
            {
            uint16_t    value =  (uint16_t) m_ids[i];
            memcpy (&packed.at(2 + (i << 1)), &value, sizeof (value));
            }
        }
    else
        {
        packed.resize (2 + 4 * count);
        packed.at(0) = (Byte) m_type;
        packed.at(1) = (Byte) IdSize_UInt32;
        memcpy ((void*) &packed.at(2), &m_ids.front(), count * sizeof (uint32_t));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::Init (void const* pData, size_t dataBytes)
    {
    Byte const*  data = (Byte const*) pData;

    m_ids.clear();
    if (dataBytes < 2)
        {
        m_type = Type::Unknown;
        return SUCCESS;
        }

    dataBytes -= 2;     // Type & count.
    m_type = (Type) *data++;
    switch (*data++)
        {
        case IdSize_UInt8:
            {
            size_t count = ((uint8_t) dataBytes);

            m_ids.resize (count);
            for (size_t i=0; i<count; i++)
                m_ids[i] = data[i];

            return SUCCESS;
            }

        case IdSize_UInt16:
            {
            size_t count = ((uint8_t) (dataBytes >> 1));

            m_ids.resize (count);

            uint16_t const*     pValue = (uint16_t const*) data;
            for (size_t i=0; i<count; i++)
                m_ids[i] = *pValue++;

            return SUCCESS;
            }

        case IdSize_UInt32:
            {
            size_t  count = ((uint8_t) (dataBytes >> 2));

            m_ids.resize (count);

            uint32_t const*     pValue = (uint32_t const*) data;
            for (size_t i=0; i<count; i++)
                m_ids[i] = *pValue++;

            return SUCCESS;
            }
        }
    assert (false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
Utf8String CurveTopologyId::GetDebugString () const
    {
    static Utf8CP s_typeStrings[] = {"Unknown",                 // 0
                                     "B-Rep Shared Edge",       // 1
                                     "B-Rep Sheet Edge",        // 2
                                     "B-Rep Silhouette Edge",   // 3
                                     "Mesh Shared Edge",        // 4
                                     "Mesh Sheet Edge",         // 5
                                     "Mesh Unknown",            // 6
                                     "Mesh Indexed Edge",       // 7
                                     "Wire",                    // 8
                                     "Section Shape",           // 9
                                     "Section Wire",            // 10
                                     "Analytic Surface Curve",  // 11
                                     "Bounded Plane Edge",      // 12
                                     "Geometry Map",            // 13
                                     "Projection Profile",      // 14
                                     "Projection Lateral",      // 15
                                     "Cut Fill Geometry",       // 16
                                     "Cut Wireframe Geometry",  // 17
                                     "Intersection",            // 18
                                     "SweepSilhouette",         // 19
                                     "BRep Isoline",            // 20
                                     "Curve Vector",            // 21
                                     "Polyface Cut",            // 22
                                     "Polyface Edge",           // 23
                                     "Mesh Edge Vertices",      // 24
                                     "BRep Planar Face",        // 25
                                     "Triangulation Boundary",  // 26
                                     "BRep UnIdentified Edge",  // 27
                                     };
    // When a type is added ... 1) add the string above, 2) change the Type::XXX below to agree
    
    Utf8String string = m_type <= Type::BRepUnIdentifiedEdge ? Utf8String(s_typeStrings[(uint8_t)m_type]) : Utf8String("Error: m_type > Type::Max");

    if (0 == GetCount())
        return string + "Null";

    string = string + " (";
    for (size_t i=0, count = GetCount(); i<count; i++)
        {
        if (i)
            string = string + ", ";

        Utf8String intString;
        intString.Sprintf ("%d", GetId(i));
        string = string + intString;
        }

    string = string + ")";

    return string;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId CurveTopologyId::FromBRepIsoline (FaceId const& faceId, size_t isolineIndex)
    {
    uint32_t    ids[3];

    ids[0] = (uint32_t) faceId.nodeId;
    ids[1] = (uint32_t) faceId.entityId;
    ids[2] = (uint32_t) isolineIndex;

    return CurveTopologyId (Type::BRepIsoIsoline, ids, 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId CurveTopologyId::FromBRepSharedEdge (FaceId const& faceId0, FaceId const& faceId1)
    {
    uint32_t    ids[4];

    ids[0] = (uint32_t) faceId0.nodeId;
    ids[1] = (uint32_t) faceId0.entityId;
    ids[2] = (uint32_t) faceId1.nodeId;
    ids[3] = (uint32_t) faceId1.entityId;

    return CurveTopologyId (Type::BRepSharedEdge, ids, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId CurveTopologyId::FromBRepSheetEdge(FaceId const& faceId) {return CurveTopologyId(Type::BRepSheetEdge, faceId.nodeId, faceId.entityId);}
CurveTopologyId CurveTopologyId::FromBRepPlanarFace(FaceId const& faceId) {return CurveTopologyId(Type::BRepPlanarFace, faceId.nodeId, faceId.entityId);}
CurveTopologyId CurveTopologyId::FromBRepSilhouette(FaceId const& faceId) {return CurveTopologyId(Type::BRepSilhouette, faceId.nodeId, faceId.entityId);}
CurveTopologyId CurveTopologyId::FromSweepProfile(size_t profileIndex) {return CurveTopologyId(Type::SweepProfile, (uint32_t) profileIndex);}
CurveTopologyId CurveTopologyId::FromSweepLateral(size_t lateralIndex) {return CurveTopologyId(Type::SweepLateral, (uint32_t) lateralIndex);}
CurveTopologyId CurveTopologyId::FromSweepSilhouette (size_t silhouetteIndex) {return CurveTopologyId(Type::SweepSilhouette, (uint32_t) silhouetteIndex);}
CurveTopologyId CurveTopologyId::FromMeshEdgeVertices(size_t vertexIndex0, size_t vertexIndex1) {return CurveTopologyId(Type::MeshEdgeVertices, (uint32_t) vertexIndex0, (uint32_t) vertexIndex1);}
CurveTopologyId CurveTopologyId::FromVisEdgesAnalytic(size_t curveIndex) {return CurveTopologyId(Type::VisEdgesAnalytic, (uint32_t) curveIndex);}
CurveTopologyId CurveTopologyId::FromVisEdgesBoundedPlane(size_t loopIndex) {return CurveTopologyId(Type::VisEdgesBoundedPlane, (uint32_t) loopIndex);}
CurveTopologyId CurveTopologyId::FromParasolidGPArrayId(size_t loopIndex, size_t curveIndex) {return CurveTopologyId(Type::VisEdgesBoundedPlane, (uint32_t) loopIndex, (uint32_t) curveIndex);}
CurveTopologyId CurveTopologyId::FromMeshSharedEdge(size_t faceIndex0, size_t faceIndex1) {return CurveTopologyId(Type::MeshSharedEdge, (uint32_t) faceIndex0, (uint32_t) faceIndex1);}
CurveTopologyId CurveTopologyId::FromUnknownCurve(size_t curveIndex) {return CurveTopologyId(Type::Unknown, (uint32_t) curveIndex);}
CurveTopologyId CurveTopologyId::FromCurveVector() {return CurveTopologyId(Type::CurveVector);}
CurveTopologyId CurveTopologyId::FromWire() {return CurveTopologyId(Type::Wire);}
CurveTopologyId CurveTopologyId::FromGeometryMap() {return CurveTopologyId(Type::GeometryMap);}
CurveTopologyId CurveTopologyId::FromVisEdgesIntersection() {return CurveTopologyId(Type::VisEdgesIntersection);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::GetBRepIsoline (FaceId* faceId, size_t* isolineIndex) const
    {
    if (Type::BRepIsoIsoline != GetType() || GetCount() < 3)
        return ERROR;

    if (NULL != faceId)
        {
        faceId->nodeId   = m_ids[0];
        faceId->entityId = m_ids[1];
        }

    if (NULL != isolineIndex)
        *isolineIndex = m_ids[2];

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::GetBRepSharedEdge (FaceId* faceId0, FaceId* faceId1) const
    {
    if (Type::BRepSharedEdge != GetType() || GetCount() < 4)
        return ERROR;

    if (NULL != faceId0)
        {
        faceId0->nodeId   = m_ids[0];
        faceId0->entityId = m_ids[1];
        }

    if (NULL != faceId1)
        {
        faceId1->nodeId   = m_ids[2];
        faceId1->entityId = m_ids[3];
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::GetBRepSheetEdge (FaceId* edgeId) const
    {
    if (Type::BRepSheetEdge != GetType() || GetCount() < 2)
        return ERROR;

    if (NULL != edgeId)
        {
        edgeId->nodeId   = m_ids[0];
        edgeId->entityId = m_ids[1];
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::GetBRepPlanarFace (FaceId* faceId) const
    {
    if (Type::BRepPlanarFace != GetType() || GetCount() < 2)
        return ERROR;

    if (NULL != faceId)
        {
        faceId->nodeId   = m_ids[0];
        faceId->entityId = m_ids[1];
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::AddCurveVectorIds (CurveVectorCR curveVector, CurvePrimitiveId::Type type, CurveTopologyIdCR id, uint16_t index, uint16_t partIndex)
    {
    size_t  curveIndex = 0;

    for (ICurvePrimitivePtr curve: curveVector)
        {
        CurveTopologyId     thisId = id;

        thisId.m_ids.push_back ((uint32_t) curveVector.GetBoundaryType());
        thisId.m_ids.push_back ((uint32_t) curveIndex++);

        if (NULL == curve->GetId())
            {
            CurvePrimitiveIdPtr newID = CurvePrimitiveId::Create (type, thisId, index, partIndex);
            curve->SetId (newID.get());
            }

        CurveVectorPtr children = curve->GetChildCurveVectorP();

        if (children.IsValid())
            AddCurveVectorIds (*children, type, thisId, index, partIndex);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurveTopologyId::AddPolyfaceCutIds (CurveVectorCR curveVector, uint16_t index, uint16_t partIndex)
    {
    for (ICurvePrimitivePtr curve: curveVector)
        {
        FacetEdgeLocationDetailVectorPtr facetEdgeDetails = curve->GetFacetEdgeLocationDetailVectorPtr();

        if (facetEdgeDetails.IsValid() && facetEdgeDetails->size() < Max_PolyfaceCutIds)
            {
            double              fraction;
            size_t              edgeIndex;
            bvector<uint32_t>   ids (facetEdgeDetails->size());

            for (size_t i=0; i<facetEdgeDetails->size(); i++)
                if (facetEdgeDetails->TryGet (i, edgeIndex, fraction))
                    ids[i] = ((uint32_t) edgeIndex);

            CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create (CurvePrimitiveId::Type::PolyfaceCut, CurveTopologyId (Type::PolyfaceCut, &ids.front(), ids.size()), index, partIndex);
            curve->SetId (newId.get());
            }

        CurveVectorPtr children = curve->GetChildCurveVectorP();

        if (children.IsValid())
            AddPolyfaceCutIds (*children, index, partIndex);
        }

    return SUCCESS;
    }

END_BENTLEY_GEOMETRY_NAMESPACE