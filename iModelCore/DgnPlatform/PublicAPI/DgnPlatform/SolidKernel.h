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

BEGIN_BENTLEY_DGN_NAMESPACE

typedef RefCountedPtr<IFaceMaterialAttachments> IFaceMaterialAttachmentsPtr; //!< Reference counted type to manage the life-cycle of the IFaceMaterialAttachments.

//=======================================================================================
//! @private
//! Facet table per-face material and color inforation.
//=======================================================================================
struct FaceAttachment
{
private:

    bool            m_useColor:1;       //!< true - color/transparency does not follow sub-category appearance.
    bool            m_useMaterial:1;    //!< true - material does not follow sub-category appearance.
    ColorDef        m_color;
    double          m_transparency;
    DgnMaterialId   m_material;
    DPoint2d        m_uv;

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

//! Returns face identifier for T_FaceToSubElemIdMap pair from face, edge, or vertex sub-entity.
DGNPLATFORM_EXPORT static uint32_t GetFaceIdentifierFromSubEntity(ISubEntityCR);

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
virtual Render::GraphicBuilderPtr _GetGraphic(ViewContextR) const = 0;
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
Render::GraphicBuilderPtr GetGraphic(ViewContextR context) const {return _GetGraphic(context);}

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
DGNPLATFORM_EXPORT static bool Locate(IBRepEntityCR entity, DRay3dCR boresite, bvector<ISubEntityPtr>& intersectEntities, size_t maxFace, size_t maxEdge, size_t maxVertex, double maxEdgeDistance, double maxVertexDistance);

//! Find the closest sub-entity on body to a given point.
//! @param[in] entity The entity to find the closest sub-entity for.
//! @param[in] testPt The space point.
//! @return the face, edge, or vertex sub-entity that contains the closest point.
DGNPLATFORM_EXPORT static ISubEntityPtr ClosestSubEntity(IBRepEntityCR entity, DPoint3dCR testPt);

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
    };

//! Support for modification of bodies.
struct Modify
    {
    //! Modify the target body by intersecting with one or more tool bodies.
    //! @param[in,out] target The target body to modify.
    //! @param[in,out] tools A list of one or more tool bodies (consumed in boolean).
    //! @param[in] nTools Count of tool bodies.
    //! @return SUCCESS if boolean operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus BooleanIntersect(IBRepEntityPtr& target, IBRepEntityPtr* tools, size_t nTools);

    //! Modify the target body by subtracting one or more tool bodies.
    //! @param[in,out] target The target body to modify.
    //! @param[in,out] tools Array of one or more tool bodies (consumed in boolean).
    //! @param[in] nTools Count of tool bodies.
    //! @return SUCCESS if boolean operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus BooleanSubtract(IBRepEntityPtr& target, IBRepEntityPtr* tools, size_t nTools);

    //! Modify the target body by uniting with one or more tool bodies.
    //! @param[in,out] target The target body to modify.
    //! @param[in,out] tools Array of one or more tool bodies (consumed in boolean).
    //! @param[in] nTools Count of tool bodies.
    //! @return SUCCESS if boolean operation was completed.
    DGNPLATFORM_EXPORT static BentleyStatus BooleanUnion(IBRepEntityPtr& target, IBRepEntityPtr* tools, size_t nTools);

    //! Sew the given set of sheet bodies together by joining those that share edges in common.
    //! @param[out] sewn The new bodies produced by sewing.
    //! @param[out] unsewn The bodies that were not able to be sewn.
    //! @param[in,out] tools The array of sheet bodies. (invalidated after sew).
    //! @param[in] nTools Count of tool bodies.
    //! @param[in] gapWidthBound Defines a limit on the width of the gap between sheet body edges that will be allowed to remain.
    //! @param[in] nIterations To request repeated sew attempts that automatically increase gap up to limit set by gapWidthBound.
    //! @return SUCCESS if some bodies were able to be sewn together.
    DGNPLATFORM_EXPORT static BentleyStatus SewBodies(bvector<IBRepEntityPtr>& sewn, bvector<IBRepEntityPtr>& unsewn, IBRepEntityPtr* tools, size_t nTools, double gapWidthBound, size_t nIterations = 1);

    //! 
    DGNPLATFORM_EXPORT static BentleyStatus DisjoinBody(bvector<IBRepEntityPtr>& output, IBRepEntityR entity);
    };

}; // BRepUtil

END_BENTLEY_DGN_NAMESPACE
