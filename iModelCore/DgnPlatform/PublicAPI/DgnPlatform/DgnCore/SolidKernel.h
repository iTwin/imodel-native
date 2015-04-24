/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/SolidKernel.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/RefCounted.h>
#include "IViewDraw.h"
#include "IPickGeom.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef RefCountedPtr<ISolidKernelEntity> ISolidKernelEntityPtr; //!< Reference counted type to manage the life-cycle of the ISolidKernelEntity.

//=======================================================================================
//! ISolidKernelEntity represents a boundary representation body (BRep). A BRep is
//! defined by it's topology and geometry. The geometry is the surfaces, curves, and points.
//! The topology is the faces, edges, and vertices that describe how the geometry is connected.
//! The ISolidKernelEntity is used to provide solid modeling functionality such as booleans,
//! blending, and hollowing. Typically only solids and non-planar sheet bodies are persisted
//! as BRep elements. A wire body or planar sheet body can be efficiently represented as
//! a CurveVector.
//! @bsiclass 
//=======================================================================================
struct ISolidKernelEntity : Bentley::IRefCounted
{
public:

enum KernelEntityType
    {
    EntityType_Solid   = 0, //!< Body consisting of at least one solid region.
    EntityType_Sheet   = 1, //!< Body consisting of connected sets of faces having edges that are shared by a maximum of two faces. 
    EntityType_Wire    = 2, //!< Body consisting of connected sets of edges having vertices that are shared by a maximum of two edges.
    EntityType_Minimal = 3, //!< Body consisting of a single vertex.
    };

//__PUBLISH_SECTION_END__
//__PUBLISH_SCOPE_1_START__
//! For use with SolidsKernelAdmin::_SaveEntityToMemory/_RestoreEntityFromMemory for persistent storage.
enum SolidKernelType
    {
    SolidKernel_PSolid = 0, //!< Parasolid kernel BRep.
    SolidKernel_ACIS   = 1, //!< ACIS kernel BRep.
    };

//! For use with SolidsKernelAdmin::_SaveEntityToMemory/_RestoreEntityFromMemory for persistent storage.
enum SolidDataVersion
    {
    DataVersion_PSolid = 120, //!< Transmit schema version used to persist Parasolid data in a dgn file.
    DataVersion_ACIS   = 600, //!< Value to derive version used to persist ACIS data in a dgn file.
    };

//! For use with SolidsKernelAdmin::_QueryEntityData to answer simple yes/no queries.
enum KernelEntityQuery
    {
    EntityQuery_HasCurvedFaceOrEdge = 0, //!< Check if body has only planar faces and linear edges.
    EntityQuery_HasHiddenEdge       = 1, //!< Check if body has at least one edge with hidden attribute.
    EntityQuery_HasHiddenFace       = 2, //!< Check if body has at least one face with hidden attribute.
    EntityQuery_HasOnlyPlanarFaces  = 3, //!< Check if body has only planar faces.
    };

protected:

virtual bool                    _IsEqual (ISolidKernelEntityCR) const = 0;
virtual KernelEntityType        _GetEntityType () const = 0;
virtual TransformCR             _GetEntityTransform () const = 0;
virtual void                    _SetEntityTransform (TransformCR) = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Compare entities to see if they refer to the same body.
//! @return true if entities are equal.
DGNPLATFORM_EXPORT bool IsEqual (ISolidKernelEntityCR) const;

//! Get the body type for this entity.
//! @return The solid kernel entity type.
DGNPLATFORM_EXPORT KernelEntityType GetEntityType () const;

//! Get the body to uor transform for this entity. This transform can have
//! translation, rotation, and scale. A distance of 1.0 in solid kernel coordinates
//! represents 1 meter, how many uors this represents is determined by the transform scale.
//! Typically the body to uor scale comes from the model's unit settings (ModelInfo::GetSolidExtent).
//! In order to achieve a precision of 1.0e-8 for modeling operations, a single body must 
//! fit inside a 1 km box centered at the origin (in solid kernel coordinates not uors).
//! In order to perform a boolean operation between 2 bodies, the position of the tool body 
//! relative to the target must also be within this 1 km size box. The body to uor translation 
//! allows for a solid that will typically have a basis point of 0,0,0 to be displayed at 
//! any uor location in a dgn model.
//! @return The solid kernel to uor transform for the entity.
DGNPLATFORM_EXPORT TransformCR GetEntityTransform () const;

//! Changes the solid to uor transform for the entity.
//! @param[in] transform The new solid to uor transform.
DGNPLATFORM_EXPORT void SetEntityTransform (TransformCR transform);

//! PreMultiply the entity transform by the supplied (solid) transform
//! @param[in] uorTransform The transform to pre-multiply.
DGNPLATFORM_EXPORT void PreMultiplyEntityTransformInPlace (TransformCR uorTransform);

//! PostMultiply the entity transform by the supplied (solid) transform
//! @param[in] solidTransform The transform to post-multiply.
DGNPLATFORM_EXPORT void PostMultiplyEntityTransformInPlace (TransformCR solidTransform);

//! Create deep copy of this ISolidKernelEntity.
DGNPLATFORM_EXPORT ISolidKernelEntityPtr Clone () const;

}; // ISolidKernelEntity

