/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/bset.h>
#include <Bentley/BeId.h>
#include "GeomApi.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(IBRepEntity)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ISubEntity)
DEFINE_POINTER_SUFFIX_TYPEDEFS(IFaceMaterialAttachments)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TopologyPrimitive)

DEFINE_REF_COUNTED_PTR(IBRepEntity)
DEFINE_REF_COUNTED_PTR(ISubEntity)
DEFINE_REF_COUNTED_PTR(IFaceMaterialAttachments)
DEFINE_REF_COUNTED_PTR(TopologyPrimitive)

//=======================================================================================
//! @private
//! Facet table per-face material and color information.
//=======================================================================================
struct FaceAttachment
{
private:

    bool                m_useColor:1;       //!< true - color/transparency does not follow sub-category appearance.
    bool                m_useMaterial:1;    //!< true - material does not follow sub-category appearance.
    uint32_t            m_color;            //!< TBGR - T ignored, set from m_transparency
    double              m_transparency;     //!< 0.0 (fully opaque) <-> 1.0 (fully transparent)
    BeInt64Id           m_material;         //!< RenderMaterialId

public:

GEOMDLLIMPEXP FaceAttachment();

GEOMDLLIMPEXP bool operator == (struct FaceAttachment const&) const;
GEOMDLLIMPEXP bool operator < (struct FaceAttachment const&) const;

bool GetUseColor() const {return m_useColor;} //!< GetColor/GetTransparency are valid...
bool GetUseMaterial() const {return m_useMaterial;} //!< GetMaterial is valid...

uint32_t GetColor() const {return m_color;}
double GetTransparency() const {return m_transparency;}
BeInt64Id GetMaterial() const {return m_material;}

void SetColor(uint32_t color, double transparency) { m_color = color; m_transparency = transparency; m_useColor = true; }
void SetMaterial(BeInt64Id material) { m_material = material; m_useMaterial = true; }

}; // FaceAttachment

//! @private
typedef bvector<FaceAttachment> T_FaceAttachmentsVec; //!< Unique face attachments - first entry is "base" symbology
//! @private
typedef bmap<uint32_t, size_t> T_FaceToAttachmentIndexMap; //!< Face identifier to attachment index map

//=======================================================================================
//! @private
//! Face index to attachment symbology index used to add index attributes to faces.
//=======================================================================================
struct FaceIndexToAttachmentIndex
{
uint32_t m_faceIndex = 0;
uint32_t m_symbIndex = 0;
};

//! @private
typedef bvector<FaceIndexToAttachmentIndex> T_FaceIndexToAttachmentIndexVec;

//=======================================================================================
//! @private
//! Per-face material and color override
//=======================================================================================
struct IFaceMaterialAttachments : public IRefCounted
{
virtual T_FaceAttachmentsVec const& _GetFaceAttachmentsVec() const = 0;
virtual T_FaceAttachmentsVec& _GetFaceAttachmentsVecR() = 0;
};

