/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef struct CurveTopologyId* CurveTopologyIdP;
typedef struct CurveTopologyId& CurveTopologyIdR;
typedef struct CurveTopologyId const* CurveTopologyIdCP;
typedef struct CurveTopologyId const& CurveTopologyIdCR;

/*=================================================================================**//**
* The CurveTopologyId class identifies the source of a curve within a larger
* wireframe entity such as a B-Rep body or SolidPrimitive. The "Type" enumeration 
* identifies the type of data stored.  The data itself is packed into a Byte array.
* A CurveTopologyId is generally embedded with a CurvePrimitiveId. The CurvePrimitiveId
* includes additional information to identify the curve within the GeometryStream.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CurveTopologyId
{
//! Anonymous enumeration to define CurveTopologyId constants
enum
    {
    INVALID_Id  = 0xffffffff,
    Max_PolyfaceCutIds = 255,
    };

//! Enumeration of how the curve originated.
enum class Type : uint8_t
    {
    Unknown                     = 0,
    BRepSharedEdge              = 1,        // 2 X FaceId (NodeId, Entity Id), Optional sub index for CVE.
    BRepSheetEdge               = 2,        // 1 Face/Edge Id (Node Id, Entity Id), Optional sub index for CVE.
    BRepSilhouette              = 3,        // FaceId (Node, EntityId)
    MeshSharedEdge              = 4,        // 2 X Face Index
    MeshSheetEdge               = 5,        // Face Index
    MeshUnknown                 = 6,        // 2 Values -  Primitive Index, Mesh Index.
    IndexedMeshEdge             = 7,        // 2 Values -  Primitive Index, Chain Index.
    Wire                        = 8,        // Curve Primitive Index.
    UnannnouncedSectionShape    = 9,        // 1 Value  -  Section Primitive(GPA) Index.
    UnannouncedSectionWire      = 10,       // 2 Values -  Section Primitive(GPA) Index.
    VisEdgesAnalytic            = 11,       // Analytic Surface Curve Index.
    VisEdgesBoundedPlane        = 12,       // LoopIndex.
    GeometryMap                 = 13,       // 
    SweepProfile                = 14,       // Profile index, Curve Primitive Index
    SweepLateral                = 15,       // Lateral Index.
    CutWires                    = 16,
    CutFill                     = 17,
    VisEdgesIntersection        = 18,       // None (not currently associable)..
    SweepSilhouette             = 19,       // Primitive Index, SilhouetteIndex (cones).
    BRepIsoIsoline              = 20,       // 4 Values. - Primitive Index, FaceId (Node, EntityId), Isoline Index.
    CurveVector                 = 21,
    PolyfaceCut                 = 22,
    PolyfaceEdge                = 23,
    MeshEdgeVertices            = 24,       // SS3 Sheet edges stored only a single face edge which may not be unique. Replaced by start/end vertex edges.
    BRepPlanarFace              = 25,       // Draw Method Index, FaceId.   (For Planar Constraints).
    TriangulationBoundary       = 26,        // triangulation of points
    BRepUnIdentifiedEdge        = 27,
    Max,
    };

private:
    Type                m_type;
    bvector<uint32_t>   m_ids;

public:
//! Default constructor.
CurveTopologyId() : m_type(Type::Unknown) {}
//! Constructor with type code
CurveTopologyId(Type type) : m_type (type) {}
//! Constructor with array of id data.
CurveTopologyId(void const* data, size_t dataBytes) {Init (data, dataBytes);}
//! Constructor with type code and single integer id.
GEOMDLLIMPEXP CurveTopologyId(Type, uint32_t id);
//! Constructor with type code and two integer ids.
GEOMDLLIMPEXP CurveTopologyId(Type, uint32_t id0, uint32_t id1);
//! Constructor with type code and three integer ids.
GEOMDLLIMPEXP CurveTopologyId(Type, uint32_t id0, uint32_t id1, uint32_t id2);
//! Constructor with type code and arrasy of ids.
GEOMDLLIMPEXP CurveTopologyId(Type type, uint32_t const* ids, size_t idCount) {Init(type, ids, idCount);}
//! Constructor with id and referenced curve (id saved first, then ids from curveId)
GEOMDLLIMPEXP CurveTopologyId(uint32_t id, CurveTopologyIdCR CurveId);
//! Constructor with referenced curve and id (ids from curveId are saved first, then the added id)
GEOMDLLIMPEXP CurveTopologyId(CurveTopologyIdCR curveId, uint32_t id);
//! Constructor, copy ids from rhs
GEOMDLLIMPEXP CurveTopologyId(CurveTopologyIdCR rhs);

//! Initializer with type code and array of ids.
GEOMDLLIMPEXP BentleyStatus Init(Type type, uint32_t const* ids, size_t idCount);
//! Initialize with array of id data.
GEOMDLLIMPEXP BentleyStatus Init (void const* data, size_t dataBytes);
//! Clear the id array and set the type code to default unknown value.
GEOMDLLIMPEXP void Clear();

//! Return the type code
Type GetType() const {return (Type) m_type;}
//! Return the id data count (in UInt32's)
size_t GetCount() const {return m_ids.size();}
//! Return id by index
uint32_t GetId (size_t index) const {return index >= m_ids.size() ? INVALID_Id : m_ids[index];}
//! Return true if there are no ids.
bool IsEmpty() const {return m_ids.empty();}

//! Direct equality tests
GEOMDLLIMPEXP bool operator==(CurveTopologyIdCR rhs) const;
//! Lexical sort or (typeCode, idCount, id0, id1...)
GEOMDLLIMPEXP bool operator<(CurveTopologyIdCR rhs) const;

//! construct and return with FaceId and isolineIndex
GEOMDLLIMPEXP static CurveTopologyId FromBRepIsoline(FaceId const& faceId, size_t isolineIndex);              
//! Construct an return for brep edge with two faces.
GEOMDLLIMPEXP static CurveTopologyId FromBRepSharedEdge(FaceId const& faceId0, FaceId const& faceId1);
//! Construct and return for brep edge with one face.
GEOMDLLIMPEXP static CurveTopologyId FromBRepSheetEdge(FaceId const& edgeId);
//! Construct and return for brep silhouette edge.
GEOMDLLIMPEXP static CurveTopologyId FromBRepSilhouette(FaceId const& edgeId);
//! Construct and return for brep edge with one face.
GEOMDLLIMPEXP static CurveTopologyId FromBRepPlanarFace(FaceId const& faceId);
//! construct and return for index of swept profile curve.
GEOMDLLIMPEXP static CurveTopologyId FromSweepProfile(size_t profileIndex);
//! construct and return for indexed lateral within sweep.
GEOMDLLIMPEXP static CurveTopologyId FromSweepLateral(size_t lateralIndex);
//! construct and return for silhouette of sweep
GEOMDLLIMPEXP static CurveTopologyId FromSweepSilhouette(size_t silhouetteIndex);
//! construct and return for mesh edge with 2 faces
GEOMDLLIMPEXP static CurveTopologyId FromMeshSharedEdge(size_t faceIndex0, size_t faceIndex1);
//! construct and return for mesh edge with 2 vertices
GEOMDLLIMPEXP static CurveTopologyId FromMeshEdgeVertices(size_t vertexIndex0, size_t vertexIndex1);
//! construct and return for geometry map.
GEOMDLLIMPEXP static CurveTopologyId FromGeometryMap();
//! construct and return for wire edge
GEOMDLLIMPEXP static CurveTopologyId FromWire();
//! construct and return for unknown curve
GEOMDLLIMPEXP static CurveTopologyId FromUnknownCurve(size_t curveIndex);
//! construct and return for indexed loop in visible edges
GEOMDLLIMPEXP static CurveTopologyId FromVisEdgesBoundedPlane(size_t);
//! construct and return from Parasolid parasolid GPArray ID
GEOMDLLIMPEXP static CurveTopologyId FromParasolidGPArrayId(size_t, size_t);
//! construct and return for analytic curve in visible edges
GEOMDLLIMPEXP static CurveTopologyId FromVisEdgesAnalytic(size_t curveIndex);
//! construct and return for intersection curve in visible edges
GEOMDLLIMPEXP static CurveTopologyId FromVisEdgesIntersection();
//! construct and return for curve vector
GEOMDLLIMPEXP static CurveTopologyId FromCurveVector();

//! Extract brep isoline.
GEOMDLLIMPEXP BentleyStatus GetBRepIsoline(FaceId* faceId, size_t* isolineIndex) const;
//! extract adjacent faces of brep edge
GEOMDLLIMPEXP BentleyStatus GetBRepSharedEdge(FaceId* faceId0, FaceId* faceId1) const;
//! extract adjacent face of brep edge
GEOMDLLIMPEXP BentleyStatus GetBRepSheetEdge(FaceId* edgeId) const;
//! extract brep planar face
GEOMDLLIMPEXP BentleyStatus GetBRepPlanarFace(FaceId* faceId) const;

//! Return a packed form of the ids.
GEOMDLLIMPEXP void Pack(bvector<Byte>& packed) const;
//! Return debug string form
GEOMDLLIMPEXP Utf8String GetDebugString() const;

GEOMDLLIMPEXP static BentleyStatus AddCurveVectorIds(CurveVectorCR curveVector, CurvePrimitiveId::Type type, CurveTopologyIdCR id, uint16_t index = 0, uint16_t partIndex = 0);
GEOMDLLIMPEXP static BentleyStatus AddPolyfaceCutIds(CurveVectorCR curveVector, uint16_t index = 0, uint16_t partIndex = 0);

}; // CurveTopologyId

END_BENTLEY_GEOMETRY_NAMESPACE

