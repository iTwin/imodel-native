/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurvePrimitiveId::operator==(CurvePrimitiveIdCR rhs) const 
    { 
    return (m_type == rhs.m_type && m_idData == rhs.m_idData && m_geomStreamIndex == rhs.m_geomStreamIndex && m_partStreamIndex == rhs.m_partStreamIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------+---------------+---------------+---------------+---------------+------*/
bool CurvePrimitiveId::operator<(CurvePrimitiveIdCR rhs) const
    {
    if (m_type != rhs.m_type)
        return m_type < rhs.m_type;

    if (m_idData.size() != rhs.m_idData.size())
        return m_idData.size() < rhs.m_idData.size();

    for (size_t i=0, count = m_idData.size(); i<count; i++)
        if (m_idData[i] != rhs.m_idData[i])
            return m_idData[i] < rhs.m_idData[i];

    if (m_geomStreamIndex != rhs.m_geomStreamIndex)
        return m_geomStreamIndex < rhs.m_geomStreamIndex;
    
    if (m_partStreamIndex != rhs.m_partStreamIndex)
        return m_partStreamIndex < rhs.m_partStreamIndex;

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurvePrimitiveId::CurvePrimitiveId (CurvePrimitiveId::Type type, void const* id, size_t idSize, uint16_t index, uint16_t partIndex) : m_type(type), m_geomStreamIndex(index), m_partStreamIndex(partIndex)
    {
    m_idData.resize(idSize);
    if (0 != idSize)
        memcpy ((void*) &m_idData.front(), id, idSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurvePrimitiveId::CurvePrimitiveId (CurvePrimitiveId::Type type, CurveTopologyIdCR topologyId, uint16_t index, uint16_t partIndex) : m_type (type), m_geomStreamIndex(index), m_partStreamIndex(partIndex)
    {
    topologyId.Pack(m_idData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CurvePrimitiveId::Store (bvector<uint8_t>& data) const 
    {
    size_t hdrSize = (sizeof(m_type) + sizeof(m_geomStreamIndex) + sizeof(m_partStreamIndex));

    data.resize(hdrSize + m_idData.size());

    uint8_t* p = &data.front();

    memcpy(p, &m_type, sizeof(m_type));
    p += sizeof(m_type);

    memcpy(p, &m_geomStreamIndex, sizeof(m_geomStreamIndex));
    p += sizeof(m_geomStreamIndex);

    memcpy(p, &m_partStreamIndex, sizeof(m_partStreamIndex));
    p += sizeof(m_partStreamIndex);

    if (0 == m_idData.size())
        return;

    memcpy(p, &m_idData.front(), m_idData.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurvePrimitiveIdPtr CurvePrimitiveId::Create(CurvePrimitiveId::Type type, CurveTopologyIdCR topologyId, uint16_t index, uint16_t partIndex) {return new CurvePrimitiveId (type, topologyId, index, partIndex);}
CurvePrimitiveIdPtr CurvePrimitiveId::Create(CurvePrimitiveId::Type type, void const* id, size_t idSize, uint16_t index, uint16_t partIndex) {return new CurvePrimitiveId (type, id, idSize, index, partIndex);}
CurvePrimitiveIdPtr CurvePrimitiveId::Create(void const* data, size_t size) {return new CurvePrimitiveId (data, size);}
CurvePrimitiveIdPtr CurvePrimitiveId::Create(CurvePrimitiveIdCR id) {return new CurvePrimitiveId (id);}
CurvePrimitiveIdPtr CurvePrimitiveId::Clone() const {return new CurvePrimitiveId (*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurvePrimitiveId::CurvePrimitiveId (void const* data, size_t dataSize)
    { 
    size_t hdrSize = (sizeof(m_type) + sizeof(m_geomStreamIndex) + sizeof(m_partStreamIndex));

    if (nullptr == data || dataSize < hdrSize)
        {
        m_type = Type::CutGeometry;
        m_geomStreamIndex = 0;
        m_partStreamIndex = 0;
        assert (false);
        return;
        }

    uint8_t const* p = (uint8_t const*) data;

    memcpy(&m_type, p, sizeof(m_type));
    p += sizeof(m_type);

    memcpy(&m_geomStreamIndex, p, sizeof(m_geomStreamIndex));
    p += sizeof(m_geomStreamIndex);

    memcpy(&m_partStreamIndex, p, sizeof(m_partStreamIndex));
    p += sizeof(m_partStreamIndex);

    size_t idSize = (dataSize - hdrSize);

    if (0 == idSize)
        return;

    m_idData.resize(idSize);
    memcpy(&m_idData.front(), p, idSize);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CurvePrimitiveId::GetDebugString () const
    {
    static Utf8CP s_typeStrings[] = {"Parasolid Cut",        // 0
                                     "Unknown",              // 1
                                     "ACIS Cut",             // 2
                                     "CVE Edge",             // 3
                                     "CVE Cut",              // 4
                                     "CVE Underlay",         // 5
                                     "Parasolid Body",       // 6
                                     "Solid Primitive",      // 7
                                     "CurveVector",          // 8
                                     "Polyface Cut",         // 9   
                                     "Polyface Edge",        // 10
                                     "UnspecifiedTopologyId" // 11
                                     }; 
    
    Type        type = GetType();
    Utf8String  string = Utf8String ("Curve Primitive Id: ") + Utf8String (Type::CutGeometry == type || type >= Type::Max ? "Other" : s_typeStrings[(uint16_t)type]);
    
    CurveTopologyId topologyId = GetCurveTopologyId ();
    if (!topologyId.IsEmpty())
        string += " Topology: " +  topologyId.GetDebugString();
        
    string = string + Utf8PrintfString(" (%d-%d)\n", m_geomStreamIndex, m_partStreamIndex);

    return string;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveTopologyId CurvePrimitiveId::GetCurveTopologyId() const
    {
    Type type = GetType();

    if (type > Type::CachedEdge && type != Type::CutGeometry)
        return CurveTopologyId (&m_idData.front(), m_idData.size());

    return CurveTopologyId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurvePrimitiveId::GetLineStringAssociationIds (int& topologyType, bvector<uint32_t>& ids, size_t nTargetIds)
    {
    if (m_type != Type::PolyfaceCut)
        return ERROR;

    CurveTopologyId topologyId = GetCurveTopologyId();
    
    if (topologyId.GetCount() != nTargetIds)
        return ERROR;

    ids.resize (nTargetIds);

    for (size_t i=0; i<nTargetIds; i++)
        ids[i] = topologyId.GetId (i);

    topologyType = (int) topologyId.GetType();

    return SUCCESS;                                                                                                                                                                                                                                                     
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurvePrimitiveId::GetParasolidBodyId (CurveTopologyIdR id) const  {return (Type::ParasolidBody == GetType()) ? id.Init (PeekId(), GetIdSize()) : ERROR;}
BentleyStatus CurvePrimitiveId::GetSolidPrimitiveId (CurveTopologyIdR id) const  {return (Type::SolidPrimitive == GetType()) ? id.Init (PeekId(), GetIdSize()) : ERROR;}

END_BENTLEY_GEOMETRY_NAMESPACE