//=======================================================================================
//! IBRepEntity represents a boundary representation body (BRep). A BRep is
//! defined by it's topology and geometry. The geometry is the surfaces, curves, and points.
//! The topology is the faces, edges, and vertices that describe how the geometry is connected.
//! The IBRepEntity is used to provide solid modeling functionality such as booleans,
//! blending, and hollowing. Typically only solids and non-planar sheet bodies are persisted
//! as BRep elements. A wire body or planar sheet body can be efficiently represented as
//! a CurveVector.
//=======================================================================================
struct IBRepEntity : BentleyApi::IRefCounted
{
public:

enum class EntityType
    {
    Solid    = 0, //!< Body consisting of at least one solid region.
    Sheet    = 1, //!< Body consisting of connected sets of faces having edges that are shared by a maximum of two faces.
    Wire     = 2, //!< Body consisting of connected sets of edges having vertices that are shared by a maximum of two edges.
    Invalid  = 3, //!< Body no longer valid (ex. extracted or consumed by operation).
    };

protected:

//! @private
virtual bool _IsEqual(IBRepEntityCR) const = 0;
//! @private
virtual EntityType _GetEntityType() const = 0;
//! @private
virtual DRange3d _GetEntityRange() const = 0;
//! @private
virtual DRange3d _GetLocalEntityRange() const = 0;
//! @private
virtual bool _HasCurvedFaceOrEdge() const = 0;
//! @private
virtual Transform _GetEntityTransform() const = 0;
//! @private
virtual bool _SetEntityTransform(TransformCR) = 0;
//! @private
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const = 0;
//! @private
virtual void _SetFaceMaterialAttachments(T_FaceAttachmentsVec const&, T_FaceIndexToAttachmentIndexVec const*) = 0;
//! @private
virtual IBRepEntityPtr _Clone() const = 0;
//! @private
virtual IBRepEntityPtr _CreateInstance(bool owned) const = 0;
//! @private
virtual bool _IsAlive() const = 0;
//! @private
virtual void _ChangePartition() const = 0;
//! @private
virtual bool _IsSameStructureAndGeometry(IBRepEntityCR, double tolerance) const = 0;

public:

//! Compare entities to see if they refer to the same body.
//! @return true if entities are equal.
bool IsEqual (IBRepEntityCR entity) const {return _IsEqual(entity);}

//! Get the body type for this entity.
//! @return The solid kernel entity type.
EntityType GetEntityType() const {return _GetEntityType();}

//! Get the axis aligned bounding box for this entity.
//! @return The axis aligned bounding box for the entity.
DRange3d GetEntityRange() const {return _GetEntityRange();}

//! Get the axis aligned bounding box for this entity without the entity's world transform applied.
DRange3d GetLocalEntityRange() const {return _GetLocalEntityRange();}

//! Check whether this entity has a non-linear edge or non-planar face.
bool HasCurvedFaceOrEdge() const {return _HasCurvedFaceOrEdge();}

//! Get the body to world transform for this entity.
//! The body to world translation allows for a body that will typically have a basis
//! point of 0,0,0 to be displayed at any world location in the model.
//! @return The body to world transform for the entity.
Transform GetEntityTransform() const {return _GetEntityTransform();}

//! Changes the body to world transform for the entity.
//! @param[in] transform The new solid to uor transform.
bool SetEntityTransform (TransformCR transform) {return _SetEntityTransform(transform);}

//! Modify the entity transform by pre-multiplying the current entity transform with the input transform. This method does
//! not change the BRep data in any way, it only changes the body to world transform.
//! @param[in] transform The transform to apply to the body.
//! @note This method was added for increased discoverability compared to PreMultiplyEntityTransformInPlace.
bool ApplyTransform (TransformCR transform) {return PreMultiplyEntityTransformInPlace(transform);}

//! PreMultiply the entity transform by the supplied (uor) transform
//! @param[in] uorTransform The transform to pre-multiply.
bool PreMultiplyEntityTransformInPlace (TransformCR uorTransform) {return _SetEntityTransform(Transform::FromProduct(uorTransform, _GetEntityTransform()));}

//! PostMultiply the entity transform by the supplied (solid) transform
//! @param[in] solidTransform The transform to post-multiply.
bool PostMultiplyEntityTransformInPlace (TransformCR solidTransform) {return _SetEntityTransform(Transform::FromProduct(_GetEntityTransform(), solidTransform));}

//! Optional per-face color/material overrides.
IFaceMaterialAttachmentsCP GetFaceMaterialAttachments() const {return _GetFaceMaterialAttachments();}

//! Optional editable per-face color/material overrides.
IFaceMaterialAttachmentsP GetFaceMaterialAttachmentsP() {return const_cast<IFaceMaterialAttachmentsP> (_GetFaceMaterialAttachments());}

//! Set per-face color/material overrides.
void SetFaceMaterialAttachments(T_FaceAttachmentsVec const& attachments, T_FaceIndexToAttachmentIndexVec const* faceIndexToAttachIndex=nullptr) {_SetFaceMaterialAttachments(attachments, faceIndexToAttachIndex);}

//! Create deep copy of this IBRepEntity.
IBRepEntityPtr Clone() const {return _Clone();}

//! Create a reference to this IBRepEntity without making a deep copy of the brep entity.
//! A non-owning instance can be used to optimize query operations that need to modify the entity transform.
//! A owning instance can be used in conjuction with a rollback mark for dynamics as the free of the entity will be rolled back.
virtual IBRepEntityPtr CreateInstance(bool owned = false) const {return _CreateInstance(owned);}

//! Check whether this instance still represents a live brep entity.
//! @return true if entity is alive.
bool IsAlive() const {return _IsAlive();}

//! Change partition of entity to current thread partition.
void ChangePartition() const {_ChangePartition();}

//! Check if all faces (or edges if wire bodies) are coincident to given tolerance.
virtual bool IsSameStructureAndGeometry(IBRepEntityCR entity, double tolerance) const {return _IsSameStructureAndGeometry(entity, tolerance);}

}; // IBRepEntity

