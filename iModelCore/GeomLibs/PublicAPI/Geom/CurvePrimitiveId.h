
/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/CurvePrimitiveId.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef struct CurvePrimitiveId* CurvePrimitiveIdP;
typedef struct CurvePrimitiveId& CurvePrimitiveIdR;
typedef struct CurvePrimitiveId const& CurvePrimitiveIdCR;
typedef struct CurvePrimitiveId const* CurvePrimitiveIdCP;
typedef RefCountedPtr<struct CurvePrimitiveId> CurvePrimitiveIdPtr;

/*=================================================================================**//**
* CurvePrimitiveId is a class that identifies the source of a curve primitive.
* It is stored as a packed structure with the "Type" indicating the format of the
* contents. The most common form is a CurveTopologyId - containing the 
* identification of the curve primitive from the wireframe representation of a
* geometric primitive such as a B-Rep body or solid primitive. The CurvePrimitiveId
* also contain information to identify the specific geometric primitive within
* a GeometryStream that the curve primitive originated from.
* @bsiclass                                                     Ray.Bentley     10/2012
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveId : RefCountedBase
{
    //! enumeration of sources of CurvePrimitiveId
    enum class Type : uint16_t
    {
        ParasolidCut           = 0,        // Parasolid EdgeId/FaceId.
        CachedEdge             = 3,        // CVE. ProxyHLEdgeSegmentId.
        CachedCut              = 4,        // CVE. CurveTopologyId.
        CachedUnderlay         = 5,        // CVE. CurveTopologyId.
        ParasolidBody          = 6,        // CurveTopologyId.
        SolidPrimitive         = 7,        // CurveTopologyId.
        CurveVector            = 8,        // CurveTopologyId.
        PolyfaceCut            = 9,        // CurveTopologyId.
        PolyfaceEdge           = 10,       // CurveTopologyId.
        UnspecifiedTopologyId  = 11,       // CurveTopologyId.
        ConceptStationAlignmentIndex = 50, 
        Max,
        CutGeometry            = 99        // Cut Associations.
    };

private:
    Type                m_type;
    uint16_t            m_geomStreamIndex;
    uint16_t            m_partStreamIndex;
    bvector<uint8_t>    m_idData;

    CurvePrimitiveId(Type edgeType, void const* edgeId, size_t edgeIdSize, uint16_t index = 0, uint16_t partIndex = 0);
    CurvePrimitiveId(Type edgeType, struct CurveTopologyId const& topologyId, uint16_t index = 0, uint16_t partIndex = 0);
    CurvePrimitiveId(void const* data, size_t size);
    
public:
    //! Allocate and fill with type and data.
    GEOMDLLIMPEXP static CurvePrimitiveIdPtr Create(Type type, void const* id, size_t size, uint16_t index = 0, uint16_t partIndex = 0);
    //! Allocate and fill with type and data.
    GEOMDLLIMPEXP static CurvePrimitiveIdPtr Create(Type type, struct CurveTopologyId const& topologyId, uint16_t index = 0, uint16_t partIndex = 0);
    //! Allocate and fill from packed data.
    GEOMDLLIMPEXP static CurvePrimitiveIdPtr Create(void const* data, size_t size);
    //! Allocate and fill a copy.
    GEOMDLLIMPEXP static CurvePrimitiveIdPtr Create (CurvePrimitiveIdCR id);

    //! Get the type code.
    Type GetType() const {return m_type;}
    //! Query the geometry stream index
    uint32_t GetGeometryStreamIndex() const {return m_geomStreamIndex;}
    //! Query the part geometry stream index (valid when m_geomStreamIndex refers to a part)
    uint32_t GetPartGeometryStreamIndex() const {return m_partStreamIndex;}
    //! Return size of the id data array.
    size_t GetIdSize() const {return m_idData.size();}
    //! Return interior pointer to id data.
    uint8_t const* PeekId() const {return m_idData.empty() ? NULL : &m_idData.front();}

    //! Return the topologyId object. This may fail (and return defaulted structure) for some types.
    GEOMDLLIMPEXP struct CurveTopologyId GetCurveTopologyId() const;
    //! bitwise equality test.
    GEOMDLLIMPEXP bool operator==(CurvePrimitiveIdCR rhs) const;
    //! bitwise less than test.
    GEOMDLLIMPEXP bool operator<(CurvePrimitiveIdCR rhs) const;

    //! Return a copy.
    GEOMDLLIMPEXP CurvePrimitiveIdPtr Clone() const;
    //! Copy data to target object if this is a parasolid body.
    GEOMDLLIMPEXP BentleyStatus GetParasolidBodyId(struct CurveTopologyId& id) const;
    //! Copy data to target object if this is a solid primitive.
    GEOMDLLIMPEXP BentleyStatus GetSolidPrimitiveId(struct CurveTopologyId& id) const;
    //! If this is a Type_PolyfaceCut with exactly nTargetIds id values, copy out the topology type and the ids.
    GEOMDLLIMPEXP BentleyStatus GetLineStringAssociationIds(int& topologyType, bvector<uint32_t>& ids, size_t nTargetIds);

    //! Copy the id data into bytes.
    GEOMDLLIMPEXP void Store(bvector<uint8_t>& data) const;
    //! Get debug string              
    GEOMDLLIMPEXP Utf8String GetDebugString() const;


}; // CurvePrimitiveId

END_BENTLEY_GEOMETRY_NAMESPACE
