/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Render.h"

BEGIN_BENTLEY_DGN_NAMESPACE

typedef RefCountedPtr<IFaceMaterialAttachments> IFaceMaterialAttachmentsPtr; //!< Reference counted type to manage the life-cycle of the IFaceMaterialAttachments.

//=======================================================================================
//! @private
//! Facet table per-face material and color information.
//=======================================================================================
struct FaceAttachment
{
private:

    bool                m_useColor:1;       //!< true - color/transparency does not follow sub-category appearance.
    bool                m_useMaterial:1;    //!< true - material does not follow sub-category appearance.
    ColorDef            m_color;
    double              m_transparency;
    RenderMaterialId    m_material;
    DPoint2d            m_uv;

    mutable bool m_haveGraphicParams = false;
    mutable Render::GraphicParams m_graphicParams; //!< in memory only, resolved color/transparency/material...

public:

DGNPLATFORM_EXPORT FaceAttachment();
DGNPLATFORM_EXPORT FaceAttachment(Render::GeometryParamsCR sourceParams);

DGNPLATFORM_EXPORT bool operator == (struct FaceAttachment const&) const;
DGNPLATFORM_EXPORT bool operator < (struct FaceAttachment const&) const;

//! Return GraphicParams from prior call to CookFaceAttachment. Will return nullptr if CookFaceAttachment has not been called.
Render::GraphicParamsCP GetGraphicParams() const {return (m_haveGraphicParams ? &m_graphicParams : nullptr);}

//! Cook and resolve FaceAttachment color and material. The base GeometryParams is required to supply the information that can't vary by face, like DgnSubCategoryId.
DGNPLATFORM_EXPORT void CookFaceAttachment(ViewContextR, Render::GeometryParamsCR baseParams) const;

//! Represent this FaceAttachment as a GeometryParams. The base GeometryParams is required to supply the information that can't vary by face, like DgnSubCategoryId.
DGNPLATFORM_EXPORT void ToGeometryParams(Render::GeometryParamsR faceParams, Render::GeometryParamsCR baseParams) const;

}; // FaceAttachment

//! @private
typedef bvector<FaceAttachment> T_FaceAttachmentsVec; //!< Unique face attachments - first entry is "base" symbology
typedef bmap<uint32_t, size_t> T_FaceToAttachmentIndexMap; //!< Face identifier to attachment index map

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
virtual Transform _GetEntityTransform() const = 0;
//! @private
virtual bool _SetEntityTransform(TransformCR) = 0;
//! @private
virtual IFaceMaterialAttachmentsCP _GetFaceMaterialAttachments() const = 0;
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
virtual bool _IsParentEqual(GeometricPrimitiveCR) const = 0;
//! @private
virtual SubEntityType _GetSubEntityType() const = 0;
//! @private
virtual DRange3d _GetSubEntityRange() const = 0;
//! @private
virtual GeometricPrimitiveCPtr _GetGeometry() const = 0;
//! @private
virtual GeometricPrimitiveCPtr _GetParentGeometry() const = 0;
//! @private
virtual Render::GraphicPtr _GetGraphic(ViewContextR) const = 0;
//! @private
virtual bool _GetFaceLocation(DPoint3dR, DPoint2dR) const = 0;
//! @private
virtual bool _GetEdgeLocation(DPoint3dR, double&) const = 0;
//! @private
virtual bool _GetVertexLocation(DPoint3dR) const = 0;

public:

//! @return Compare sub-entities and return true if they refer to the same topology.
bool IsEqual(ISubEntityCR subEntity) const {return _IsEqual(subEntity);}

//! @return Whether this sub-entity is from the input parent geometry.
bool IsParentEqual(GeometricPrimitiveCR parent) const {return _IsParentEqual(parent);}

//! @return The topology type for this sub-entity.
SubEntityType GetSubEntityType() const {return _GetSubEntityType();}

//! @return The axis aligned bounding box for the sub-entity.
DRange3d GetSubEntityRange() const {return _GetSubEntityRange();}

//! @return A GeometricPrimitive representing the geometry of this sub-entity.
GeometricPrimitiveCPtr GetGeometry() const {return _GetGeometry();}

//! @return A GeometricPrimitive for the parent of this sub-entity.
GeometricPrimitiveCPtr GetParentGeometry() const {return _GetParentGeometry();}

//! @return A Render::Graphic representing this sub-entity.
Render::GraphicPtr GetGraphic(ViewContextR context) const {return _GetGraphic(context);}

//! Get pick location and uv parameter from a sub-entity representing a face. 
//! @return false if the sub-entity was not created from a method where locate information was meaningful.
bool GetFaceLocation(DPoint3dR point, DPoint2dR uvParam) const {return _GetFaceLocation(point, uvParam);}

//! Get pick location and u parameter from a sub-entity representing a edge. 
//! @return false if the sub-entity was not created from a method where locate information was meaningful.
bool GetEdgeLocation(DPoint3dR point, double& uParam) const {return _GetEdgeLocation(point, uParam);}

//! Get vertex location from a sub-entity representing a vertex. 
//! @return false if vertex location wasn't set and couldn't be evaluated (ex. input sub-entity wasn't a vertex).
bool GetVertexLocation(DPoint3dR point) const {return _GetVertexLocation(point);}

}; // ISubEntity

//=======================================================================================
// @bsiclass 
//=======================================================================================
struct IsSubEntityPtrEqual : std::binary_function <ISubEntityPtr, ISubEntityCP, bool>
    {
    bool operator() (ISubEntityPtr const& subEntityPtr, ISubEntityCP subEntity) const {return subEntityPtr->IsEqual(*subEntity);}
    };

//=======================================================================================
// @bsiclass 
//=======================================================================================
struct IsParentGeometryPtrEqual : std::binary_function <ISubEntityPtr, GeometricPrimitiveCP, bool>
    {
    bool operator() (ISubEntityPtr const& subEntity, GeometricPrimitiveCP geom) const {return subEntity->IsParentEqual(*geom);}
    };

