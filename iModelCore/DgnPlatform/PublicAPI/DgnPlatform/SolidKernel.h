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

#if defined (BENTLEYCONFIG_OPENCASCADE) 
class TopoDS_Shape;
#endif

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
virtual Transform _GetEntityTransform() const = 0;
//! @private
virtual bool _SetEntityTransform(TransformCR) = 0;
//! @private
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const = 0;
//! @private
virtual bool _InitFaceMaterialAttachments(Render::GeometryParamsCP) = 0;
//! @private
virtual IBRepEntityPtr _Clone() const = 0;

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

//! Get the solid to world transform for this entity.
//! The solid to world translation allows for a solid that will typically have a basis 
//! point of 0,0,0 to be displayed at any world location in the model.
//! @return The solid to world transform for the entity.
Transform GetEntityTransform() const {return _GetEntityTransform();}

//! Changes the solid to world transform for the entity.
//! @param[in] transform The new solid to uor transform.
//! @note For a scaled transform, BRepBuilderAPI_Transform or BRepBuilderAPI_GTransform will be used to modify the shape.
//!       For translation/rotation only, only the TopoDS_Shape::Location will be set to the supplied transform.
bool SetEntityTransform (TransformCR transform) {return _SetEntityTransform(transform);}

//! PreMultiply the entity transform by the supplied (solid) transform
//! @param[in] uorTransform The transform to pre-multiply.
bool PreMultiplyEntityTransformInPlace (TransformCR uorTransform) {return _SetEntityTransform(Transform::FromProduct(uorTransform, _GetEntityTransform()));}

//! PostMultiply the entity transform by the supplied (solid) transform
//! @param[in] solidTransform The transform to post-multiply.
bool PostMultiplyEntityTransformInPlace (TransformCR solidTransform) {return _SetEntityTransform(Transform::FromProduct(_GetEntityTransform(), solidTransform));}

//! Optional per-face color/material overrides.
IFaceMaterialAttachmentsCP GetFaceMaterialAttachments() const {return _GetFaceMaterialAttachments();}

//! Initialize per-face color/material using the supplied GeometryParams or clear if nullptr.
bool InitFaceMaterialAttachments(Render::GeometryParamsCP baseParams) {return _InitFaceMaterialAttachments(baseParams);}

//! Create deep copy of this IBRepEntity.
IBRepEntityPtr Clone() const {return _Clone();}

}; // IBRepEntity

//=======================================================================================
//! ISubEntity represents a topological entity that can refer to a
//! single face, edge, or vertex of solid, sheet, or wire GeometricPrimitive.
//! A sub-entity only remains valid for as long as the parent GeometricPrimitive exists.
//! Modifications to the parent GeometricPrimitive may also invalidate a sub-entity, 
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
virtual GeometricPrimitiveCPtr _GetGeometry() const = 0;
//! @private
virtual GeometricPrimitiveCPtr _GetParentGeometry() const = 0;
//! @private
virtual Render::GraphicBuilderPtr _GetGraphic(ViewContextR) const = 0;

public:

//! @return Compare sub-entities and return true if they refer to the same topology.
bool IsEqual(ISubEntityCR subEntity) const {return _IsEqual(subEntity);}

//! @return The topology type for this sub-entity.
SubEntityType GetSubEntityType() const {return _GetSubEntityType();}

//! @return The axis aligned bounding box for the sub-entity.
DRange3d GetSubEntityRange() const {return _GetSubEntityRange();}

//! @return A GeometricPrimitive representing the geometry of this sub-entity.
GeometricPrimitiveCPtr GetGeometry() const {return _GetGeometry();}

//! @return A GeometricPrimitive for the parent of this sub-entity.
GeometricPrimitiveCPtr GetParentGeometry() const {return _GetParentGeometry();}

//! @return A Render::Graphic representing this sub-entity.
Render::GraphicBuilderPtr GetGraphic(ViewContextR context) const {return _GetGraphic(context);}

}; // ISubEntity

