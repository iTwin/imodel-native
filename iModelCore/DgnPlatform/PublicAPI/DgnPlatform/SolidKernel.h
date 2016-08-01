/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SolidKernel.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Render.h"

class TopoDS_Shape;

BEGIN_BENTLEY_DGN_NAMESPACE

typedef RefCountedPtr<IFaceMaterialAttachments> IFaceMaterialAttachmentsPtr; //!< Reference counted type to manage the life-cycle of the IFaceMaterialAttachments.

//=======================================================================================
//! @private
//! Facet table per-face material and color inforation.
//=======================================================================================
struct FaceAttachment
{
private:
    bool                m_useColor:1;       //!< true - color does not follow sub-category appearance.
    bool                m_useMaterial:1;    //!< true - material does not follow sub-category appearance.
    DgnCategoryId       m_categoryId;       //!< in memory only, can't change from element...
    DgnSubCategoryId    m_subCategoryId;    //!< in memory only, can't change per-face...
    ColorDef            m_color;
    double              m_transparency;
    DgnMaterialId       m_material;
    DPoint2d            m_uv;

public:

DGNPLATFORM_EXPORT FaceAttachment ();
DGNPLATFORM_EXPORT FaceAttachment (Render::GeometryParamsCR);

//! Input GeometryParams should be initialized from ViewContext::GetCurrentGeometryParams for anything other than color, transparency, material.
DGNPLATFORM_EXPORT void ToGeometryParams (Render::GeometryParamsR) const; 

DGNPLATFORM_EXPORT bool operator== (struct FaceAttachment const&) const;
DGNPLATFORM_EXPORT bool operator< (struct FaceAttachment const&) const;

}; // FaceAttachment

//! @private
typedef bvector<FaceAttachment> T_FaceAttachmentsVec; //!< Unique face attachments - first entry is "base" symbology
//! @private
typedef bpair<int32_t, size_t> T_SubElemIdAttachmentIndexPair; //!< subElemid/attachment index pair
//! @private
typedef bmap<uint32_t, T_SubElemIdAttachmentIndexPair> T_FaceToSubElemIdMap; //!< Face identifier to subElemId/attachment index pair

//=======================================================================================
//! @private
//! Per-face material and color override
//=======================================================================================
struct IFaceMaterialAttachments : public IRefCounted
{
virtual T_FaceAttachmentsVec const& _GetFaceAttachmentsVec() const = 0;
virtual T_FaceToSubElemIdMap const& _GetFaceToSubElemIdMap() const = 0;

virtual T_FaceAttachmentsVec& _GetFaceAttachmentsVecR() = 0;
virtual T_FaceToSubElemIdMap& _GetFaceToSubElemIdMapR() = 0;
};

typedef RefCountedPtr<ISolidKernelEntity> ISolidKernelEntityPtr; //!< Reference counted type to manage the life-cycle of the ISolidKernelEntity.