//=======================================================================================
//! BRepUtil provides support for the creation, querying, and modification of BReps.
//! Coordinates and distances are always supplied and returned in uors. Operations between 
//! entities such as BRepUtil::Modify::BooleanUnion will automatically take the 
//! individual target and tool entity transforms into account.
//! @bsiclass                                                   Brien.Bastings  07/12
//=======================================================================================
struct BRepUtil
{
//! Query the set of faces of the input body.
//! @param[out] subEntities An optional vector to hold the sub-entities of type SubEntityType::Face, pass NULL if just interested in count.
//! @param[in] in The entity to query.
//! @return A count of the number of faces.
DGNPLATFORM_EXPORT static size_t GetBodyFaces(bvector<ISubEntityPtr>* subEntities, IBRepEntityCR in);

//! Query the set of edges of the input body.
//! @param[out] subEntities An optional vector to hold the sub-entities of type SubEntityType::Edge, pass NULL if just interested in count.
//! @param[in] in The entity to query.
//! @return A count of the number of edges.
DGNPLATFORM_EXPORT static size_t GetBodyEdges(bvector<ISubEntityPtr>* subEntities, IBRepEntityCR in);

//! Query the set of vertices of the input body.
//! @param[out] subEntities An optional vector to hold the sub-entities of type SubEntityType::Vertex, pass NULL if just interested in count.
//! @param[in] in The entity to query.
//! @return A count of the number of vertices.
DGNPLATFORM_EXPORT static size_t GetBodyVertices(bvector<ISubEntityPtr>* subEntities, IBRepEntityCR in);

//! Query the set of edges for the input face sub-entity.
//! @param[out] subEntities A vector to hold the sub-entities of type SubEntityType::Edge.
//! @param[in] subEntity The face sub-entity to query.
//! @return SUCCESS if input entity was the correct type and output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetFaceEdges(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity);

//! Query the set of vertices for the input face sub-entity.
//! @param[out] subEntities A vector to hold the sub-entities of type SubEntityType::Vertex.
//! @param[in] subEntity The face sub-entity to query.
//! @return SUCCESS if input entity was the correct type and output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetFaceVertices(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity);

//! Query the set of faces for the input edge sub-entity.
//! @param[out] subEntities A vector to hold the sub-entities of type SubEntityType::Face.
//! @param[in] subEntity The edge sub-entity to query.
//! @return SUCCESS if input entity was the correct type and output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetEdgeFaces(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity);

//! Query the set of vertices for the input edge sub-entity.
//! @param[out] subEntities A vector to hold the sub-entities of type SubEntityType::Vertex.
//! @param[in] subEntity The edge sub-entity to query.
//! @return SUCCESS if input entity was the correct type and output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetEdgeVertices(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity);

//! Query the set of faces for the input vertex sub-entity.
//! @param[out] subEntities A vector to hold the sub-entities of type SubEntityType::Face.
//! @param[in] subEntity The vertex sub-entity to query.
//! @return SUCCESS if input entity was the correct type and output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetVertexFaces(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity);

//! Query the set of edges for the input vertex sub-entity.
//! @param[out] subEntities A vector to hold the sub-entities of type SubEntityType::Edge.
//! @param[in] subEntity The vertex sub-entity to query.
//! @return SUCCESS if input entity was the correct type and output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetVertexEdges(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity);

//! Query the set of edges that are connected and tangent to the given edge sub-entity.
//! @param[out] smoothEdges A vector to hold the sub-entities of type SubEntityType::Edge.
//! @param[in] edge The edge sub-entity to query smoothly connected edges for.
//! @return SUCCESS if the output vector was populated.
//! @note These are the edges that would be included by the propagate option of SolidUtil::Modify::BlendEdges and SolidUtil::Modify::ChamferEdges.
DGNPLATFORM_EXPORT static BentleyStatus GetTangentBlendEdges(bvector<ISubEntityPtr>& smoothEdges, ISubEntityCR edge);

//! Query the set of edges that comprise a single face loop containing the given edge sub-entity.
//! @param[out] loopEdges A vector to hold the sub-entities of type SubEntityType::Edge.
//! @param[in] edge The edge sub-entity that is part of the loop.
//! @param[in] face The face sub-entity that has the loop as part of it's bounds.
//! @return SUCCESS if the output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetLoopEdgesFromEdge(bvector<ISubEntityPtr>& loopEdges, ISubEntityCR edge, ISubEntityCR face);

//! Query the set of faces that are adjacent to the given face sub-entity.
//! @param[out] subEntities A vector to hold the sub-entities of type SubEntityType::Face.
//! @param[in] subEntity The face sub-entity to query adjacent faces for.
//! @param[in] includeVertex Whether to include vertex connected adjacent faces or just edge connected faces.
//! @param[in] includeRedundant Whether to include an adjacent face that has identical surface geometry as the given face.
//! @param[in] includeSmoothOnly Whether to include an adjacent face that is not smoothly connected to the given face.
//! @param[in] oneLevel When only returning smoothly connected adjacent faces, whether to return all smoothly connected faces, or just those immediately adjacent.
//! @return SUCCESS if the output vector was populated.
DGNPLATFORM_EXPORT static BentleyStatus GetAdjacentFaces(bvector<ISubEntityPtr>& subEntities, ISubEntityCR subEntity, bool includeVertex = true, bool includeRedundant = true, bool includeSmoothOnly = false, bool oneLevel = true);

//! Return a vector of unique vertices from a set of sub-entities that can include faces, edges, and vertices.
//! @note The input subEntities are all expected/required to be from the same body.
DGNPLATFORM_EXPORT static void GetSubEntityVertices(bvector<ISubEntityPtr>& vertices, bvector<ISubEntityPtr> const& subEntities);

//! Return a vector of unique edges from a set of sub-entities that can include faces, edges. Vertex sub-entities are ignored.
//! @note The input subEntities are all expected/required to be from the same body.
DGNPLATFORM_EXPORT static void GetSubEntityEdges(bvector<ISubEntityPtr>& edges, bvector<ISubEntityPtr> const& subEntities);

//! Get uv face parameter range for the given face sub-entity.
//! @param[in] subEntity The face sub-entity to query.
//! @param[out] uRange The u parameter range of the face.
//! @param[out] vRange The v parameter range of the face.
//! @return SUCCESS if face parameter range was computedx.
DGNPLATFORM_EXPORT static BentleyStatus GetFaceParameterRange(ISubEntityCR subEntity, DRange1dR uRange, DRange1dR vRange);

//! Get u edge parameter range for the given edge sub-entity.
//! @param[in] subEntity The edge sub-entity to query.
//! @param[out] uRange The u parameter range of the edge.
//! @return SUCCESS if edge parameter range was computed.
DGNPLATFORM_EXPORT static BentleyStatus GetEdgeParameterRange(ISubEntityCR subEntity, DRange1dR uRange);

//! Evaluate point, normal, and derivatives at a uv parameter on the surface of the given face sub-entity.
//! @param[in] subEntity The face sub-entity to query.
//! @param[out] point The coordinates of the point on the surface at the uv parameter.
//! @param[out] normal The normalized surface normal at the uv parameter.
//! @param[out] uDir The first derivative with respect to u at the uv parameter.
//! @param[out] vDir The first derivative with respect to v at the uv parameter.
//! @param[in] uvParam The uv parameter pair to evaluate.
//! @return SUCCESS if the parameter could be evaluated.
DGNPLATFORM_EXPORT static BentleyStatus EvaluateFace(ISubEntityCR subEntity, DPoint3dR point, DVec3dR normal, DVec3dR uDir, DVec3dR vDir, DPoint2dCR uvParam);

//! Evaluate point and tangent at a u parameter on the curve of the given edge sub-entity.
//! @param[in] subEntity The edge sub-entity to query.
//! @param[out] point The coordinates of the point on the curve at the u parameter.
//! @param[out] uDir The normalized curve tangent at the u parameter.
//! @param[in] uParam The u parameter to evaluate.
//! @return SUCCESS if the parameter could be evaluated.
DGNPLATFORM_EXPORT static BentleyStatus EvaluateEdge(ISubEntityCR subEntity, DPoint3dR point, DVec3dR uDir, double uParam);

//! Evaluate point of the given vertex sub-entity.
//! @param[in] subEntity The vertex sub-entity to query.
//! @param[out] point The coordinates of the point at the given vertex.
//! @return SUCCESS if vertex point exists.
DGNPLATFORM_EXPORT static BentleyStatus EvaluateVertex(ISubEntityCR subEntity, DPoint3dR point);

//! Return whether the supplied face has a planar surface.
DGNPLATFORM_EXPORT static bool IsPlanarFace(ISubEntityCR);

//! Return whether the angle between the normals of the supplied edge's faces never exceeds the internal smooth angle tolerance along the length of the edge.
DGNPLATFORM_EXPORT static bool IsSmoothEdge(ISubEntityCR);

//! Return whether the supplied entity is a laminar edge of a sheet body, i.e. boundary of a single face.
DGNPLATFORM_EXPORT static bool IsLaminarEdge(ISubEntityCR);

//! Return whether the supplied entity is a linear edge.
DGNPLATFORM_EXPORT static bool IsLinearEdge(ISubEntityCR);

//! Return whether the supplied entity is a disjoint body.
DGNPLATFORM_EXPORT static bool IsDisjointBody(IBRepEntityCR);

//! Return whether the supplied entity is a sheet body with a single planar face.
DGNPLATFORM_EXPORT static bool IsSingleFacePlanarSheetBody(IBRepEntityCR, bool& hasHoles);

//! Return whether the supplied sheet or solid entity has all planar faces.
DGNPLATFORM_EXPORT static bool HasOnlyPlanarFaces(IBRepEntityCR);

//! Return whether the supplied entity has any edge that is non-linear or any face that is non-planar.
DGNPLATFORM_EXPORT static bool HasCurvedFaceOrEdge(IBRepEntityCR);

//! Pick face, edge, and vertex sub-entities of a body by their proximity to a ray.
//! @param[in] entity The entity to pick sub-entities for.
//! @param[in] boresite The ray origin and direction.
//! @param[out] intersectEntities The selected sub-entities.
//! @param[in] maxFace The maximum number of face hits to return. Pass 0 to not pick faces.
//! @param[in] maxEdge The maximum number of edge hits to return. Pass 0 to not pick edges.
//! @param[in] maxVertex The maximum number of vertex hits to return. Pass 0 to not pick vertices.
//! @param[in] maxEdgeDistance An edge will be picked if it is within this distance from the ray.
//! @param[in] maxVertexDistance A vertex will be picked if it is within this distance from the ray.
//! @note The returned entities are ordered by increasing distance from ray origin to hit point on entity.
//! @return true if ray intersected a requested entity type.
//! @see ISubEntity::GetFaceLocation ISubEntity::GetEdgeLocation
DGNPLATFORM_EXPORT static bool Locate(IBRepEntityCR entity, DRay3dCR boresite, bvector<ISubEntityPtr>& intersectEntities, size_t maxFace, size_t maxEdge, size_t maxVertex, double maxEdgeDistance, double maxVertexDistance);

//! Find the closest sub-entity on body to a given point.
//! @param[in] entity The entity to find the closest sub-entity for.
//! @param[in] testPt The space point.
//! @return the face, edge, or vertex sub-entity that contains the closest point.
//! @see ISubEntity::GetFaceLocation ISubEntity::GetEdgeLocation
DGNPLATFORM_EXPORT static ISubEntityPtr ClosestSubEntity(IBRepEntityCR entity, DPoint3dCR testPt);

//! Find the closest face on body to a given point.
//! @param[in] entity The entity to find the closest sub-entity for.
//! @param[in] testPt The space point.
//! @param[in] preferredDir Optional direction for choosing the "best" face when closest entity is an edge or vertex.
//! @return the face that contains the closest point.
//! @see ISubEntity::GetFaceLocation
DGNPLATFORM_EXPORT static ISubEntityPtr ClosestFace(IBRepEntityCR entity, DPoint3dCR testPt, DVec3dCP preferredDir = nullptr);

//! Test if a point is inside or on the boundary of the given body.
//! @param[in] entity The entity to test.
//! @param[in] testPt The space point.
//! @return true if point is not outside the body.
DGNPLATFORM_EXPORT static bool IsPointInsideBody(IBRepEntityCR entity, DPoint3dCR testPt);

//! Get the ray intersection with a face.
//! @param[in] subEntity The face to intersect.
//! @param[in] boresite The ray origin and direction.
//! @param[out] intersectPts The hit points on the face.
//! @param[out] intersectParams The uv parameters on the face.
//! @return true if ray intersects face.
DGNPLATFORM_EXPORT static bool LocateFace(ISubEntityCR subEntity, DRay3dCR boresite, bvector<DPoint3d>& intersectPts, bvector<DPoint2d>& intersectParams);

//! Get the closest point on a face to a given point.
//! @param[in] subEntity The face to test.
//! @param[in] testPt The space point.
//! @param[out] point The closest point on the face.
//! @param[out] param The uv parameter at the closest point.
//! @return true if closest point was found.
DGNPLATFORM_EXPORT static bool ClosestPointToFace(ISubEntityCR subEntity, DPoint3dCR testPt, DPoint3dR point, DPoint2dR param);

//! Get the closest point on an edge to a given point.
//! @param[in] subEntity The edge to test.
//! @param[in] testPt The space point.
//! @param[out] point The closest point on the edge.
//! @param[out] param The u parameter at the closest point.
//! @return true if closest point was found.
DGNPLATFORM_EXPORT static bool ClosestPointToEdge(ISubEntityCR subEntity, DPoint3dCR testPt, DPoint3dR point, double& param);

//! Return a PolyfaceHeader created by facetting the supplied sheet or solid body using the specified facet options.
DGNPLATFORM_EXPORT static PolyfaceHeaderPtr FacetEntity(IBRepEntityCR, IFacetOptionsR);

//! Return a PolyfaceHeader and FaceAttachment for each unique symbology by facetting the supplied sheet or solid body using the specified facet options.
DGNPLATFORM_EXPORT static bool FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, IFacetOptionsR facetOptions);

