/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SolidKernel.h $
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
DGNPLATFORM_EXPORT FaceAttachment (ElemDisplayParamsCR);

//! Input ElemDisplayParams should be initialized from ViewContext::GetCurrentDisplayParams for anything other than color, transparency, material.
DGNPLATFORM_EXPORT void ToElemDisplayParams (ElemDisplayParamsR) const; 

//! @private For QvOutput use only, other callers should use ToElemDisplayParams.
DGNPLATFORM_EXPORT void ToElemMatSymb (ElemMatSymbR, DgnViewportR) const;

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
    EntityType_Solid   = 0, //!< Body consisting of at least one solid region.
    EntityType_Sheet   = 1, //!< Body consisting of connected sets of faces having edges that are shared by a maximum of two faces. 
    EntityType_Wire    = 2, //!< Body consisting of connected sets of edges having vertices that are shared by a maximum of two edges.
    EntityType_Minimal = 3, //!< Body consisting of a single vertex.
    };

protected:

//! @private
virtual bool _IsEqual (ISolidKernelEntityCR) const = 0;
//! @private
virtual KernelEntityType _GetEntityType () const = 0;
//! @private
virtual TransformCR _GetEntityTransform () const = 0;
//! @private
virtual void _SetEntityTransform (TransformCR) = 0;
//! @private
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const = 0;
//! @private
virtual bool _InitFaceMaterialAttachments(ElemDisplayParamsCP) = 0;
//! @private
virtual ISolidKernelEntityPtr _Clone() const = 0;

public:

//! Compare entities to see if they refer to the same body.
//! @return true if entities are equal.
bool IsEqual (ISolidKernelEntityCR entity) const {return _IsEqual(entity);}

//! Get the body type for this entity.
//! @return The solid kernel entity type.
KernelEntityType GetEntityType() const {return _GetEntityType();}

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
TransformCR GetEntityTransform() const {return _GetEntityTransform();}

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

//! Initialize per-face color/material using the supplied ElemDisplayParams or clear if nullptr.
bool InitFaceMaterialAttachments(ElemDisplayParamsCP baseParams) {return _InitFaceMaterialAttachments(baseParams);}

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
//! @private
//! Wrapper class around facets that at least act like Parasold fin tables.
//=======================================================================================
struct IFacetTopologyTable : BentleyApi::IRefCounted
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

virtual T_FaceAttachmentsVec const* _GetFaceAttachmentsVec () = 0;
virtual T_FaceToSubElemIdMap const* _GetFaceToSubElemIdMap () = 0;

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
bvector<PolyfaceHeaderPtr>&     polyfaces,
bmap<int, PolyfaceHeaderCP>&    facePolyfaces,
IFacetTopologyTable&            ftt,
IFacetOptionsCR                 facetOptions
);

}; // IFacetTopologyTable

typedef RefCountedPtr<IFacetTopologyTable> IFacetTopologyTablePtr;

END_BENTLEY_DGNPLATFORM_NAMESPACE