//=======================================================================================
//! ISolidKernelEntity represents a boundary representation body (BRep). A BRep is
//! defined by it's topology and geometry. The geometry is the surfaces, curves, and points.
//! The topology is the faces, edges, and vertices that describe how the geometry is connected.
//! The ISolidKernelEntity is used to provide solid modeling functionality such as booleans,
//! blending, and hollowing. Typically only solids and non-planar sheet bodies are persisted
//! as BRep elements. A wire body or planar sheet body can be efficiently represented as
//! a CurveVector.
//=======================================================================================
struct ISolidKernelEntity : BentleyApi::IRefCounted
{
public:

enum KernelEntityType
    {
    EntityType_Solid    = 0, //!< Body consisting of at least one solid region.
    EntityType_Sheet    = 1, //!< Body consisting of connected sets of faces having edges that are shared by a maximum of two faces. 
    EntityType_Wire     = 2, //!< Body consisting of connected sets of edges having vertices that are shared by a maximum of two edges.
    EntityType_Minimal  = 3, //!< Body consisting of a single vertex.
    EntityType_Compound = 4, //!< Body consisting of a non-homogeneous group of solids, sheets, and wires.
    };

protected:

//! @private
virtual bool _IsEqual(ISolidKernelEntityCR) const = 0;
//! @private
virtual KernelEntityType _GetEntityType() const = 0;
//! @private
virtual DRange3d _GetEntityRange() const = 0;
//! @private
virtual Transform _GetEntityTransform() const = 0;
//! @private
virtual void _SetEntityTransform(TransformCR) = 0;
//! @private
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const = 0;
//! @private
virtual bool _InitFaceMaterialAttachments(Render::GeometryParamsCP) = 0;
//! @private
virtual ISolidKernelEntityPtr _Clone() const = 0;

public:

//! Compare entities to see if they refer to the same body.
//! @return true if entities are equal.
bool IsEqual (ISolidKernelEntityCR entity) const {return _IsEqual(entity);}

//! Get the body type for this entity.
//! @return The solid kernel entity type.
KernelEntityType GetEntityType() const {return _GetEntityType();}

//! Get the axis aligned bounding box for this entity.
//! @return The axis aligned bounding box for the entity.
DRange3d GetEntityRange() const {return _GetEntityRange();}

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
Transform GetEntityTransform() const {return _GetEntityTransform();}

//! Changes the solid to uor transform for the entity.
//! @param[in] transform The new solid to uor transform.
void SetEntityTransform (TransformCR transform) {return _SetEntityTransform(transform);}

//! PreMultiply the entity transform by the supplied (solid) transform
//! @param[in] uorTransform The transform to pre-multiply.
void PreMultiplyEntityTransformInPlace (TransformCR uorTransform) {_SetEntityTransform(Transform::FromProduct(uorTransform, _GetEntityTransform()));}

//! PostMultiply the entity transform by the supplied (solid) transform
//! @param[in] solidTransform The transform to post-multiply.
void PostMultiplyEntityTransformInPlace (TransformCR solidTransform) {_SetEntityTransform(Transform::FromProduct(_GetEntityTransform(), solidTransform));}

//! Optional per-face color/material overrides.
IFaceMaterialAttachmentsCP GetFaceMaterialAttachments() const {return _GetFaceMaterialAttachments();}

//! Initialize per-face color/material using the supplied GeometryParams or clear if nullptr.
bool InitFaceMaterialAttachments(Render::GeometryParamsCP baseParams) {return _InitFaceMaterialAttachments(baseParams);}

//! Create deep copy of this ISolidKernelEntity.
ISolidKernelEntityPtr Clone() const {return _Clone();}

}; // ISolidKernelEntity

typedef RefCountedPtr<ISubEntity> ISubEntityPtr; //!< Reference counted type to manage the life-cycle of the ISubEntity.

//=======================================================================================
//! ISubEntity represents a topological BRep entity. The ISubEntity can refer to a
//! single face, edge, or vertex of a solid, sheet, or wire body. A sub-entity only
//! remains valid for as long as the ISolidKernelEntity exists. Modifications to the
//! ISolidKernelEntity may also invalidate a sub-entity, ex. edge is blended away.
//=======================================================================================
struct ISubEntity : BentleyApi::IRefCounted
{
public:

enum SubEntityType
    {
    SubEntityType_Face    = (1 << 0), //!< A single bounded part of a surface.
    SubEntityType_Edge    = (1 << 1), //!< A single bounded part of a curve.
    SubEntityType_Vertex  = (1 << 2), //!< A single point.
    };

protected:

//! @private
virtual bool _IsEqual (ISubEntityCR) const = 0;
//! @private
virtual SubEntityType _GetSubEntityType() const = 0;

public:

//! Compare sub-entities to see if they refer to the same topology.
//! @return true if entities are equal.
bool IsEqual (ISubEntityCR subEntity) const {return _IsEqual(subEntity);}

//! Get the topology type for this sub-entity.
//! @return The sub-entity type.
SubEntityType GetSubEntityType() const {return _GetSubEntityType();}

}; // ISubEntity

//=======================================================================================
//! SolidKernelUtil is intended as a bridge between DgnPlatform and Open CASCADE so
//! that the entire set of Open CASCADE includes isn't required for the published api.
//=======================================================================================
struct SolidKernelUtil
{
DGNPLATFORM_EXPORT static ISolidKernelEntityPtr CreateNewEntity(TopoDS_Shape const&);
DGNPLATFORM_EXPORT static TopoDS_Shape const* GetShape(ISolidKernelEntityCR);
DGNPLATFORM_EXPORT static TopoDS_Shape* GetShapeP(ISolidKernelEntityR);
DGNPLATFORM_EXPORT static PolyfaceHeaderPtr FacetEntity(ISolidKernelEntityCR, double pixelSize=0.0, DRange1dP pixelSizeRange=nullptr);
};

END_BENTLEY_DGN_NAMESPACE