//! Perform 3d clip of the supplied curve vector.
DGNPLATFORM_EXPORT static BentleyStatus ClipCurveVector(bvector<CurveVectorPtr>& output, CurveVectorCR input, ClipVectorCR clipVector, TransformCP transform);

//! Perform 3d clip of the supplied sheet or solid entity.
DGNPLATFORM_EXPORT static BentleyStatus ClipBody(bvector<IBRepEntityPtr>& output, bool& clipped, IBRepEntityCR input, ClipVectorCR clipVector);

//! Evaluate mass properties of the supplied entity. Used internally by MeasureGeomCollector which is the preferred api to use for all geometric primitive types.
//! The returned values are interpreted according to entity type, i.e. amount is length for a wire body, area for a sheet, and volume for a solid.
//! @see MeasureGeomCollector.
DGNPLATFORM_EXPORT static BentleyStatus MassProperties(IBRepEntityCR, double* amount, double* periphery, DPoint3dP centroid, double inertia[3][3], double tolerance);

//! Modify the target solid or sheet body by attaching/removing per-face color/material overrides.
//! @param[in,out] target The target body to update face material attachments for.
//! @param[in] faces The array of faces to attach/remove the supplied faceParams to.
//! @param[in] baseParams GeometryParams to initialize IFaceMaterialAttachmentsP if none currently, clears all attachments if nullptr.
//! @param[in] faceParams GeometryParams to attach to the supplied faces, clears face attachments if nullptr.
//! @return SUCCESS is face material attachments are updated.
DGNPLATFORM_EXPORT static BentleyStatus UpdateFaceMaterialAttachments(IBRepEntityR target, bvector<ISubEntityPtr>& faces, Render::GeometryParamsCP baseParams = nullptr, Render::GeometryParamsCP faceParams = nullptr);