//=======================================================================================
//! Class for multiple RefCounted geometry types that can represent face, edge, and vertex
//! sub-entities. Includes ICurvePrimitive, CurveVector, ISolidPrimitive, and IBRepEntity.
//! @ingroup GROUP_Geometry
//=======================================================================================
struct TopologyPrimitive : RefCountedBase
{
public:
    enum class GeometryType
    {
        CurvePrimitive      = 1, //!< Edge curves, or PointString for vertices.
        CurveVector         = 2, //!< Planar faces
        SolidPrimitive      = 3, //!< Untrimmed analytic faces
        BRepEntity          = 6, //!< Trimmed analytic faces or bspline faces
    };

protected:
    GeometryType                m_type;
    RefCountedPtr<IRefCounted>  m_data;

    TopologyPrimitive(ICurvePrimitivePtr const& source);
    TopologyPrimitive(CurveVectorPtr const& source);
    TopologyPrimitive(ISolidPrimitivePtr const& source);
    TopologyPrimitive(IBRepEntityPtr const& source);

public:
    GEOMDLLIMPEXP GeometryType GetGeometryType() const;

    //! Return true if the geometry is or would be represented by a solid body. Accepted geometry includes BRep solids, capped SolidPrimitves, and closed Polyfaces.
    GEOMDLLIMPEXP bool IsSolid() const;

    //! Return true if the geometry is or would be represented by a sheet body. Accepted geometry includes BRep sheets, un-capped SolidPrimitives, region CurveVectors, Bspline Surfaces, and unclosed Polyfaces.
    GEOMDLLIMPEXP bool IsSheet() const;

    //! Return true if the geometry is or would be represented by a wire body. Accepted geometry includes BRep wires and CurveVectors.
    GEOMDLLIMPEXP bool IsWire() const;

    //! Return the type of solid kernel entity that would be used to represent this geometry.
    GEOMDLLIMPEXP IBRepEntity::EntityType GetBRepEntityType() const;

    GEOMDLLIMPEXP ICurvePrimitivePtr GetAsICurvePrimitive() const;
    GEOMDLLIMPEXP CurveVectorPtr GetAsCurveVector() const;
    GEOMDLLIMPEXP ISolidPrimitivePtr GetAsISolidPrimitive() const;
    GEOMDLLIMPEXP IBRepEntityPtr GetAsIBRepEntity() const;

    GEOMDLLIMPEXP bool GetLocalCoordinateFrame(TransformR localToWorld) const;
    GEOMDLLIMPEXP bool GetLocalRange(DRange3dR localRange, TransformR localToWorld) const; // Expensive - copies geometry!
    GEOMDLLIMPEXP bool GetRange(DRange3dR range, TransformCP transform = nullptr) const;
    GEOMDLLIMPEXP bool TransformInPlace(TransformCR transform);
    GEOMDLLIMPEXP bool IsSameStructureAndGeometry(TopologyPrimitiveCR, double tolerance) const;

    GEOMDLLIMPEXP TopologyPrimitivePtr Clone() const; // Deep copy

    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(ICurvePrimitiveCR source);   //!< Create a TopologyPrimitive from a clone of source
    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(CurveVectorCR source);       //!< Create a TopologyPrimitive from a clone of source
    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(ISolidPrimitiveCR source);   //!< Create a TopologyPrimitive from a clone of source
    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(IBRepEntityCR source);       //!< Create a TopologyPrimitive from a clone of source

    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(ICurvePrimitivePtr const& source);   //!< Create a TopologyPrimitive using source directly
    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(CurveVectorPtr const& source);       //!< Create a TopologyPrimitive using source directly
    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(ISolidPrimitivePtr const& source);   //!< Create a TopologyPrimitive using source directly
    GEOMDLLIMPEXP static TopologyPrimitivePtr Create(IBRepEntityPtr const& source);       //!< Create a TopologyPrimitive using source directly

}; // TopologyPrimitive

