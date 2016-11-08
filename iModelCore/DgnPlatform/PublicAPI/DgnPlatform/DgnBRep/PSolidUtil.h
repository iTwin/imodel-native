/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnBRep/PSolidUtil.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <PSolid/frustrum_tokens.h>
#include <PSolid/kernel_interface.h>
#include <PSolid/parasolid_kernel.h>
#include <PSolid/parasolid_debug.h>
#include <PSolid/frustrum_ifails.h>
#include <DgnPlatform/SolidKernel.h>

#define PKI_ENTITY_ID_ATTRIB_NAME       "BSI_EntityId"
#define PKI_USERDATA_ATTRIB_NAME        "BSI_UserData"
#define PKI_HIDDEN_ENTITY_ATTRIB_NAME   "BSI_TriformaAdjacencyAttribute"

BEGIN_BENTLEY_DGN_NAMESPACE

enum PKIBooleanOptionEnum
    {
    PKI_BOOLEAN_OPTION_None                 = 0,
    PKI_BOOLEAN_OPTION_AllowDisjoint        = (1<<0),
    PKI_BOOLEAN_OPTION_SheetSolidFenceNone  = (1<<1),
    };

struct EdgeToCurveIdMap : bmap <uint32_t, CurvePrimitiveIdCP> {};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  01/01
+===============+===============+===============+===============+===============+======*/
struct PSolidTopoId
{
DGNPLATFORM_EXPORT static BentleyStatus IdFromEntity (FaceId& faceId, PK_ENTITY_t entityTag, bool useHighestId);
DGNPLATFORM_EXPORT static bool EntityMatchesId (FaceId const& faceId, PK_ENTITY_t entityTag);

DGNPLATFORM_EXPORT static BentleyStatus IdFromFace(FaceId& faceId, PK_FACE_t faceTag, bool useHighestId);
DGNPLATFORM_EXPORT static BentleyStatus IdFromEdge(EdgeId& edgeId, PK_EDGE_t edgeTag, bool useHighestId);
DGNPLATFORM_EXPORT static BentleyStatus IdFromVertex(VertexId& vertexId, PK_VERTEX_t vertexTag, bool useHighestId);

DGNPLATFORM_EXPORT static BentleyStatus FaceFromId(PK_FACE_t& faceTag, FaceId const& faceId, PK_BODY_t bodyTag);
DGNPLATFORM_EXPORT static BentleyStatus FacesFromId(bvector<PK_FACE_t>& faces, FaceId const& faceId, PK_BODY_t bodyTag); 
DGNPLATFORM_EXPORT static BentleyStatus EdgeFromId(PK_EDGE_t& edgeTag, EdgeId const& edgeId, PK_BODY_t bodyTag);
DGNPLATFORM_EXPORT static BentleyStatus EdgesFromId(bvector<PK_EDGE_t>& edges, EdgeId const& edgeId, PK_BODY_t bodyTag); 
DGNPLATFORM_EXPORT static BentleyStatus VertexFromId(PK_VERTEX_t& vertexTag, VertexId const& vertexId, PK_BODY_t bodyTag);
DGNPLATFORM_EXPORT static BentleyStatus VerticesFromId(bvector<PK_VERTEX_t>& vertices, VertexId const& vertexId, PK_BODY_t bodyTag); 

DGNPLATFORM_EXPORT static BentleyStatus LowestUniqueIdFromEdge(EdgeId& edgeId, PK_EDGE_t edgeTag);
DGNPLATFORM_EXPORT static BentleyStatus FacesFromNodeId(bvector<PK_FACE_t>& faces, uint32_t nodeId, PK_BODY_t bodyTag); 
DGNPLATFORM_EXPORT static BentleyStatus CurveTopologyIdFromEdge(CurveTopologyId& assocCurveId, PK_EDGE_t edgeTag, bool useHighestId = true);

DGNPLATFORM_EXPORT static BentleyStatus AssignConeFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);
DGNPLATFORM_EXPORT static BentleyStatus AssignSlabFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);
DGNPLATFORM_EXPORT static BentleyStatus AssignTorusFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);
DGNPLATFORM_EXPORT static BentleyStatus AssignProfileBodyIds(PK_BODY_t bodyTag, uint32_t nodeId);
DGNPLATFORM_EXPORT static uint32_t AssignSweptProfileLateralIds(int nLaterals, int* baseArray, int* laterals);
DGNPLATFORM_EXPORT static BentleyStatus AssignEdgeIds(PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting); // NOTE: Edges normally identified by a pair of faces, not this attribute stored on the edge...
DGNPLATFORM_EXPORT static BentleyStatus AssignFaceIds(PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting);