//! Support for the creation of new bodies from other types of geometry.
struct Create
    {
    //! Create a new wire or planar sheet body from a CurveVector that represents an open path, closed path, region with holes, or union region.
    //! @param[out] out The new body.
    //! @param[in] curve The curve vector to create a body from.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @note The CurvePrimitives that define an open path or closed loop are expected to be connected head-to-tail and may not intersect except at a vertex. A vertex can be shared by at most 2 edges.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus BodyFromCurveVector(IBRepEntityPtr& out, CurveVectorCR curve, uint32_t nodeId = 0L);
    
    //! Represent a wire body or a single face planar sheet body as a CurveVector.
    //! @param[in] entity The wire or sheet body to try to convert.
    //! @return nullptr if supplied entity was not a wire or single face planar sheet body that could be represented as a CurveVector. 
    DGNPLATFORM_EXPORT static CurveVectorPtr BodyToCurveVector(IBRepEntityCR entity);

    //! Represent a planar face as a CurveVector.
    //! @param[in] face The planar face to try to convert.
    //! @return nullptr if supplied sub-entity was not a planar face that could be represented as a CurveVector. 
    DGNPLATFORM_EXPORT static CurveVectorPtr PlanarFaceToCurveVector(ISubEntityCR face);

    //! Represent edges with the given offset distance on the supplied planar face as a CurveVector.
    //! @param[in] face The target face sub-entity to offset the edges onto.
    //! @param[in] edges The array of edges to offset with the first edge used as the reference edge for the offset distance. Edges that don't surround the target face are ignored.
    //! @param[in] distance The offset distance.
    //! @return nullptr if edge offset could not be created.
    DGNPLATFORM_EXPORT static CurveVectorPtr OffsetEdgesOnPlanarFaceToCurveVector(ISubEntityCR face, bvector<ISubEntityPtr>& edges, double distance);

    //! Create a sheet body suitable for the BooleanCut operation from an open profile.
    //! @param[out] out The new sheet body.
    //! @param[in] curve The curve vector to create a body from. Closed regions are also supported and will just be handled by BodyFromVector.
    //! @param[in] targetRange The range of the target entity the returned sheet body will be used to modify.
    //! @param[in] defaultNormal The normal to use for a curve without a well defined normal, ex. a single line segment.
    //! @param[in] reverseClosure Whether to reverse the "natural" closure direction.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @note The CurvePrimitives that define an open path or closed loop are expected to be connected head-to-tail and may not intersect except at a vertex. A vertex can be shared by at most 2 edges.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus CutProfileBodyFromOpenCurveVector(IBRepEntityPtr& out, CurveVectorCR curve, DRange3dCR targetRange, DVec3dCP defaultNormal = nullptr, bool reverseClosure = false, uint32_t nodeId = 0L);

    //! Create a sheet body suitable for the BooleanSubtract operation by extending/sweeping an open profile.
    //! @param[out] out The new sheet body.
    //! @param[in] curve The curve vector to create a body from.
    //! @param[in] targetRange The range of the target entity the returned sheet body will be used to modify.
    //! @param[in] defaultNormal The normal to use for a curve without a well defined normal, ex. a single line segment.
    //! @param[in] extend Whether to extend the ends of an open profile past the target range.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @note The CurvePrimitives that define an open path are expected to be connected head-to-tail and may not intersect except at a vertex. A vertex can be shared by at most 2 edges.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus SweptBodyFromOpenCurveVector(IBRepEntityPtr& out, CurveVectorCR curve, DRange3dCR targetRange, DVec3dCP defaultNormal = nullptr, bool extend = true, uint32_t nodeId = 0L);
    
    //! Create a new sheet or solid body from an ISolidPrimitive.
    //! @param[out] out The new body.
    //! @param[in] primitive The surface or solid to create a body from.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus BodyFromSolidPrimitive(IBRepEntityPtr& out, ISolidPrimitiveCR primitive, uint32_t nodeId = 0L);
    
    //! Create a new sheet body from a MSBsplineSurface.
    //! @param[out] out The new body.
    //! @param[in] surface The surface to create a body from.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus BodyFromBSurface(IBRepEntityPtr& out, MSBsplineSurfaceCR surface, uint32_t nodeId = 0L);

    //! Create a new sheet or solid body from a Polyface.
    //! @param[out] out The new body.
    //! @param[in] meshData The surface or solid to create a body from.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus BodyFromPolyface(IBRepEntityPtr& out, PolyfaceQueryCR meshData, uint32_t nodeId = 0L);

    //! @param[out] out The new body.
    //! @param[in] profiles The cross sections profiles.
    //! @param[in] guides An optional set of guide curves for controlling the loft.
    //! @param[in] periodic When true start profile is also used as end profile to create a periodic result in loft direction.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @note Requires a minimum input of 2 profiles, or 1 profile and 1 guide to produce a valid loft.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus BodyFromLoft(IBRepEntityPtr& out, bvector<CurveVectorPtr>& profiles, bvector<CurveVectorPtr>* guides, bool periodic, uint32_t nodeId = 0L);

    //! Create a new sheet or solid body by sweeping a cross section profile along a path.
    //! @param[out] out The new body.
    //! @param[in] profile The cross section profile. (open, closed, or region with holes)
    //! @param[in] path The path to sweep along.
    //! @param[in] alignParallel true to keep profile at a fixed angle to global axis instead of path tangent (and lock direction).
    //! @param[in] selfRepair true to attempt repair of self-intersections.
    //! @param[in] createSheet true to force a sheet body to be created from a closed profile which would normally produce a solid body. (Similiar behavior to ISolidPrimitive::GetCapped)
    //! @param[in] lockDirection Optionally keep profile at a fixed angle relative to the path tangent projected into a plane perpendicular to the lock direction. Only valid when alignParallel is false.
    //! @param[in] twistAngle Optionally spin profile as it moves along the path.
    //! @param[in] scale Optionally scale profile as it moves along the path.
    //! @param[in] scalePoint The profile point to scale about, required when applying scale.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus BodyFromSweep(IBRepEntityPtr& out, CurveVectorCR profile, CurveVectorCR path, bool alignParallel, bool selfRepair, bool createSheet, BentleyApi::DVec3dCP lockDirection = NULL, double const* twistAngle = NULL, double const* scale = NULL, BentleyApi::DPoint3dCP scalePoint = NULL, uint32_t nodeId = 0L);

    //! Create a new body by extruding a planar sheet body up to another body.
    //! @param[out] out The new body.
    //! @param[in] extrudeTo The body to trim the extruded body to.
    //! @param[in] profile The planar sheet body to extrude.
    //! @param[in] reverseDirection To specify if extrusion is in the same direction or opposite direction to the surface normal of the profile sheet body.
    //! @param[in] nodeId Assign topology ids to the faces of the body being created when nodeId is non-zero.
    //! @return SUCCESS if body was created.
    DGNPLATFORM_EXPORT static BentleyStatus BodyFromExtrusionToBody(IBRepEntityPtr& out, IBRepEntityCR extrudeTo, IBRepEntityCR profile, bool reverseDirection, uint32_t nodeId = 0L);
    };