typedef RefCountedPtr<ISubEntity> ISubEntityPtr; //!< Reference counted type to manage the life-cycle of the ISubEntity.

//=======================================================================================
//! ISubEntity represents a topological BRep entity. The ISubEntity can refer to a
//! single face, edge, or vertex of a solid, sheet, or wire body. A sub-entity only
//! remains valid for as long as the ISolidKernelEntity exists. Modifications to the
//! ISolidKernelEntity may also invalidate a sub-entity, ex. edge is blended away.
//! @bsiclass 
//=======================================================================================
struct ISubEntity : Bentley::IRefCounted
{
public:

enum SubEntityType
    {
    SubEntityType_Face    = (1 << 0), //!< A single bounded part of a surface.
    SubEntityType_Edge    = (1 << 1), //!< A single bounded part of a curve.
    SubEntityType_Vertex  = (1 << 2), //!< A single point.
    };

//__PUBLISH_SECTION_END__
//__PUBLISH_SCOPE_1_START__
//! For use with SolidsKernelAdmin::_QuerySubEntityData to answer simple yes/no queries.
enum SubEntityQuery
    {
    SubEntityQuery_IsPlanarFace = 1, //!< Check if face sub-entity has as planar surface.
    SubEntityQuery_IsSmoothEdge = 2, //!< Check if edge sub-entity is smooth by comparing the face normals along the edge.
    };

protected:

virtual BentleyStatus   _ToString (WStringR subEntityStr) const = 0;
virtual bool            _IsEqual (ISubEntityCR) const = 0;
virtual SubEntityType   _GetSubEntityType () const = 0;

public:

//! Represent the sub-entity as a persistent topological id string.
DGNPLATFORM_EXPORT BentleyStatus ToString (WStringR subEntityStr) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

//! Compare sub-entities to see if they refer to the same topology.
//! @return true if entities are equal.
DGNPLATFORM_EXPORT bool IsEqual (ISubEntityCR) const;

//! Get the topology type for this sub-entity.
//! @return The sub-entity type.
DGNPLATFORM_EXPORT SubEntityType GetSubEntityType () const;

}; // ISubEntity

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass 
//=======================================================================================
struct BRepSubEntityTopo : IElemTopology
{
uint32_t m_entityTag;

BRepSubEntityTopo () {m_entityTag = 0;}
explicit BRepSubEntityTopo (BRepSubEntityTopo const& from) {m_entityTag = from.m_entityTag;}
virtual BRepSubEntityTopo* _Clone () const override {return new BRepSubEntityTopo (*this);}
}; // BRepSubEntityTopo
//__PUBLISH_SECTION_START__

//__PUBLISH_SECTION_END__
//__PUBLISH_SCOPE_1_START__
//=======================================================================================
// @bsiclass Facet table per-face material and color inforation.
//=======================================================================================
struct FaceAttachment
{
private:

DgnCategoryId   m_category;
ColorDef        m_color;
double          m_transparency;
MaterialCP      m_material;

public:

DGNPLATFORM_EXPORT FaceAttachment ();
DGNPLATFORM_EXPORT FaceAttachment (ElemDisplayParamsCR, ViewContextR);

DGNPLATFORM_EXPORT void ToElemDisplayParams (ElemDisplayParamsR) const; // NOTE: ElemDisplayParams should be initialized from ViewContext::GetCurrentDisplayParams if you need anything more than color/transparency/level.
DGNPLATFORM_EXPORT void ToElemMatSymb (ElemMatSymbR) const; // NOTE: For QvOutput use only, other callers should use ToElemDisplayParams and FromElemDisplayParams/CookDisplayParams.

DGNPLATFORM_EXPORT bool operator== (struct FaceAttachment const&) const;
DGNPLATFORM_EXPORT bool operator< (struct FaceAttachment const&) const;

}; // FaceAttachment

typedef bmap <int, FaceAttachment> T_FaceAttachmentsMap;
typedef bmap <int, int> T_FaceToSubElemIdMap;