DGNPLATFORM_EXPORT static BentleyStatus FindNodeIdRange(PK_BODY_t bodyTag, uint32_t& highestNodeId, uint32_t& lowestNodeId);
DGNPLATFORM_EXPORT static int EntityAttribCompare(PK_ENTITY_t entityTag1, PK_ENTITY_t entityTag2);
DGNPLATFORM_EXPORT static BentleyStatus ExtractEntityIdForNodeId(uint32_t& foundEntityId, bool& uniformNodeIds, PK_ENTITY_t entityTag, uint32_t findNodeId, bool useHighest);
DGNPLATFORM_EXPORT static BentleyStatus ResolveDuplicateFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);

DGNPLATFORM_EXPORT static BentleyStatus AddNodeIdAttributes(PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting);
DGNPLATFORM_EXPORT static BentleyStatus AddNewNodeIdAttributes(PK_BODY_t bodyTag, uint32_t nodeId);
DGNPLATFORM_EXPORT static BentleyStatus IncrementNodeIdAttributes(PK_BODY_t bodyTag, int32_t increment);
DGNPLATFORM_EXPORT static BentleyStatus DeleteNodeIdAttributes(PK_BODY_t bodyTag);

DGNPLATFORM_EXPORT static BentleyStatus AttachEntityId(PK_ENTITY_t entityTagIn, uint32_t nodeIdIn, uint32_t entityIdIn);
DGNPLATFORM_EXPORT static void AskEntityId(uint32_t* pNodeIdOut, uint32_t *pEntityIdOut, PK_ATTRIB_t attribTagIn);
DGNPLATFORM_EXPORT static void DeleteEntityId(PK_ENTITY_t entityTag);

}; // PSolidTopoId

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  01/01
+===============+===============+===============+===============+===============+======*/
struct PSolidAttrib
{
DGNPLATFORM_EXPORT static void GetAttrib(int* pNumAttribOut, PK_ATTRIB_t** ppAttribArrayOut, PK_ENTITY_t entityTagIn, char *pAttribNameIn);
DGNPLATFORM_EXPORT static void DeleteAttrib(PK_ENTITY_t entityTagIn, char* pAttribNameIn);

DGNPLATFORM_EXPORT static BentleyStatus GetHiddenAttribute(bool& isHidden, PK_ENTITY_t entityTag);
DGNPLATFORM_EXPORT static BentleyStatus SetHiddenAttribute(PK_ENTITY_t entityTag, bool isHidden);
DGNPLATFORM_EXPORT static void DeleteHiddenAttribute(PK_ENTITY_t entityTag);

DGNPLATFORM_EXPORT static BentleyStatus GetHiddenBodyFaces(bset<PK_FACE_t>& faces, PK_BODY_t body);
DGNPLATFORM_EXPORT static BentleyStatus GetHiddenBodyEdges(bset<PK_EDGE_t>& edges, PK_BODY_t body);
DGNPLATFORM_EXPORT static void DeleteHiddenAttributeOnEdges(PK_BODY_t entityTag);
DGNPLATFORM_EXPORT static void DeleteHiddenAttributeOnFaces(PK_BODY_t entityTag);

DGNPLATFORM_EXPORT static bool HasHiddenEdge(PK_BODY_t entityTag);
DGNPLATFORM_EXPORT static bool HasHiddenFace(PK_BODY_t entityTag);
DGNPLATFORM_EXPORT static bool IsEntityHidden(PK_ENTITY_t entity);

DGNPLATFORM_EXPORT static BentleyStatus GetUserAttributes(bvector<int32_t>& attributes, PK_ENTITY_t entity, int32_t ownerId, int findIndex = 0);

}; // PSolidAttrib

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  01/01
+===============+===============+===============+===============+===============+======*/
struct PSolidTopo
{
DGNPLATFORM_EXPORT static BentleyStatus GetBodyFaces(bvector<PK_FACE_t>& faces, PK_BODY_t body);
DGNPLATFORM_EXPORT static BentleyStatus GetBodyEdges(bvector<PK_EDGE_t>& edges, PK_BODY_t body);
DGNPLATFORM_EXPORT static BentleyStatus GetBodyVertices(bvector<PK_VERTEX_t>& vertices, PK_BODY_t body);
DGNPLATFORM_EXPORT static BentleyStatus GetBodyEdgesAndFaces(bvector<PK_ENTITY_t>& edgesAndFaces, PK_BODY_t body);

DGNPLATFORM_EXPORT static BentleyStatus GetFaceEdges(bvector<PK_EDGE_t>& edges, PK_FACE_t face);
DGNPLATFORM_EXPORT static BentleyStatus GetFaceVertices(bvector<PK_VERTEX_t>& vertices, PK_FACE_t face);
DGNPLATFORM_EXPORT static BentleyStatus GetFaceLoops(bvector<PK_LOOP_t>& edges, PK_FACE_t face);

DGNPLATFORM_EXPORT static BentleyStatus GetEdgeFaces(bvector<PK_FACE_t>& faces, PK_EDGE_t edge);
DGNPLATFORM_EXPORT static BentleyStatus GetEdgeVertices(bvector<PK_VERTEX_t>& vertices, PK_EDGE_t edge);
DGNPLATFORM_EXPORT static BentleyStatus GetEdgeFins(bvector<PK_FIN_t>& fins, PK_EDGE_t edge);

DGNPLATFORM_EXPORT static BentleyStatus GetVertexFaces(bvector<PK_FACE_t>& faces, PK_VERTEX_t vertex);
DGNPLATFORM_EXPORT static BentleyStatus GetVertexEdges(bvector<PK_EDGE_t>& edges, PK_VERTEX_t vertex);

DGNPLATFORM_EXPORT static BentleyStatus GetLoopFins(bvector<PK_FIN_t>& fins, PK_LOOP_t loop);
DGNPLATFORM_EXPORT static BentleyStatus GetLoopEdgesFromEdge(bvector<PK_EDGE_t>& loopEdges, PK_EDGE_t edge, PK_FACE_t face);
DGNPLATFORM_EXPORT static BentleyStatus GetTangentBlendEdges(bvector<PK_EDGE_t>& smoothEdges, PK_EDGE_t edgeTag);
DGNPLATFORM_EXPORT static BentleyStatus GetCurveOfEdge(PK_CURVE_t& curveTagOut, double* startParamP, double* endParamP, bool* reversedP, PK_EDGE_t edgeTagIn);

}; // PSolidTopo

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  01/01
+===============+===============+===============+===============+===============+======*/
struct PSolidGeom
{
DGNPLATFORM_EXPORT static BentleyStatus CreateBCurveFromSPCurve(PK_BCURVE_t& bCurveTag, PK_SPCURVE_t spCurve, PK_INTERVAL_t* intervalP, PK_LOGICAL_t& isExact, bool makeNonPeriodic = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineCurveFromBCurve(MSBsplineCurveR curve, PK_BCURVE_t curveTag, bool normalizeKnotVector = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineCurveFromSPCurve(MSBsplineCurveR curve, PK_SPCURVE_t spCurveTag, PK_INTERVAL_t* intervalP, bool* isExactP = NULL);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineCurveFromCurve(MSBsplineCurveR curve, PK_CURVE_t curveTag, PK_INTERVAL_t& interval, bool reverse = false, double tolerance = 1.0E-5, bool* isExactP = NULL);
DGNPLATFORM_EXPORT static BentleyStatus CreateCurveFromMSBsplineCurve(PK_CURVE_t* curveTag, MSBsplineCurveCR curve);
DGNPLATFORM_EXPORT static BentleyStatus CreateCurveFromMSBsplineCurve2d(PK_CURVE_t* curveTag, MSBsplineCurveCR curve);

DGNPLATFORM_EXPORT static BentleyStatus CreateBSurfaceFromSurface(PK_BSURF_t& bSurfaceTag, PK_PARAM_sf_t param[2], PK_SURF_t surfTag, bool makeNonPeriodic = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineSurfaceFromFace(MSBsplineSurfaceR surface, PK_FACE_t faceTag, int uRules, int vRules, double tolerance);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineSurfaceFromSurface(MSBsplineSurfaceR surface, PK_SURF_t surfTag, PK_SURF_trim_data_t* trimData, PK_GEOM_t* spaceCurves, PK_INTERVAL_t* interval, int uRules, int vRules, double tolerance, bool normalizeSurface = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateSurfaceFromMSBsplineSurface(PK_BSURF_t* surfTag, MSBsplineSurfaceCR surface);
DGNPLATFORM_EXPORT static BentleyStatus CreateSheetBodyFromTrimmedSurface(PK_ENTITY_t* bodyTagP, PK_ENTITY_t** spaceCurveEntitiesPP, PK_ENTITY_t** uvCurveEntitiesPP, int preferSpaceCurvesFlag, double** trimPP, int boundCount, PK_ENTITY_t surfaceTag, double tolerance, int adaptive);

DGNPLATFORM_EXPORT static BentleyStatus BodyFromGPA(PK_BODY_t* bodyTag, PK_VERTEX_t* startVertexP, GPArrayCP gpa, TransformCR gpaToBodyTransform, bool cap);
DGNPLATFORM_EXPORT static BentleyStatus BodyFromCurveVector(PK_BODY_t& bodyTag, PK_VERTEX_t* startVertexP, CurveVectorCR profile, TransformCR curveToBodyTransform, bool coverClosed = true, EdgeToCurveIdMap* idMap = NULL);
DGNPLATFORM_EXPORT static BentleyStatus BodyFromPolyface(PK_BODY_t& bodyTag, PolyfaceQueryCR polyface, TransformCR dgnToSolid);
DGNPLATFORM_EXPORT static BentleyStatus BodyFromMSBsplineSurface(PK_BODY_t& bodyTag, MSBsplineSurfaceCR surface);
DGNPLATFORM_EXPORT static BentleyStatus BodyFromCone(PK_BODY_t& bodyTag, RotMatrixCR rMatrix, DPoint3dCR topCenter, DPoint3dCR bottomCenter, double topRadius, double bottomRadius, bool capped, uint32_t nodeId);
DGNPLATFORM_EXPORT static BentleyStatus BodyFromLoft(PK_BODY_t& bodyTag, PK_BODY_t* profiles, PK_VERTEX_t* startVertices, size_t nProfiles, PK_BODY_t* guides, size_t nGuides);
DGNPLATFORM_EXPORT static BentleyStatus BodyFromSweep(PK_BODY_t& bodyTag, PK_BODY_t profileTag, PK_BODY_t pathTag, PK_VERTEX_t pathVertex, bool alignParallel, bool selfRepair, DVec3dCP lockDirection, double const* twistAngle, double const* scale, DPoint3dCP scalePoint);

DGNPLATFORM_EXPORT static CurveVectorPtr FaceToUVCurveVector(PK_FACE_t faceTag, PK_UVBOX_t* uvBox, bool splineParameterization);
DGNPLATFORM_EXPORT static CurveVectorPtr PlanarFaceToCurveVector(PK_FACE_t face, EdgeToCurveIdMap const* idMap = nullptr);
DGNPLATFORM_EXPORT static ISolidPrimitivePtr FaceToSolidPrimitive(PK_FACE_t faceTag, CurveVectorPtr* uvBoundaries = nullptr);
DGNPLATFORM_EXPORT static StatusInt FaceToBSplineSurface(MSBsplineSurfacePtr& bSurface, CurveVectorPtr& uvBoundaries, PK_FACE_t faceTag);

DGNPLATFORM_EXPORT static ICurvePrimitivePtr GetAsCurvePrimitive(PK_CURVE_t curve, PK_INTERVAL_t interval, bool reverseDirection);
DGNPLATFORM_EXPORT static BentleyStatus EdgeToCurvePrimitive(ICurvePrimitivePtr& curvePrimitive, PK_EDGE_t edgeTag);
DGNPLATFORM_EXPORT static BentleyStatus BodyToCurveVectors(bvector<CurveVectorPtr>& curves, IBRepEntityCR entity, EdgeToCurveIdMap const* idMap = NULL);
DGNPLATFORM_EXPORT static CurveVectorPtr WireBodyToCurveVector(IBRepEntityCR entity);
DGNPLATFORM_EXPORT static CurveVectorPtr PlanarSheetBodyToCurveVector(IBRepEntityCR entity);

DGNPLATFORM_EXPORT static BentleyStatus BodyFromCurveVector(IBRepEntityPtr& entityOut, CurveVectorCR curveVector, TransformCP curveToDgn = NULL, uint32_t nodeId = 0L, EdgeToCurveIdMap* idMap = NULL); //!< Calls PSolidKernelManager::StartSession.
DGNPLATFORM_EXPORT static BentleyStatus BodyFromSolidPrimitive(IBRepEntityPtr& out, ISolidPrimitiveCR primitive, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
DGNPLATFORM_EXPORT static BentleyStatus BodyFromBSurface(IBRepEntityPtr& out, MSBsplineSurfaceCR surface, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
DGNPLATFORM_EXPORT static BentleyStatus BodyFromPolyface(IBRepEntityPtr& out, PolyfaceQueryCR meshData, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
DGNPLATFORM_EXPORT static BentleyStatus BodyFromLoft(IBRepEntityPtr& out, CurveVectorPtr* profiles, size_t nProfiles, CurveVectorPtr* guides, size_t nGuides, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
DGNPLATFORM_EXPORT static BentleyStatus BodyFromSweep(IBRepEntityPtr& out, CurveVectorCR profile, CurveVectorCR path, bool alignParallel, bool selfRepair, bool createSheet,DVec3dCP lockDirection, double const* twistAngle, double const* scale, DPoint3dCP scalePoint, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.

}; // PSolidGeom

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  01/01
+===============+===============+===============+===============+===============+======*/
struct PSolidSubEntity
{
DGNPLATFORM_EXPORT static ISubEntityPtr CreateSubEntity(PK_ENTITY_t entityTag, TransformCR entityTransform);
DGNPLATFORM_EXPORT static ISubEntityPtr CreateSubEntity(PK_ENTITY_t entityTag, IBRepEntityCR parent);

DGNPLATFORM_EXPORT static void UpdateCache(ISubEntityR subEntity, ISubEntityCR donorEntity); //!< @private LocateSubEntityTool use.
DGNPLATFORM_EXPORT static void SetLocation(ISubEntityR subEntity, DPoint3dCR point, DPoint2dCR param); //!< @private param x/y is face u/v, param x is edge u, na for vertex...

DGNPLATFORM_EXPORT static PK_ENTITY_t GetSubEntityTag(ISubEntityCR);
DGNPLATFORM_EXPORT static Transform GetSubEntityTransform(ISubEntityCR);

DGNPLATFORM_EXPORT static bool GetFaceLocation(ISubEntityCR subEntity, DPoint3dR point, DPoint2dR param);
DGNPLATFORM_EXPORT static bool GetEdgeLocation(ISubEntityCR subEntity, DPoint3dR point, double& uParam);
DGNPLATFORM_EXPORT static bool GetVertexLocation(ISubEntityCR subEntity, DPoint3dR point);

}; // PSolidSubEntity

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  01/01
+===============+===============+===============+===============+===============+======*/
struct PSolidUtil
{
DGNPLATFORM_EXPORT static IBRepEntityPtr CreateNewEntity(PK_ENTITY_t entityTag, TransformCR entityTransform, bool owned = true); //!< NOTE: Will return an invalid entity if entity tag is not valid.
DGNPLATFORM_EXPORT static IBRepEntityPtr InstanceEntity(IBRepEntityCR); //!< Create non-owning instance of an existing entity...

DGNPLATFORM_EXPORT static BentleyStatus SaveEntityToMemory(uint8_t** ppBuffer, size_t& bufferSize, IBRepEntityCR); //!< NOTE: The entity transform must be saved separately.
DGNPLATFORM_EXPORT static BentleyStatus RestoreEntityFromMemory(IBRepEntityPtr&, uint8_t const* pBuffer, size_t bufferSize, TransformCR); //!< Calls PSolidKernelManager::StartSession.

//! Get the tag value that the solid kernel uses to identify the entity.
//! @note To be used by application in query operations. Do not pass this to any kernel api that will modify the entity, use GetEntityTagForModify instead.
//! @return The tag value.
DGNPLATFORM_EXPORT static PK_ENTITY_t GetEntityTag(IBRepEntityCR, bool* isOwned = nullptr);

//! Get the tag value of the solid kernel entity suitable for modification by first attempting ExtractEntityTag, and if it fails creating a copy of the entity.
//! @note To be used by application in operations where the entity is consumed or modified such as being a tool body in a union.
//! @return The tag value.
DGNPLATFORM_EXPORT static PK_ENTITY_t GetEntityTagForModify(IBRepEntityR entity);

//! Extract and take ownership of solid kernel entity associated with this instance. Input entity will be set to the null tag and the caller is responsible for freeing extracted entity.
//! @note To be used by application in operations where the entity is consumed or modified such as being a tool body in a union.
//! @remarks This method will return a null tag if the entity isn't "owned" by this instance (Application may then choose to copy the entity, see GetEntityTagForModify).
DGNPLATFORM_EXPORT static PK_ENTITY_t ExtractEntityTag(IBRepEntityR entity);

//! Return body tag for supplied face, edge, or vertex topological entity.
DGNPLATFORM_EXPORT static PK_BODY_t GetBodyForEntity(PK_ENTITY_t entityTag);

//! Get axis aligned bounding box for the supplied body, face, or edge topological entity.
DGNPLATFORM_EXPORT static BentleyStatus GetEntityRange(DRange3dR range, PK_TOPOL_t entity);

DGNPLATFORM_EXPORT static IFaceMaterialAttachmentsPtr CreateNewFaceAttachments(PK_ENTITY_t entityTag, Render::GeometryParamsCR baseParams);
DGNPLATFORM_EXPORT static void SetFaceAttachments(IBRepEntityR, IFaceMaterialAttachmentsP);

DGNPLATFORM_EXPORT static PolyfaceHeaderPtr FacetEntity(IBRepEntityCR entity, double pixelSize=0.0, DRange1dP pixelSizeRange=nullptr);
DGNPLATFORM_EXPORT static PolyfaceHeaderPtr FacetEntity(IBRepEntityCR entity, IFacetOptionsR);
DGNPLATFORM_EXPORT static bool FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<Render::GeometryParams>& params, double pixelSize=0.0, DRange1dP pixelSizeRange=nullptr);
DGNPLATFORM_EXPORT static bool FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<Render::GeometryParams>& params, IFacetOptionsR facetOptions);

DGNPLATFORM_EXPORT static bool HasCurvedFaceOrEdge(PK_BODY_t entityTag);
DGNPLATFORM_EXPORT static bool HasOnlyPlanarFaces(PK_BODY_t entityTag);
DGNPLATFORM_EXPORT static bool IsSmoothEdge(PK_EDGE_t edgeTag);
DGNPLATFORM_EXPORT static bool IsPlanarFace(PK_FACE_t faceTag);
 
DGNPLATFORM_EXPORT static BentleyStatus GetPlanarFaceData (DPoint3dP point, DVec3dP normal, PK_FACE_t entityTag);
DGNPLATFORM_EXPORT static BentleyStatus EvaluateFace(DPoint3dR point, DVec3dR normal, DVec3dR uDir, DVec3dR vDir, DPoint2dCR uvParam, PK_FACE_t faceTag);
DGNPLATFORM_EXPORT static BentleyStatus EvaluateEdge(DPoint3dR point, DVec3dR tangent, double uParam, PK_EDGE_t edgeTag);
DGNPLATFORM_EXPORT static BentleyStatus GetVertex(DPoint3dR point, PK_VERTEX_t vertexTag);

DGNPLATFORM_EXPORT static void ExtractStartAndSweepFromInterval(double& start, double& sweep, PK_INTERVAL_t const& interval, bool reverse);
DGNPLATFORM_EXPORT static BentleyStatus MakeEllipseCurve(PK_CURVE_t* curveTag, double* startParam, double* endParam, DPoint3dP center, RotMatrixP rMatrix, double x1, double x2, double startAngle, double sweepAngle);
DGNPLATFORM_EXPORT static BentleyStatus ImprintSegment(PK_BODY_t bodyTag, PK_EDGE_t* edgeTag, DPoint3dCP segment);
DGNPLATFORM_EXPORT static BentleyStatus CoverWires(PK_BODY_t bodyTag);

DGNPLATFORM_EXPORT static double CalculateToleranceFromMinCurvature(PK_CURVE_t curveTag, PK_INTERVAL_t* intervalP);
DGNPLATFORM_EXPORT static void NormalizeBsplineCurve(MSBsplineCurveR curve);
DGNPLATFORM_EXPORT static BentleyStatus FixupNonG1BodyGeometry(PK_BODY_t bodyTag);

DGNPLATFORM_EXPORT static BentleyStatus CreateTransf(PK_TRANSF_t& transfTag, TransformCR transform);
DGNPLATFORM_EXPORT static BentleyStatus ApplyTransform(PK_BODY_t bodyTag, PK_TRANSF_t transfTag);
DGNPLATFORM_EXPORT static void GetTransforms(TransformR solidToUor, TransformR uorToSolid, DPoint3dCP origin = nullptr, double solidScale = 1.0);

DGNPLATFORM_EXPORT static BentleyStatus ClipCurveVector(bvector<CurveVectorPtr>& output, CurveVectorCR input, ClipVectorCR clipVector, TransformCP transformToDgn);
DGNPLATFORM_EXPORT static BentleyStatus ClipBody(bvector<IBRepEntityPtr>& output, bool& clipped, IBRepEntityCR input, ClipVectorCR clipVector);
DGNPLATFORM_EXPORT static BentleyStatus ConvertSolidBodyToSheet(PK_BODY_t body);

DGNPLATFORM_EXPORT static BentleyStatus SweepBodyVector(PK_BODY_t bodyTag, DVec3dCR direction, double distance);
DGNPLATFORM_EXPORT static BentleyStatus SweepBodyAxis(PK_BODY_t bodyTag, DVec3dCR revolveAxis, DPoint3dCR center, double sweep);

DGNPLATFORM_EXPORT static BentleyStatus Boolean(PK_BODY_t** ppResultBodies, int* pNumResultBodies, PK_boolean_function_t boolOpIn, bool generalTopology, PK_BODY_t blankBodyIn, PK_BODY_t* pToolBodies, int numToolBodiesIn, PKIBooleanOptionEnum booleanOptions);
DGNPLATFORM_EXPORT static BentleyStatus DisjoinBody(bvector<PK_BODY_t>& bodies, PK_BODY_t body);
DGNPLATFORM_EXPORT static BentleyStatus TransformBody(PK_BODY_t body, TransformCR transform);

DGNPLATFORM_EXPORT static BentleyStatus CheckBody(PK_BODY_t body, bool checkGeometry, bool checkTopology, bool checkSize);
DGNPLATFORM_EXPORT static bool AreBodiesEqual(PK_BODY_t body1Tag, PK_BODY_t body2Tag, double tolerance, TransformCP deltaTransform1To2);
DGNPLATFORM_EXPORT static BentleyStatus MassProperties(double* amount, double* periphery, DPoint3dP centroid, double inertia[3][3], PK_BODY_t bodyTag, TransformCP transform, double tolerance);

DGNPLATFORM_EXPORT static BentleyStatus DoBoolean(IBRepEntityPtr& targetEntity, IBRepEntityPtr* toolEntities, size_t nTools, PK_boolean_function_t operation, PKIBooleanOptionEnum options = PKI_BOOLEAN_OPTION_AllowDisjoint, bool assignNodeIds = true);
DGNPLATFORM_EXPORT static bool LocateSubEntities(PK_ENTITY_t bodyTag, TransformCR bodyTransform, bvector<PK_ENTITY_t>& subEntities, bvector<DPoint3d>& intersectPts, bvector<DPoint2d>& intersectParams, size_t maxFace, size_t maxEdge, size_t maxVertex, DRay3dCR boresite, double maxEdgeDistance, double maxVertexDistance);
DGNPLATFORM_EXPORT static bool ClosestPoint(PK_ENTITY_t bodyTag, TransformCR bodyTransform, PK_ENTITY_t& entityTag, DPoint3dR point, DPoint2dR param, double& distance, DPoint3dCR testPt);
DGNPLATFORM_EXPORT static bool ClosestPointToFace(PK_FACE_t faceTag, TransformCR bodyTransform, DPoint3dR point, DPoint2dR uvParam, double& distance, DPoint3dCR testPt);
DGNPLATFORM_EXPORT static bool ClosestPointToEdge(PK_EDGE_t edgeTag, TransformCR bodyTransform, DPoint3dR point, double& uParam, double& distance, DPoint3dCR testPt);

}; // PSolidUtil

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  12/2009
+===============+===============+===============+===============+===============+======*/
struct PSolidKernelManager
{
DGNPLATFORM_EXPORT static void StartSession();
DGNPLATFORM_EXPORT static void StopSession();
DGNPLATFORM_EXPORT static void SetExternalFrustrum(); // Frustrum registered and session started by external dll (V8 convert)...

}; // PSolidKernelManager

END_BENTLEY_DGN_NAMESPACE