//! Support for modification of bodies.
struct Modify
    {
    enum class BooleanMode
        {                       
        Unite       = 0, //!< Unite target with one or more tool entities.
        Subtract    = 1, //!< Subtract one or more tool entities from target entity.
        Intersect   = 2, //!< Intersect target with one or more tool entities.
        };

    enum class StepFacesOption
        {
        AddNone           = 0, //!< Don't create any step faces.
        AddSmooth         = 1, //!< Create step faces at smooth boundary edges.
        AddNonCoincident  = 2, //!< Create step faces at edges where step would not be coincident with the adjacent face.
        AddAll            = 3, //!< Create step faces at all boundary edges.
        };
    
    enum class CutDirectionMode
        {                       
        Forward                 = 0, //!< Remove material in direction of surface normal.
        Backward                = 1, //!< Remove material in opposite direction of surface normal.
        Both                    = 2, //!< Remove material in both directions.
        };

    enum class CutDepthMode
        {
        All                     = 0, //!< Cut extends through entire solid.
        Blind                   = 1, //!< Cut extends to a specified depth.
        };

    enum class ChamferMode
        {                       
        Ranges                  = 0, //!< Chamfer ranges.
        Length                  = 1, //!< Chamfer length. Specify lengths using values1, values2 is unused and can be NULL.
        Distances               = 2, //!< Right/Left distances. Can pass NULL for values2 for equal distance.
        DistanceAngle           = 3, //!< Right distance and angle (radians).
        AngleDistance           = 4, //!< Angle (radians) and left distance.
        };

    //! Create a rollback mark that represents the current modeller state and that will return the modeller to this state when the mark is destroyed.
    DGNPLATFORM_EXPORT static RefCountedPtr<IRefCounted> CreateRollbackMark();

    //! Perform the specified boolean operation between the target body and tool body.
    //! @param[in,out] target The target body to modify (may be consumed by subtract operation).
    //! @param[in,out] tool The tool body (consumed in boolean).
    //! @param[in] mode The boolean operation to perform.
    //! @return SUCCESS if boolean operation was completed.
    //! @note: A successful boolean subtract can produce no geometry, check target.IsValid().
    DGNPLATFORM_EXPORT static BentleyStatus BooleanOperation(IBRepEntityPtr& target, IBRepEntityPtr& tool, BooleanMode mode);

    //! Perform the specified boolean operation between the target body and one or more tool bodies.
    //! @param[in,out] target The target body to modify (may be consumed by subtract operation).
    //! @param[in,out] tools A vector of one or more tool bodies (consumed in boolean).
    //! @param[in] mode The boolean operation to perform.
    //! @return SUCCESS if boolean operation was completed.
    //! @note: A successful boolean subtract can produce no geometry, check target.IsValid().
    DGNPLATFORM_EXPORT static BentleyStatus BooleanOperation(IBRepEntityPtr& target, bvector<IBRepEntityPtr>& tools, BooleanMode mode);

    //! Modify the target body by subtracting a cut body produced from sweeping the sheet tool body according to the specified cut direction and depth.
    //! @param[in,out] target The target body to modify (may be consumed by cut operation).
    //! @param[in] planarTool The planar sheet body for the cut profile.
    //! @param[in] directionMode The sweep direction relative to the sheet body normal of the cut profile.
    //! @param[in] depthMode To specify if the cut should extended through the entire body or only create a pocket of fixed depth.
    //! @param[in] distance To specify the cut depth for CutDepthMode::Blind.
    //! @param[in] inside Whether to remove material inside profile or outside profile.
    //! @return SUCCESS if cut operation was completed.
    //! @note: A successful boolean cut can produce no geometry, check target.IsValid().
    DGNPLATFORM_EXPORT static BentleyStatus BooleanCut(IBRepEntityPtr& target, IBRepEntityCR planarTool, CutDirectionMode directionMode, CutDepthMode depthMode, double distance, bool inside);

    //! Sew the given set of sheet bodies together by joining those that share edges in common.
    //! @param[out] sewn The new bodies produced by sewing.
    //! @param[out] unsewn The bodies that were not able to be sewn.
    //! @param[in,out] tools The array of sheet bodies. (invalidated after sew).
    //! @param[in] gapWidthBound Defines a limit on the width of the gap between sheet body edges that will be allowed to remain.
    //! @param[in] nIterations To request repeated sew attempts that automatically increase gap up to limit set by gapWidthBound.
    //! @return SUCCESS if some bodies were able to be sewn together.
    DGNPLATFORM_EXPORT static BentleyStatus SewBodies(bvector<IBRepEntityPtr>& sewn, bvector<IBRepEntityPtr>& unsewn, bvector<IBRepEntityPtr>& tools, double gapWidthBound, size_t nIterations = 1);

    //! Separate a disjoint body into multiple bodies. If the input body does not have disjoint regions, it will be unchanged and the output vector will be empty.
    //! In the case of a disjoint body, the original entity will reference the first solid region, and any additional regions will be returned in the output vector.
    //! @return SUCCESS if operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus DisjoinBody(bvector<IBRepEntityPtr>& additionalEntities, IBRepEntityR entity);

    //! Modify the target body by removing redundant topology. An example of redundant topology would be an edge where the faces on either side have identical surface geometry.
    //! @param[in,out] target The target body to modify.
    //! @return SUCCESS if operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus DeleteRedundantTopology(IBRepEntityR target);

    //! Reverse the surface normals of the target sheet body.
    //! @param[in,out] target The target sheet body to reverse the orientation of.
    //! @return SUCCESS if surface normals could be negated.
    DGNPLATFORM_EXPORT static BentleyStatus ReverseOrientation(IBRepEntityR target);

    //! Modify the target body by sweeping along a path vector.
    //! @param[in,out] target The target body to sweep. A wire body becomes a sheet, and a sheet body becomes a solid.
    //! @param[in] path A scaled vector to define the sweep direction and distance.
    //! @return SUCCESS if sweep could be completed.
    DGNPLATFORM_EXPORT static BentleyStatus SweepBody(IBRepEntityR target, DVec3dCR path);

    //! Modify the target body by spinning along an arc specified by a revolve axis and sweep angle.
    //! @param[in,out] target The target body to spin. A wire body becomes a sheet, and a sheet body becomes a solid.
    //! @param[in] axis The revolve axis.
    //! @param[in] angle The sweep angle. (value in range of -2pi to 2pi)
    //! @return SUCCESS if spin could be completed.
    DGNPLATFORM_EXPORT static BentleyStatus SpinBody(IBRepEntityR target, DRay3dCR axis, double angle);

    //! Modify the target body by adding a pad or pocket constructed from the sheet tool body and its swept imprint on the target body.
    //! @param[in,out] target The target body to modify, can be a sheet or solid.
    //! @param[in] tool The sheet body for the emboss end cap.
    //! @param[in] reverseDirection true to reverse tool surface normal. Material is added in the opposite direction as the surface normal when creating a pad (points outwards from solid).
    //! @param[in] direction Optional direction for swept sidewall (aligned with outward normal of end cap). If not specified tool surface normal at center of uv range is used.
    //! @return SUCCESS if emboss operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus Emboss(IBRepEntityR target, IBRepEntityCR tool, bool reverseDirection, DVec3dCP direction = nullptr);

    //! Modify the target sheet body by thickening to create a solid body.
    //! @param[in,out] target The target sheet body to thicken.
    //! @param[in] frontDistance The offset distance in the direction of the sheet body face normal.
    //! @param[in] backDistance The offset distance in the opposite direction of the sheet body face normal.
    //! @return SUCCESS if thicken could be completed.
    DGNPLATFORM_EXPORT static BentleyStatus ThickenSheet(IBRepEntityR target, double frontDistance, double backDistance);

    //! Perform the intersection operation on 2 sheet bodies
    //! @param[out] vectorOut the result of the operation
    //! @param[in] sheet1 the 1st sheet body to intersect
    //! @param[in] sheet2 the 2nd sheet body to intersect
    //! @return SUCCESS if intersection operation was completed.
    //! @note: A successful boolean subtract can produce no geometry, check target.IsValid().
    DGNPLATFORM_EXPORT static BentleyStatus IntersectSheetFaces(CurveVectorPtr& vectorOut, IBRepEntityCR sheet1, IBRepEntityCR sheet2);

    //! Modify the specified edges of the given body by changing them into faces having the requested blending surface geometry.
    //! @param[in,out] target The target body to blend.
    //! @param[in] edges The vector of edge sub-entities to attach blends to.
    //! @param[in] radii The vector of blend radius values for each edge.
    //! @param[in] propagateSmooth Whether to automatically continue blend along connected and tangent edges that aren't explicitly specified in edges array.
    //! @return SUCCESS if blends could be created.
    DGNPLATFORM_EXPORT static BentleyStatus BlendEdges(IBRepEntityR target, bvector<ISubEntityPtr>& edges, bvector<double> const& radii, bool propagateSmooth = true);

    //! Modify the specified edges of the given body by changing them into faces having the requested chamfer surface geometry.
    //! @param[in,out] target The target body to chamfer.
    //! @param[in] edges The vector of edge sub-entities to attach chamfers to.
    //! @param[in] values1 The vector of chamfer values for each edge, value meaning varies by ChamferMode.
    //! @param[in] values2 The vector of chamfer values for each edge, value meaning varies by ChamferMode. (Unused for ChamferMode::Length, required for ChamferMode::AngleDistance)
    //! @param[in] mode Specifies chamfer type and determines how values1 and values2 are interpreted and used.
    //! @param[in] propagateSmooth Whether to automatically continue chamfer along connected and tangent edges that aren't explicitly specified in edges array.
    //! @return SUCCESS if chamfers could be created.
    DGNPLATFORM_EXPORT static BentleyStatus ChamferEdges(IBRepEntityR target, bvector<ISubEntityPtr>& edges, bvector<double> const& values1, bvector<double> const* values2, ChamferMode mode, bool propagateSmooth = true);

    //! Modify the target solid body by hollowing using specified face offsets.
    //! @param[in,out] target The target body to hollow.
    //! @param[in] faces The array of faces to be offset by other than the default offset distance.
    //! @param[in] defaultDistance The offset distance to apply to any face not specifically included in the faces array.
    //! @param[in] distances The array of offsets for each face.
    //! @param[in] addStep The option for how to handle the creation of step faces.
    //! @note A positive offset goes outwards (in the direction of the surface normal), a negative offset is inwards, and a face with zero offset will be pierced/removed.
    //! @return SUCCESS if hollow could be created.
    DGNPLATFORM_EXPORT static BentleyStatus HollowFaces(IBRepEntityR target, bvector<ISubEntityPtr>& faces, double defaultDistance, bvector<double> const& distances, StepFacesOption addStep = StepFacesOption::AddNonCoincident);

    //! Modify the target solid or sheet body by offsetting selected faces.
    //! @param[in,out] target The target body to modify.
    //! @param[in] faces The array of faces to be offset.
    //! @param[in] distances The array of offsets for each face.
    //! @param[in] addStep The option for how to handle the creation of step faces.
    //! @return SUCCESS if faces could be offset.
    DGNPLATFORM_EXPORT static BentleyStatus OffsetFaces(IBRepEntityR target, bvector<ISubEntityPtr>& faces, bvector<double> const& distances, StepFacesOption addStep = StepFacesOption::AddNonCoincident);

    //! Modify the target solid or sheet body by offsetting selected edges.
    //! @param[in,out] target The target body to modify.
    //! @param[in] edges The array of edges to offset with the first edge used as the reference edge for the offset distance. Edges that don't share a face with the reference edge are ignored.
    //! @param[in] offsetDir The offset direction relative to the reference edge.
    //! @param[in] offset The offset distance for each edge.
    //! @param[in] propagateSmooth Whether to automatically continue offset along connected and tangent edges that aren't explicitly specified in edges array.
    //! @param[in] addStep The option for how to handle the creation of step faces.
    //! @return SUCCESS if edges could be offset.
    DGNPLATFORM_EXPORT static BentleyStatus OffsetEdges(IBRepEntityR target, bvector<ISubEntityPtr>& edges, DVec3dCR offsetDir, double offset, bool propagateSmooth = true, StepFacesOption addStep = StepFacesOption::AddNonCoincident);

    //! Modify the target solid or sheet body by transforming selected faces.
    //! @param[in,out] target The target body to modify.
    //! @param[in] faces The array of faces to be transformed.
    //! @param[in] transforms The array of transforms for each face.
    //! @param[in] addStep The option for how to handle the creation of step faces.
    //! @return SUCCESS if faces could be transformed.
    DGNPLATFORM_EXPORT static BentleyStatus TransformFaces(IBRepEntityR target, bvector<ISubEntityPtr>& faces, bvector<Transform> const& transforms, StepFacesOption addStep = StepFacesOption::AddNonCoincident);

    //! Modify the target solid or sheet body by transforming selected edges.
    //! @param[in,out] target The target body to modify.
    //! @param[in] edges The array of edges to be transformed.
    //! @param[in] transforms The array of transforms for each edge.
    //! @param[in] addStep The option for how to handle the creation of step faces. NOTE: AddNonCoincident is only supported for pure translation/rotation...
    //! @return SUCCESS if edges could be transformed.
    DGNPLATFORM_EXPORT static BentleyStatus TransformEdges(IBRepEntityR target, bvector<ISubEntityPtr>& edges, bvector<Transform> const& transforms, StepFacesOption addStep = StepFacesOption::AddNone);

    //! Modify the target solid or sheet body by transforming selected vertices.
    //! @param[in,out] target The target body to modify.
    //! @param[in] vertices The array of vertices to be transformed.
    //! @param[in] transforms The array of transforms for each vertex.
    //! @param[in] addStep The option for how to handle the creation of step faces. NOTE: AddNonCoincident is only supported for pure translation/rotation...
    //! @return SUCCESS if vertices could be transformed.
    DGNPLATFORM_EXPORT static BentleyStatus TransformVertices(IBRepEntityR target, bvector<ISubEntityPtr>& vertices, bvector<Transform> const& transforms, StepFacesOption addStep = StepFacesOption::AddNone);

    //! Modify the target solid or sheet body by sweeping selected faces along a path vector.
    //! @param[in,out] target The target body to modify.
    //! @param[in] faces The array of faces to be swept.
    //! @param[in] path A scaled vector to define the sweep direction and distance.
    //! @return SUCCESS if faces could be swept.
    DGNPLATFORM_EXPORT static BentleyStatus SweepFaces(IBRepEntityR target, bvector<ISubEntityPtr>& faces, DVec3dCR path);

    //! Modify the target solid or sheet body by spinning selected faces along an arc specified by a revolve axis and sweep angle.
    //! @param[in,out] target The target body to modify.
    //! @param[in] faces The array of faces to be spun.
    //! @param[in] axis The revolve axis.
    //! @param[in] angle The sweep angle. (value in range of -2pi to 2pi)
    //! @return SUCCESS if faces could be spun.
    DGNPLATFORM_EXPORT static BentleyStatus SpinFaces(IBRepEntityR target, bvector<ISubEntityPtr>& faces, DRay3dCR axis, double angle);

    //! Modify the target solid or sheet body by tapering selected faces.
    //! @param[in,out] target The target body to modify.
    //! @param[in] faces The array of faces to taper.
    //! @param[in] refEntities The array of references entities (one for each face entry) that should retain their geometry after the taper has been applied. Can be edges or faces.
    //! @param[in] direction The taper direction.
    //! @param[in] angles The taper angle(s). Either a single taper angle or a taper angle for each face entry. (value in range of -2pi to 2pi)
    //! @param[in] addStep The option for how to handle the creation of step faces.
    //! @return SUCCESS if faces could be tapered.
    DGNPLATFORM_EXPORT static BentleyStatus TaperFaces(IBRepEntityR target, bvector<ISubEntityPtr>& faces, bvector<ISubEntityPtr>& refEntities, DVec3dCR direction, bvector<double>& angles, StepFacesOption addStep = StepFacesOption::AddNonCoincident);

    //! Modify the target solid or sheet body by removing selected faces and healing.
    //! @param[in,out] target The target body to modify.
    //! @param[in] faces The array of faces to be deleted.
    //! @param[in] createCap Try to heal by finding a surface to fit the hole resulting from the removal of the supplied faces.
    //! @return SUCCESS if faces could be deleted.
    DGNPLATFORM_EXPORT static BentleyStatus DeleteFaces(IBRepEntityR target, bvector<ISubEntityPtr>& faces, bool createCap = false);

    //! Modify the target solid or sheet body by removing selected edges and healing.
    //! @param[in,out] target The target body to modify.
    //! @param[in] edges The array of edges to be deleted.
    //! @return SUCCESS if edges could be deleted.
    DGNPLATFORM_EXPORT static BentleyStatus DeleteEdges(IBRepEntityR target, bvector<ISubEntityPtr>& edges);

    //! Modify a face of a body by imprinting new edges from the specified curve vector.
    //! @param[in,out] face The target face sub-entity to imprint.
    //! @param[in] curveVector The curve geometry to imprint.
    //! @param[in] direction The project direction (optional, uses curvature if nullptr).
    //! @param[in] extend Whether to extend an open wire body to ensure that it splits the face.
    //! @return SUCCESS if face imprint created.
    DGNPLATFORM_EXPORT static BentleyStatus ImprintCurveVectorOnFace(ISubEntityPtr& face, CurveVectorCR curveVector, DVec3dCP direction = nullptr, bool extend = true);

    //! Modify the target body by imprinting new edges from the specified curve vector.
    //! @param[in,out] target The target body to imprint.
    //! @param[in] curveVector The curve geometry to imprint.
    //! @param[in] direction The project direction (optional, uses curvature if nullptr).
    //! @param[in] extend Whether to extend an open curve to ensure that it splits the face.
    //! @return SUCCESS if imprint created.
    DGNPLATFORM_EXPORT static BentleyStatus ImprintCurveVectorOnBody(IBRepEntityR target, CurveVectorCR curveVector, DVec3dCP direction = nullptr, bool extend = true); 

    //! Modify the target body by imprinting edges where the faces from the supplied tool body intersect.
    //! @param[in,out] target The target body to imprint (must be solid or sheet)
    //! @param[in] tool The tool body to imprint (must be solid or sheet).
    //! @param[in] extend Whether to extend a tool surface to ensure that it splits the face on the target.
    //! @return SUCCESS if imprint created.
    DGNPLATFORM_EXPORT static BentleyStatus ImprintBodyOnBody(IBRepEntityR target, IBRepEntityCR tool, bool extend = true); 

    //! Modify a planar face of a body by imprinting edges with the supplied offset distance.
    //! @param[in,out] face The target face sub-entity to imprint.
    //! @param[in] edges The list of edges to imprint with the first edge used as the reference edge for the offset distance. Edges that don't surround the target face are ignored.
    //! @param[in] distance The offset distance.
    //! @param[in] extend Whether to extend edges that don't form a closed loop to ensure that they splits the face.
    //! @return SUCCESS if face imprint created.
    DGNPLATFORM_EXPORT static BentleyStatus ImprintOffsetEdgesOnPlanarFace(ISubEntityPtr& face, bvector<ISubEntityPtr>& edges, double distance, bool extend = true);
    };