#if defined (BENTLEYCONFIG_OPENCASCADE)    
//=======================================================================================
//! SolidKernelUtil is intended as a bridge between DgnPlatform and the solid kernel so
//! that the entire set of solid kernel includes isn't required for the published api.
//=======================================================================================
struct SolidKernelUtil
{
DGNPLATFORM_EXPORT static IBRepEntityPtr CreateNewEntity(TopoDS_Shape const&); //!< NOTE: Will return an invalid entity if supplied shape is an empty compound, caller should check IsValid.
DGNPLATFORM_EXPORT static TopoDS_Shape const* GetShape(IBRepEntityCR);
DGNPLATFORM_EXPORT static TopoDS_Shape* GetShapeP(IBRepEntityR);
}; // SolidKernelUtil
#endif

//=======================================================================================
//! BRepUtil provides support for the creation, querying, and modification of BReps.
//! Coordinates and distances are always supplied and returned in uors. Operations between 
//! entities such as BRepUtil::Modify::BooleanUnion will automatically take the 
//! individual target and tool entity transforms into account.
//! @bsiclass                                                   Brien.Bastings  07/12
//=======================================================================================
struct BRepUtil
{
DGNPLATFORM_EXPORT static PolyfaceHeaderPtr FacetEntity(IBRepEntityCR, double pixelSize=0.0, DRange1dP pixelSizeRange=nullptr);
DGNPLATFORM_EXPORT static PolyfaceHeaderPtr FacetEntity(IBRepEntityCR, IFacetOptionsR);
DGNPLATFORM_EXPORT static bool FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<Render::GeometryParams>& params, double pixelSize=0.0, DRange1dP pixelSizeRange=nullptr);
DGNPLATFORM_EXPORT static bool FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<Render::GeometryParams>& params, IFacetOptionsR facetOptions);
DGNPLATFORM_EXPORT static bool HasCurvedFaceOrEdge(IBRepEntityCR);

//! Support for modification of bodies.
struct Modify
    {
    //! Modify the target body by intersecting with one or more tool bodies.
    //! @param[in,out] target The target body to modify.
    //! @param[in,out] tools A list of one or more tool bodies (consumed in boolean).
    //! @param[in] nTools Count of tool bodies.
    //! @return SUCCESS if boolean operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus BooleanIntersect (IBRepEntityPtr& target, IBRepEntityPtr* tools, size_t nTools);

    //! Modify the target body by subtracting one or more tool bodies.
    //! @param[in,out] target The target body to modify.
    //! @param[in,out] tools Array of one or more tool bodies (consumed in boolean).
    //! @param[in] nTools Count of tool bodies.
    //! @return SUCCESS if boolean operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus BooleanSubtract (IBRepEntityPtr& target, IBRepEntityPtr* tools, size_t nTools);

    //! Modify the target body by uniting with one or more tool bodies.
    //! @param[in,out] target The target body to modify.
    //! @param[in,out] tools Array of one or more tool bodies (consumed in boolean).
    //! @param[in] nTools Count of tool bodies.
    //! @return SUCCESS if boolean operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus BooleanUnion (IBRepEntityPtr& target, IBRepEntityPtr* tools, size_t nTools);

    //! Sew the given set of sheet bodies together by joining those that share edges in common.
    //! @param[out] sewn The new bodies produced by sewing.
    //! @param[out] unsewn The bodies that were not able to be sewn.
    //! @param[in,out] tools The array of sheet bodies. (invalidated after sew).
    //! @param[in] nTools Count of tool bodies.
    //! @param[in] gapWidthBound Defines a limit on the width of the gap between sheet body edges that will be allowed to remain.
    //! @param[in] nIterations To request repeated sew attempts that automatically increase gap up to limit set by gapWidthBound.
    //! @return SUCCESS if some bodies were able to be sewn together.
    DGNPLATFORM_EXPORT static BentleyStatus SewBodies (bvector<IBRepEntityPtr>& sewn, bvector<IBRepEntityPtr>& unsewn, IBRepEntityPtr* tools, size_t nTools, double gapWidthBound, size_t nIterations = 1);
    };

}; // BRepUtil

END_BENTLEY_DGN_NAMESPACE