//__PUBLISH_SECTION_START__
//=======================================================================================
// Per-face material and color override
//=======================================================================================
struct IFaceMaterialAttachments : public IRefCounted
{
//__PUBLISH_SECTION_END__
//__PUBLISH_SCOPE_1_START__
virtual T_FaceToSubElemIdMap const& _GetFaceToSubElemIdMap () const = 0;
virtual T_FaceAttachmentsMap const& _GetFaceAttachmentsMap () const = 0;

virtual T_FaceToSubElemIdMap& _GetFaceToSubElemIdMapR () = 0;
virtual T_FaceAttachmentsMap& _GetFaceAttachmentsMapR () = 0;
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
};

typedef RefCountedPtr<IFaceMaterialAttachments> IFaceMaterialAttachmentsPtr;

//__PUBLISH_SECTION_END__
//__PUBLISH_SCOPE_1_START__
//=======================================================================================
// @bsiclass Wrapper class around facets that at least act like Parasold fin tables.
//=======================================================================================
struct IFacetTopologyTable : Bentley::IRefCounted
{
public:

virtual bool        _IsTableValid () = 0;
virtual int         _GetFacetCount () = 0;
virtual int         _GetFinCount () = 0;

virtual DPoint3dCP  _GetPoint () = 0;
virtual int         _GetPointCount () = 0;

virtual int const*  _GetPointIndex () = 0;
virtual int         _GetPointIndexCount () = 0;

virtual DVec3dCP    _GetNormal () = 0;
virtual int         _GetNormalCount () = 0;

virtual int const*  _GetNormalIndex () = 0;
virtual int         _GetNormalIndexCount () = 0;

virtual DPoint2dCP  _GetParamUV () = 0;
virtual int         _GetParamUVCount () = 0;

virtual int const*  _GetParamUVIndex () = 0;
virtual int         _GetParamUVIndexCount () = 0;

virtual int const*  _GetFinData () = 0;
virtual int         _GetFinDataCount () = 0;

virtual int const*  _GetFinFin () = 0;
virtual int         _GetFinFinCount () = 0;

virtual Point2dCP   _GetFacetFin () = 0;
virtual int         _GetFacetFinCount () = 0;

virtual Point2dCP   _GetStripFin () = 0;
virtual int         _GetStripFinCount () = 0;

virtual int const*  _GetStripFaceId () = 0;         // Array of face subElemId (body face index)
virtual int         _GetStripFaceIdCount () = 0;

virtual Point2dCP   _GetFinEdge () = 0;             // NOTE: Parasolid only hidden edge support (fin index, edge tag pair)
virtual int         _GetFinEdgeCount () = 0;

virtual int const*  _GetFacetFace () = 0;           // NOTE: Parasolid only hidden face support (array of face tags)
virtual int         _GetFacetFaceCount () = 0;

virtual bool        _GetEdgeCurveId (CurveTopologyId& edgeId, int32_t edge, bool useHighestId) = 0; 

virtual bool        _IsHiddenFace (int32_t entityTag) = 0;
virtual bool        _IsHiddenEdge (int32_t entityTag) = 0;

virtual T_FaceToSubElemIdMap const* _GetFaceToSubElemIdMap () = 0;
virtual T_FaceAttachmentsMap const* _GetFaceAttachmentsMap () = 0;

public:

//! Translate from IFacetTopologyTable to PolyfaceHeader.
//! @param [in,out] polyface polyface data.  Prior contents are cleared.
//! @param [in] ftt Facet topology table
//! @param [in] facetOptions Facet options
DGNPLATFORM_EXPORT static StatusInt ConvertToPolyface
(
PolyfaceHeaderR         polyface,
IFacetTopologyTable&    ftt,
IFacetOptionsCR         facetOptions
);

//! Translate from IFacetTopologyTable to multi symbology polyfaces with face ids.
//! @param [in,out] polyfaces Map from face index to the polyfaces.
//! @param [in] facePolyfaces Face color/material attachments nap.
//! @param [in] ftt Facet topology table
//! @param [in] facetOptions Facet options
DGNPLATFORM_EXPORT static StatusInt ConvertToPolyfaces
(
bvector <PolyfaceHeaderPtr>&    polyfaces,
bmap <int, PolyfaceHeaderCP>&   facePolyfaces,
IFacetTopologyTable&            ftt,
IFacetOptionsCR                 facetOptions
);

}; // IFacetTopologyTable

typedef RefCountedPtr<IFacetTopologyTable> IFacetTopologyTablePtr;

//__PUBLISH_SECTION_START__
END_BENTLEY_DGNPLATFORM_NAMESPACE