//! Support for persistent topological ids on faces, edges, and vertices.
//!
//! A topology id provides a mechanism for identifying a ISubEntity that is independent of any IBRepEntity instance. 
//! Useful for identifying a ISubEntity across sessions or different instances of a IBRepEntity in the same session. 
//! In feature-based modeling topology ids allow the corresponding ISubEntity to be found again after re-evaluating the features.
//!
//! The foundation for topology identification is the FaceId. A FaceId is a (nodeId-entityId) pair that is assigned to each face 
//! of a sheet or solid body. The nodeId typically denotes the operation that produced the face and the entityId is used to 
//! differentiate between all new faces produced.
//! 
//! Consider a newly created cube. It has 6 faces which we could assign a nodeId of 1, each face would then be assigned its
//! own unique entityId from 1 to 6. Now consider cutting a circular slot that splits the top face of our cube (1-1), the single 
//! circular face of the slot could be assigned (2-1), i.e. operation 2 and face 1. After creating the slot, FaceId (1-1) now 
//! identifies 2 faces, one on either side of the split; additional ids will be added to resolve the duplicate, (1-7, 1-1) and (1-8, 1-1).
//! So FaceId (1-1) still exists but we now also have new "highest" FaceIds to uniquely identity the post-split geometry.
//!
//! An edge is identified by its face(s), therefore an EdgeId consists of 2 FaceId pairs. For the laminar edge of a sheet body, both FaceIds
//! will be the same. Similarly a vertex is identified by 3 FaceId pairs.
struct TopologyID
    {
    //! Assign new topology ids to faces of the given body. If the caller does not supply a non-zero node id the next highest available will be used.
    //! @param[in,out] entity The body to modify.
    //! @param[in] nodeId The non-zero topology node id to use in the new nodeId-entityId pairs or 0 to find and use the next available.
    //! @return The node id assigned to any new faces or edges.
    //! @see AddNodeIdAttributes FindNodeIdRange
    DGNPLATFORM_EXPORT static uint32_t AssignNewTopologyIds(IBRepEntityR entity, uint32_t nodeId = 0L);

    //! Assign new topology ids to faces of the given body. Resolves duplicate face ids such as from a face being split.
    //! @param[in,out] entity The body to modify.
    //! @param[in] nodeId The non-zero topology node id to use in the new nodeId-entityId pairs.
    //! @param[in] overrideExisting false to assign new ids only to currently un-assigned faces and true to replace all existing ids.
    //! @return SUCCESS if ids could be added.
    DGNPLATFORM_EXPORT static BentleyStatus AddNodeIdAttributes(IBRepEntityR entity, uint32_t nodeId, bool overrideExisting);

    //! Change the topology ids for all faces of the given body to the supplied node id. Useful for ElementGeometryTool sub-classes where
    //! converted geometry can be assigned a node id of 1, or where an existing BRep entity needs it's node ids changed to the current feature's.
    //! The reason to call this over AddNodeIdAttributes is that preserves the non-sequentially assigned geometry and profile specific assigments.
    //! @param[in,out] entity The body to modify.
    //! @param[in] nodeId The non-zero topology node id to use in the nodeId-entityId pairs.
    //! @return SUCCESS if ids were changed or removed.
    //! @note This method does not assign the node id to currently un-assigned faces, it only updates or removes existing ids.
    //!       You should still call AddNodeIdAttributes post-modify to assign any still un-assigned faces and to resolve any duplicates.
    DGNPLATFORM_EXPORT static BentleyStatus ChangeNodeIdAttributes(IBRepEntityR entity, uint32_t nodeId);

    //! Remove the topology ids from all faces of the given body.
    //! @param[in,out] entity The body to modify.
    //! @return SUCCESS if ids could be removed.
    DGNPLATFORM_EXPORT static BentleyStatus DeleteNodeIdAttributes(IBRepEntityR entity);

    //! Increment the topology ids for all faces of the given body. Used to avoid nodeId conflicts between target and tool bodies.
    //! @param[in,out] entity The body to modify.
    //! @param[in] increment The topology node id in each nodeId-entityId pair will be incremented by this amount.
    //! @return SUCCESS if ids could be incremented.
    DGNPLATFORM_EXPORT static BentleyStatus IncrementNodeIdAttributes(IBRepEntityR entity, int32_t increment);

    //! Find the highest and lowest nodeId values from the topology ids currently assigned to the faces of the given body. Used to avoid nodeId conflicts between target and tool bodies.
    //! @param[in] entity The solid or sheet body to inspect.
    //! @param[out] highestNodeId The highest nodeId currently assigned.
    //! @param[out] lowestNodeId The lowest nodeId currently assigned.
    //! @return SUCCESS if face ids are assigned to the body.
    DGNPLATFORM_EXPORT static BentleyStatus FindNodeIdRange(IBRepEntityCR entity, uint32_t& highestNodeId, uint32_t& lowestNodeId);

    //! Assign topology id to the given face. Does not check or resolve duplicate face ids.
    //! @param[in,out] subEntity The face to modify.
    //! @param[in] faceId The face nodeId-entityId pair.
    //! @return SUCCESS if id was added.
    DGNPLATFORM_EXPORT static BentleyStatus AddNodeIdAttribute(ISubEntityR subEntity, FaceId faceId);

    //! Remove topology id from the given face.
    //! @param[in,out] subEntity The face to modify.
    //! @return SUCCESS if id was removed.
    DGNPLATFORM_EXPORT static BentleyStatus DeleteNodeIdAttribute(ISubEntityR subEntity);

    //! Get the FaceId currently assigned to a given face sub-entity.
    //! @param[out] faceId The requested nodeId-entityId pair.
    //! @param[in] subEntity The face sub-entity to query.
    //! @param[in] useHighestId true to return the highest nodeId-entityId pair for this face, false to return the lowest. Typically true.
    //! @return SUCCESS if a FaceId was assigned.
    DGNPLATFORM_EXPORT static BentleyStatus IdFromFace(FaceId& faceId, ISubEntityCR subEntity, bool useHighestId);

    //! Get the EdgeId currently assigned to a given edge sub-entity.
    //! @param[out] edgeId The requested nodeId-entityId pairs.
    //! @param[in] subEntity The edge sub-entity to query.
    //! @param[in] useHighestId true to return the highest nodeId-entityId pairs for this face, false to return the lowest. Typically true.
    //! @return SUCCESS if an EdgeId was assigned.
    DGNPLATFORM_EXPORT static BentleyStatus IdFromEdge(EdgeId& edgeId, ISubEntityCR subEntity, bool useHighestId);

    //! Get the VertexId currently assigned to a given vertex sub-entity.
    //! @param[out] vertexId The requested nodeId-entityId triple.
    //! @param[in] subEntity The vertex sub-entity to query.
    //! @param[in] useHighestId true to return the highest nodeId-entityId triple for this face, false to return the lowest. Typically true.
    //! @return SUCCESS if a VertexId was assigned.
    DGNPLATFORM_EXPORT static BentleyStatus IdFromVertex(VertexId& vertexId, ISubEntityCR subEntity, bool useHighestId);

    //! Get the set of faces of a body having the given FaceId assignment.
    //! @param[out] subEntities The sub-entities found.
    //! @param[in] faceId The FaceId to search for.
    //! @param[in] entity The body to query.
    //! @return SUCCESS if FaceId was assigned.
    DGNPLATFORM_EXPORT static BentleyStatus FacesFromId(bvector<ISubEntityPtr>& subEntities, FaceId const& faceId, IBRepEntityCR entity);

    //! Get the set of edges of a body having the given EdgeId assignment.
    //! @param[out] subEntities The sub-entities found.
    //! @param[in] edgeId The EdgeId to search for.
    //! @param[in] entity The body to query.
    //! @return SUCCESS if EdgeId was assigned.
    DGNPLATFORM_EXPORT static BentleyStatus EdgesFromId(bvector<ISubEntityPtr>& subEntities, EdgeId const& edgeId, IBRepEntityCR entity);

    //! Get the set of vertices of a body having the given VertexId assignment.
    //! @param[out] subEntities The sub-entities found.
    //! @param[in] vertexId The VertexId to search for.
    //! @param[in] entity The body to query.
    //! @return SUCCESS if VertexId was assigned.
    DGNPLATFORM_EXPORT static BentleyStatus VerticesFromId(bvector<ISubEntityPtr>& subEntities, VertexId const& vertexId, IBRepEntityCR entity);
    };

}; // BRepUtil

//=======================================================================================
//! @private
//=======================================================================================
struct BRepDataCache
{
DGNPLATFORM_EXPORT static IBRepEntityPtr FindCachedBRepEntity(DgnElementCR element, GeometryStreamEntryIdCR entryId);
DGNPLATFORM_EXPORT static void AddCachedBRepEntity(DgnElementCR element, GeometryStreamEntryIdCR entryId, IBRepEntityR entity);

}; // BRepDataCache

END_BENTLEY_DGN_NAMESPACE