//=======================================================================================
//! ISubEntity represents a topological entity that can refer to a
//! single face, edge, or vertex of solid, sheet, or wire body.
//! A sub-entity only remains valid for as long as the parent IBRepEntity exists.
//! Modifications to the parent IBRepEntity may also invalidate a sub-entity,
//! for example, an edge is blended away.
//=======================================================================================
struct ISubEntity : BentleyApi::IRefCounted
{
public:

enum class SubEntityType
    {
    Face    = 0, //!< A single bounded part of a surface.
    Edge    = 1, //!< A single bounded part of a curve.
    Vertex  = 2, //!< A single point.
    };

protected:

//! @private
virtual bool _IsEqual(ISubEntityCR) const = 0;
//! @private
virtual SubEntityType _GetSubEntityType() const = 0;
//! @private
virtual DRange3d _GetSubEntityRange() const = 0;
//! @private
virtual TopologyPrimitiveCPtr _GetGeometry() const = 0;
//! @private
virtual IBRepEntityCPtr _GetParentGeometry() const = 0;
//! @private
virtual bool _SetFaceLocation(DPoint3dCR, DPoint2dCR) = 0;
//! @private
virtual bool _SetEdgeLocation(DPoint3dCR, double) = 0;
//! @private
virtual bool _GetFaceLocation(DPoint3dR, DPoint2dR) const = 0;
//! @private
virtual bool _GetEdgeLocation(DPoint3dR, double&) const = 0;
//! @private
virtual bool _GetVertexLocation(DPoint3dR) const = 0;
//! @private
virtual bool _IsHidden() const = 0;

public:

//! @return Compare sub-entities and return true if they refer to the same topology.
bool IsEqual(ISubEntityCR subEntity) const {return _IsEqual(subEntity);}

//! @return The topology type for this sub-entity.
SubEntityType GetSubEntityType() const {return _GetSubEntityType();}

//! @return The axis aligned bounding box for the sub-entity.
DRange3d GetSubEntityRange() const {return _GetSubEntityRange();}

//! @return A TopologyPrimitive representing the geometry of this sub-entity.
TopologyPrimitiveCPtr GetGeometry() const { return _GetGeometry(); }

//! @return A IBRepEntity for the parent of this sub-entity.
IBRepEntityCPtr GetParentGeometry() const { return _GetParentGeometry(); }

//! Set a location and uv parameter for a sub-entity representing a face.
//! @return false if the sub-entity was not a face.
bool SetFaceLocation(DPoint3dCR point, DPoint2dCR uvParam) {return _SetFaceLocation(point, uvParam);}

//! Set a location and u parameter for a sub-entity representing a edge.
//! @return false if the sub-entity was not an edge.
bool SetEdgeLocation(DPoint3dCR point, double uParam) {return _SetEdgeLocation(point, uParam);}

//! Get pick location and uv parameter from a sub-entity representing a face.
//! @return false if the sub-entity was not created from a method where locate information was meaningful.
bool GetFaceLocation(DPoint3dR point, DPoint2dR uvParam) const {return _GetFaceLocation(point, uvParam);}

//! Get pick location and u parameter from a sub-entity representing a edge.
//! @return false if the sub-entity was not created from a method where locate information was meaningful.
bool GetEdgeLocation(DPoint3dR point, double& uParam) const {return _GetEdgeLocation(point, uParam);}

//! Get vertex location from a sub-entity representing a vertex.
//! @return false if vertex location wasn't set and couldn't be evaluated (ex. input sub-entity wasn't a vertex).
bool GetVertexLocation(DPoint3dR point) const {return _GetVertexLocation(point);}

//! Get location from a sub-entity representing a face, edge, or vertex.
//! @return false if the sub-entity was not created from a method where locate information was meaningful.
bool GetLocation(DPoint3dR point) const
    {
    DPoint2d param;
    switch (GetSubEntityType())
        {
        case ISubEntity::SubEntityType::Face:
            return GetFaceLocation(point, param);

        case ISubEntity::SubEntityType::Edge:
            return GetEdgeLocation(point, param.x);

        case ISubEntity::SubEntityType::Vertex:
            return GetVertexLocation(point);

        default:
            return false;
        }
    }

//! Check the sub-entity for the hidden attribute.
//! @return true if the sub-entity should not be displayed.
bool IsHidden() const {return _IsHidden();};

}; // ISubEntity

END_BENTLEY_GEOMETRY_NAMESPACE
