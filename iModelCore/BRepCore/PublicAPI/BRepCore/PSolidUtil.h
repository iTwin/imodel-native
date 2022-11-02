/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <PSolid/frustrum_tokens.h>
#include <PSolid/kernel_interface.h>
#include <PSolid/parasolid_kernel.h>
#include <PSolid/parasolid_debug.h>
#include <PSolid/frustrum_ifails.h>
#include <BRepCore/SolidKernel.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <Bentley/Logging.h>

#define LOG_BREPCORE (NativeLogging::CategoryLogger("BRepCore"))

#define PKI_ENTITY_ID_ATTRIB_NAME       "BSI_EntityId"
#define PKI_USERDATA_ATTRIB_NAME        "BSI_UserData"
#define PKI_HIDDEN_ENTITY_ATTRIB_NAME   "BSI_TriformaAdjacencyAttribute"
#define PKI_FACE_MATERIAL_ATTRIB_NAME   "BSI_FaceMatIdx"

BEGIN_BENTLEY_DGN_NAMESPACE

enum PKIBooleanOptionEnum
    {
    PKI_BOOLEAN_OPTION_None                 = 0,
    PKI_BOOLEAN_OPTION_AllowDisjoint        = (1<<0),
    PKI_BOOLEAN_OPTION_SheetSolidFenceNone  = (1<<1),
    };

struct EdgeToCurveIdMap : bmap <uint32_t, CurvePrimitiveIdCP> {};

struct IParasolidWireOutput
{
virtual BentleyStatus _ProcessGoOutput(ICurvePrimitiveCR curve, PK_ENTITY_t entity) = 0;
};

struct IParasolidHLineOutput
{
virtual bool _ReturnHidden() = 0;
virtual bool _IncludeTraceEdges() = 0;
virtual BentleyStatus _ProcessGoOutput(ICurvePrimitiveCR curve, PK_ENTITY_t entity, bool isHidden, bool isSmooth, int type) = 0;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidGoOutput
{
//! Output silhouette curves for the supplied entity and view information.
//! @param[in] output IParasolidWireOutput to process each silhouette curve.
//! @param[in] eyePoint The eye point (nullptr if parallel).
//! @param[in] direction The direction toward the view (positive Z)
//! @param[in] entityTag The input entity to compute silhouettes for.
//! @param[in] tolerance The curve chord tolerance.
//! @remarks This method outputs silhouette curves through GO and is NOT thread safe.
BREPCORE_EXPORT static void ProcessSilhouettes(IParasolidWireOutput& output, DPoint3dCP eyePoint, DVec3dCR direction, PK_ENTITY_t entityTag, double tolerance = 0.0);

//! Output hatch curves for the supplied face sub-entity and snap divisor.
//! @param[in] output IParasolidWireOutput to process each hatch curve.
//! @param[in] divisor The snap divisor to control how many hatch curves to output.
//! @param[in] entityTag The input face entity to compute hatching for.
//! @param[in] tolerance The tolerance for hatching of a parametric face.
//! @remarks This method outputs face hatch curves through GO and is NOT thread safe.
BREPCORE_EXPORT static void ProcessFaceHatching(IParasolidWireOutput& output, uint32_t divisor, PK_FACE_t entityTag, double tolerance = 0.0);

}; // PSolidGoOutput

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidTopoId
{
BREPCORE_EXPORT static BentleyStatus IdFromEntity (FaceId& faceId, PK_ENTITY_t entityTag, bool useHighestId);
BREPCORE_EXPORT static bool EntityMatchesId (FaceId const& faceId, PK_ENTITY_t entityTag);

BREPCORE_EXPORT static BentleyStatus IdFromFace(FaceId& faceId, PK_FACE_t faceTag, bool useHighestId);
BREPCORE_EXPORT static BentleyStatus IdFromEdge(EdgeId& edgeId, PK_EDGE_t edgeTag, bool useHighestId);
BREPCORE_EXPORT static BentleyStatus IdFromVertex(VertexId& vertexId, PK_VERTEX_t vertexTag, bool useHighestId);

BREPCORE_EXPORT static BentleyStatus FaceFromId(PK_FACE_t& faceTag, FaceId const& faceId, PK_BODY_t bodyTag);
BREPCORE_EXPORT static BentleyStatus FacesFromId(bvector<PK_FACE_t>& faces, FaceId const& faceId, PK_BODY_t bodyTag);
BREPCORE_EXPORT static BentleyStatus EdgeFromId(PK_EDGE_t& edgeTag, EdgeId const& edgeId, PK_BODY_t bodyTag);
BREPCORE_EXPORT static BentleyStatus EdgesFromId(bvector<PK_EDGE_t>& edges, EdgeId const& edgeId, PK_BODY_t bodyTag);
BREPCORE_EXPORT static BentleyStatus VertexFromId(PK_VERTEX_t& vertexTag, VertexId const& vertexId, PK_BODY_t bodyTag);
BREPCORE_EXPORT static BentleyStatus VerticesFromId(bvector<PK_VERTEX_t>& vertices, VertexId const& vertexId, PK_BODY_t bodyTag);

BREPCORE_EXPORT static BentleyStatus LowestUniqueIdFromEdge(EdgeId& edgeId, PK_EDGE_t edgeTag);
BREPCORE_EXPORT static BentleyStatus FacesFromNodeId(bvector<PK_FACE_t>& faces, uint32_t nodeId, PK_BODY_t bodyTag);
BREPCORE_EXPORT static BentleyStatus CurveTopologyIdFromEdge(CurveTopologyId& assocCurveId, PK_EDGE_t edgeTag, bool useHighestId = true);

BREPCORE_EXPORT static BentleyStatus AssignConeFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);
BREPCORE_EXPORT static BentleyStatus AssignSlabFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);
BREPCORE_EXPORT static BentleyStatus AssignTorusFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);
BREPCORE_EXPORT static BentleyStatus AssignProfileBodyIds(PK_BODY_t bodyTag, uint32_t nodeId, bool singleHoleLoopPriority = false);
BREPCORE_EXPORT static uint32_t AssignSweptProfileLateralIds(int nLaterals, int* baseArray, int* laterals);
BREPCORE_EXPORT static BentleyStatus AssignEdgeIds(PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting); // NOTE: Edges normally identified by a pair of faces, not this attribute stored on the edge...
BREPCORE_EXPORT static BentleyStatus AssignFaceIds(PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting);

BREPCORE_EXPORT static BentleyStatus FindNodeIdRange(PK_BODY_t bodyTag, uint32_t& highestNodeId, uint32_t& lowestNodeId);
BREPCORE_EXPORT static int EntityAttribCompare(PK_ENTITY_t entityTag1, PK_ENTITY_t entityTag2);
BREPCORE_EXPORT static BentleyStatus ExtractEntityIdForNodeId(uint32_t& foundEntityId, bool& uniformNodeIds, PK_ENTITY_t entityTag, uint32_t findNodeId, bool useHighest);
BREPCORE_EXPORT static BentleyStatus ResolveDuplicateFaceIds(PK_BODY_t bodyTag, uint32_t nodeId);

BREPCORE_EXPORT static BentleyStatus AddNodeIdAttributes(PK_BODY_t bodyTag, uint32_t nodeId, bool overrideExisting);
BREPCORE_EXPORT static BentleyStatus AddNewNodeIdAttributes(PK_BODY_t bodyTag, uint32_t nodeId);
BREPCORE_EXPORT static BentleyStatus IncrementNodeIdAttributes(PK_BODY_t bodyTag, int32_t increment);
BREPCORE_EXPORT static BentleyStatus ChangeNodeIdAttributes(PK_BODY_t bodyTag, uint32_t nodeId);
BREPCORE_EXPORT static BentleyStatus DeleteNodeIdAttributes(PK_BODY_t bodyTag);

BREPCORE_EXPORT static BentleyStatus AttachEntityId(PK_ENTITY_t entityTagIn, uint32_t nodeIdIn, uint32_t entityIdIn);
BREPCORE_EXPORT static void AskEntityId(uint32_t* pNodeIdOut, uint32_t *pEntityIdOut, PK_ATTRIB_t attribTagIn);
BREPCORE_EXPORT static void DeleteEntityId(PK_ENTITY_t entityTag);

}; // PSolidTopoId

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidAttrib
{
BREPCORE_EXPORT static void GetAttrib(int* pNumAttribOut, PK_ATTRIB_t** ppAttribArrayOut, PK_ENTITY_t entityTagIn, char const*pAttribNameIn);
BREPCORE_EXPORT static void DeleteAttrib(PK_ENTITY_t entityTagIn, char* pAttribNameIn);

BREPCORE_EXPORT static BentleyStatus CreateEntityIdAttributeDef(bool isSessionStart = false);
BREPCORE_EXPORT static BentleyStatus CreateUserDataAttributeDef(bool isSessionStart = false);
BREPCORE_EXPORT static BentleyStatus CreateHiddenEntityAttributeDef(bool isSessionStart = false);
BREPCORE_EXPORT static BentleyStatus CreateFaceMaterialIndexAttributeDef(bool isSessionStart = false);

BREPCORE_EXPORT static BentleyStatus GetFaceMaterialIndexAttribute(int32_t& index, PK_FACE_t entityTag);
BREPCORE_EXPORT static BentleyStatus SetFaceMaterialIndexAttribute(PK_FACE_t entityTag, int32_t index);
BREPCORE_EXPORT static void DeleteFaceMaterialIndexAttribute(PK_ENTITY_t entityTag); // <= PK_FACE_t or PK_BODY_t...
BREPCORE_EXPORT static bool PopulateFaceMaterialIndexMap(T_FaceToAttachmentIndexMap& faceToIndexMap, PK_BODY_t entityTag, size_t numAttachments); // <= Returns false if invalid index was found attached to a face...

BREPCORE_EXPORT static BentleyStatus GetHiddenAttribute(bool& isHidden, PK_ENTITY_t entityTag);
BREPCORE_EXPORT static BentleyStatus SetHiddenAttribute(PK_ENTITY_t entityTag, bool isHidden);
BREPCORE_EXPORT static void DeleteHiddenAttribute(PK_ENTITY_t entityTag);

BREPCORE_EXPORT static BentleyStatus GetHiddenBodyFaces(bset<PK_FACE_t>& faces, PK_BODY_t body);
BREPCORE_EXPORT static BentleyStatus GetHiddenBodyEdges(bset<PK_EDGE_t>& edges, PK_BODY_t body);
BREPCORE_EXPORT static void DeleteHiddenAttributeOnEdges(PK_BODY_t entityTag);
BREPCORE_EXPORT static void DeleteHiddenAttributeOnFaces(PK_BODY_t entityTag);

BREPCORE_EXPORT static bool HasHiddenEdge(PK_BODY_t entityTag);
BREPCORE_EXPORT static bool HasHiddenFace(PK_BODY_t entityTag);
BREPCORE_EXPORT static bool IsEntityHidden(PK_ENTITY_t entity);

BREPCORE_EXPORT static BentleyStatus GetUserAttributes(bvector<int32_t>& attributes, PK_ENTITY_t entity, int32_t ownerId, int findIndex = 0);

}; // PSolidAttrib

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidTopo
{
BREPCORE_EXPORT static BentleyStatus GetBodyFaces(bvector<PK_FACE_t>& faces, PK_BODY_t body);
BREPCORE_EXPORT static BentleyStatus GetBodyEdges(bvector<PK_EDGE_t>& edges, PK_BODY_t body);
BREPCORE_EXPORT static BentleyStatus GetBodyVertices(bvector<PK_VERTEX_t>& vertices, PK_BODY_t body);
BREPCORE_EXPORT static BentleyStatus GetBodyEdgesAndFaces(bvector<PK_ENTITY_t>& edgesAndFaces, PK_BODY_t body);

BREPCORE_EXPORT static BentleyStatus GetFaceEdges(bvector<PK_EDGE_t>& edges, PK_FACE_t face);
BREPCORE_EXPORT static BentleyStatus GetFaceVertices(bvector<PK_VERTEX_t>& vertices, PK_FACE_t face);
BREPCORE_EXPORT static BentleyStatus GetFaceLoops(bvector<PK_LOOP_t>& loops, PK_FACE_t face);

BREPCORE_EXPORT static BentleyStatus GetEdgeFaces(bvector<PK_FACE_t>& faces, PK_EDGE_t edge);
BREPCORE_EXPORT static BentleyStatus GetEdgeVertices(bvector<PK_VERTEX_t>& vertices, PK_EDGE_t edge);
BREPCORE_EXPORT static BentleyStatus GetEdgeFins(bvector<PK_FIN_t>& fins, PK_EDGE_t edge);

BREPCORE_EXPORT static BentleyStatus GetVertexFaces(bvector<PK_FACE_t>& faces, PK_VERTEX_t vertex);
BREPCORE_EXPORT static BentleyStatus GetVertexEdges(bvector<PK_EDGE_t>& edges, PK_VERTEX_t vertex);

BREPCORE_EXPORT static BentleyStatus GetLoopFins(bvector<PK_FIN_t>& fins, PK_LOOP_t loop);
BREPCORE_EXPORT static BentleyStatus GetLoopEdgesFromEdge(bvector<PK_EDGE_t>& loopEdges, PK_EDGE_t edge, PK_FACE_t face);
BREPCORE_EXPORT static BentleyStatus GetTangentBlendEdges(bvector<PK_EDGE_t>& smoothEdges, PK_EDGE_t edgeTag);
BREPCORE_EXPORT static BentleyStatus GetCurveOfEdge(PK_CURVE_t& curveTagOut, double* startParamP, double* endParamP, bool* reversedP, PK_EDGE_t edgeTagIn);

}; // PSolidTopo

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidGeom
{
BREPCORE_EXPORT static BentleyStatus CreateBCurveFromSPCurve(PK_BCURVE_t& bCurveTag, PK_SPCURVE_t spCurve, PK_INTERVAL_t* intervalP, PK_LOGICAL_t& isExact, bool makeNonPeriodic = true);
BREPCORE_EXPORT static BentleyStatus CreateMSBsplineCurveFromBCurve(MSBsplineCurveR curve, PK_BCURVE_t curveTag, bool normalizeKnotVector = true);
BREPCORE_EXPORT static BentleyStatus CreateMSBsplineCurveFromSPCurve(MSBsplineCurveR curve, PK_SPCURVE_t spCurveTag, PK_INTERVAL_t* intervalP, bool* isExactP = NULL);
BREPCORE_EXPORT static BentleyStatus CreateMSBsplineCurveFromCurve(MSBsplineCurveR curve, PK_CURVE_t curveTag, PK_INTERVAL_t& interval, bool reverse = false, double tolerance = 1.0E-5, bool* isExactP = NULL);
BREPCORE_EXPORT static BentleyStatus CreateCurveFromMSBsplineCurve(PK_CURVE_t* curveTag, MSBsplineCurveCR curve);
BREPCORE_EXPORT static BentleyStatus CreateCurveFromMSBsplineCurve2d(PK_CURVE_t* curveTag, MSBsplineCurveCR curve);

BREPCORE_EXPORT static BentleyStatus CreateBSurfaceFromSurface(PK_BSURF_t& bSurfaceTag, PK_PARAM_sf_t param[2], PK_SURF_t surfTag, bool makeNonPeriodic = true);
BREPCORE_EXPORT static BentleyStatus CreateMSBsplineSurfaceFromFace(MSBsplineSurfaceR surface, PK_FACE_t faceTag, int uRules, int vRules, double tolerance);
BREPCORE_EXPORT static BentleyStatus CreateMSBsplineSurfaceFromSurface(MSBsplineSurfaceR surface, PK_SURF_t surfTag, PK_SURF_trim_data_t* trimData, PK_GEOM_t* spaceCurves, PK_INTERVAL_t* interval, int uRules, int vRules, double tolerance, bool normalizeSurface = true);
BREPCORE_EXPORT static BentleyStatus CreateSurfaceFromMSBsplineSurface(PK_BSURF_t* surfTag, MSBsplineSurfaceCR surface);
BREPCORE_EXPORT static BentleyStatus CreateSheetBodyFromTrimmedSurface(PK_ENTITY_t* bodyTagP, PK_ENTITY_t** spaceCurveEntitiesPP, PK_ENTITY_t** uvCurveEntitiesPP, int preferSpaceCurvesFlag, double** trimPP, int boundCount, PK_ENTITY_t surfaceTag, double tolerance, int adaptive);

BREPCORE_EXPORT static BentleyStatus BodyFromCurveVector(PK_BODY_t& bodyTag, PK_VERTEX_t* startVertexP, CurveVectorCR profile, TransformCR curveToBodyTransform, bool coverClosed = true, EdgeToCurveIdMap* idMap = NULL);
BREPCORE_EXPORT static BentleyStatus BodyFromPolyface(PK_BODY_t& bodyTag, PolyfaceQueryCR polyface, TransformCR dgnToSolid);
BREPCORE_EXPORT static BentleyStatus BodyFromMSBsplineSurface(PK_BODY_t& bodyTag, MSBsplineSurfaceCR surface);
BREPCORE_EXPORT static BentleyStatus BodyFromCone(PK_BODY_t& bodyTag, RotMatrixCR rMatrix, DPoint3dCR topCenter, DPoint3dCR bottomCenter, double topRadius, double bottomRadius, bool capped, uint32_t nodeId);
BREPCORE_EXPORT static BentleyStatus BodyFromLoft(PK_BODY_t& bodyTag, PK_BODY_t* profiles, PK_VERTEX_t* startVertices, size_t nProfiles, PK_BODY_t* guides, size_t nGuides, bool periodic);
BREPCORE_EXPORT static BentleyStatus BodyFromSweep(PK_BODY_t& bodyTag, PK_BODY_t profileTag, PK_BODY_t pathTag, PK_VERTEX_t pathVertex, bool alignParallel, bool selfRepair, DVec3dCP lockDirection, double const* twistAngle, double const* scale, DPoint3dCP scalePoint);

BREPCORE_EXPORT static CurveVectorPtr FaceToUVCurveVector(PK_FACE_t faceTag, PK_UVBOX_t* uvBox, bool splineParameterization);
BREPCORE_EXPORT static CurveVectorPtr PlanarFaceToCurveVector(PK_FACE_t face, EdgeToCurveIdMap const* idMap = nullptr);
BREPCORE_EXPORT static ISolidPrimitivePtr FaceToSolidPrimitive(PK_FACE_t faceTag, CurveVectorPtr* uvBoundaries = nullptr);
BREPCORE_EXPORT static StatusInt FaceToBSplineSurface(MSBsplineSurfacePtr& bSurface, CurveVectorPtr& uvBoundaries, PK_FACE_t faceTag);

BREPCORE_EXPORT static ICurvePrimitivePtr GetAsCurvePrimitive(PK_CURVE_t curve, PK_INTERVAL_t interval, bool reverseDirection);
BREPCORE_EXPORT static BentleyStatus EdgeToCurvePrimitive(ICurvePrimitivePtr& curvePrimitive, PK_EDGE_t edgeTag);
BREPCORE_EXPORT static BentleyStatus BodyToCurveVectors(bvector<CurveVectorPtr>& curves, IBRepEntityCR entity, EdgeToCurveIdMap const* idMap = NULL);
BREPCORE_EXPORT static CurveVectorPtr WireBodyToCurveVector(IBRepEntityCR entity);
BREPCORE_EXPORT static CurveVectorPtr PlanarSheetBodyToCurveVector(IBRepEntityCR entity);

BREPCORE_EXPORT static BentleyStatus BodyFromCurveVector(IBRepEntityPtr& entityOut, CurveVectorCR curveVector, TransformCP curveToDgn = NULL, uint32_t nodeId = 0L, EdgeToCurveIdMap* idMap = NULL); //!< Calls PSolidKernelManager::StartSession.
BREPCORE_EXPORT static BentleyStatus BodyFromSolidPrimitive(IBRepEntityPtr& out, ISolidPrimitiveCR primitive, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
BREPCORE_EXPORT static BentleyStatus BodyFromBSurface(IBRepEntityPtr& out, MSBsplineSurfaceCR surface, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
BREPCORE_EXPORT static BentleyStatus BodyFromPolyface(IBRepEntityPtr& out, PolyfaceQueryCR meshData, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
BREPCORE_EXPORT static BentleyStatus BodyFromLoft(IBRepEntityPtr& out, CurveVectorPtr* profiles, size_t nProfiles, CurveVectorPtr* guides, size_t nGuides, bool periodic, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
BREPCORE_EXPORT static BentleyStatus BodyFromSweep(IBRepEntityPtr& out, CurveVectorCR profile, CurveVectorCR path, bool alignParallel, bool selfRepair, bool createSheet,DVec3dCP lockDirection, double const* twistAngle, double const* scale, DPoint3dCP scalePoint, uint32_t nodeId = 0L); //!< Calls PSolidKernelManager::StartSession.
BREPCORE_EXPORT static BentleyStatus BodyFromExtrusionToBody(IBRepEntityPtr& target, IBRepEntityCR extrudeTo, IBRepEntityCR profile, bool reverseDirection, uint32_t nodeId = 0L);

}; // PSolidGeom

enum class SimplifiedGeometryType
    {
    Unknown,
    CurveVector,
    SolidPrimitive,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidSimplify
{
BREPCORE_EXPORT static SimplifiedGeometryType DeduceGeometryType(PK_BODY_t bodyTag);
BREPCORE_EXPORT static IGeometryPtr Simplify(PK_BODY_t bodyTag, bool removeRedundantTopology, SimplifiedGeometryType type = SimplifiedGeometryType::Unknown);
BREPCORE_EXPORT static BentleyStatus RemoveRedundantTopology(PK_BODY_t bodyTag);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidSubEntity
{
BREPCORE_EXPORT static ISubEntityPtr CreateSubEntity(PK_ENTITY_t entityTag, TransformCR entityTransform);
BREPCORE_EXPORT static ISubEntityPtr CreateSubEntity(PK_ENTITY_t entityTag, IBRepEntityCR parent);

BREPCORE_EXPORT static PK_ENTITY_t GetSubEntityTag(ISubEntityCR);
BREPCORE_EXPORT static Transform GetSubEntityTransform(ISubEntityCR);

}; // PSolidSubEntity

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidUtil
{
BREPCORE_EXPORT static IBRepEntityPtr CreateNewEntity(PK_ENTITY_t entityTag, TransformCR entityTransform, bool owned = true); //!< NOTE: Will return an invalid entity if entity tag is not valid.
BREPCORE_EXPORT static IBRepEntityPtr InstanceEntity(IBRepEntityCR, bool owned = false); //!< Create reference to an existing entity without doing a deep copy...

BREPCORE_EXPORT static BentleyStatus SaveEntityToMemory(uint8_t** ppBuffer, size_t& bufferSize, IBRepEntityCR); //!< NOTE: The entity transform must be saved separately.
BREPCORE_EXPORT static BentleyStatus RestoreEntityFromMemory(IBRepEntityPtr&, uint8_t const* pBuffer, size_t bufferSize, TransformCR); //!< Calls PSolidKernelManager::StartSession.

//! Get the tag value that the solid kernel uses to identify the entity.
//! @note To be used by application in query operations. Do not pass this to any kernel api that will modify the entity, use GetEntityTagForModify instead.
//! @return The tag value.
BREPCORE_EXPORT static PK_ENTITY_t GetEntityTag(IBRepEntityCR, bool* isOwned = nullptr);

//! Get the tag value of the solid kernel entity for modification.
//! @note To be used by application in operations where the entity is consumed or modified such as being a tool body in a union.
//! @remarks This method will return a null tag if the entity isn't "owned" by this instance (Application may then choose to copy the entity).
BREPCORE_EXPORT static PK_ENTITY_t GetEntityTagForModify(IBRepEntityR entity);

//! Extract and take ownership of solid kernel entity associated with this instance. Input entity will be set to the null tag and the caller is responsible for freeing extracted entity.
//! @note To be used by application in operations where the entity is consumed or modified such as being a tool body in a union.
//! @remarks This method will return a null tag if the entity isn't "owned" by this instance (Application may then choose to copy the entity).
BREPCORE_EXPORT static PK_ENTITY_t ExtractEntityTag(IBRepEntityR entity);

//! Set the tag value that the solid kernel uses to identify the entity and delete the existing entity.
//! @note To be used by applications after a modify operation where a new entity has been created such as having to copy a non-owned entity.
//! @return The true if entity tag was changed.
BREPCORE_EXPORT static bool SetEntityTag(IBRepEntityR entity, PK_ENTITY_t entityTag);

//! Return body tag for supplied face, edge, or vertex topological entity.
BREPCORE_EXPORT static PK_BODY_t GetBodyForEntity(PK_ENTITY_t entityTag);

//! Get axis aligned bounding box for the supplied body, face, or edge topological entity.
BREPCORE_EXPORT static BentleyStatus GetEntityRange(DRange3dR range, PK_TOPOL_t entity);

BREPCORE_EXPORT static IFaceMaterialAttachmentsPtr CreateNewFaceAttachments(PK_ENTITY_t entityTag, FaceAttachment baseAttachment);
BREPCORE_EXPORT static void SetFaceAttachments(IBRepEntityR, IFaceMaterialAttachmentsP);
BREPCORE_EXPORT static PK_FACE_t GetPreferredFaceAttachmentFaceForEdge(PK_EDGE_t edgeTag);
BREPCORE_EXPORT static PK_FACE_t GetPreferredFaceAttachmentFaceForVertex(PK_VERTEX_t vertexTag);

BREPCORE_EXPORT static PolyfaceHeaderPtr FacetEntity(IBRepEntityCR entity, IFacetOptionsCR);
BREPCORE_EXPORT static bool FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, IFacetOptionsCR facetOptions);
BREPCORE_EXPORT static PolyfaceHeaderPtr FacetEntity(IBRepEntityCR entity, double pixelSize=0.0, DRange1dP pixelSizeRange=nullptr);
BREPCORE_EXPORT static bool FacetEntity(IBRepEntityCR entity, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<FaceAttachment>& params, double pixelSize=0.0, DRange1dP pixelSizeRange=nullptr);

BREPCORE_EXPORT static bool HasCurvedFaceOrEdge(PK_BODY_t entityTag);
BREPCORE_EXPORT static bool HasOnlyPlanarFaces(PK_BODY_t entityTag);
BREPCORE_EXPORT static bool IsSmoothEdge(PK_EDGE_t edgeTag);
BREPCORE_EXPORT static bool IsPlanarFace(PK_FACE_t faceTag);

BREPCORE_EXPORT static BentleyStatus GetPlanarFaceData (DPoint3dP point, DVec3dP normal, PK_FACE_t entityTag);
BREPCORE_EXPORT static BentleyStatus EvaluateFace(DPoint3dR point, DVec3dR normal, DVec3dR uDir, DVec3dR vDir, DPoint2dCR uvParam, PK_FACE_t faceTag);
BREPCORE_EXPORT static BentleyStatus EvaluateEdge(DPoint3dR point, DVec3dR tangent, double uParam, PK_EDGE_t edgeTag);
BREPCORE_EXPORT static BentleyStatus GetVertex(DPoint3dR point, PK_VERTEX_t vertexTag);

BREPCORE_EXPORT static void ExtractStartAndSweepFromInterval(double& start, double& sweep, PK_INTERVAL_t const& interval, bool reverse);
BREPCORE_EXPORT static BentleyStatus MakeEllipseCurve(PK_CURVE_t* curveTag, double* startParam, double* endParam, DPoint3dP center, RotMatrixP rMatrix, double x1, double x2, double startAngle, double sweepAngle);
BREPCORE_EXPORT static BentleyStatus ImprintSegment(PK_BODY_t bodyTag, PK_EDGE_t* edgeTag, DPoint3dCP segment);
BREPCORE_EXPORT static BentleyStatus ImprintCurves(PK_ENTITY_t targetTag, bvector<PK_CURVE_t> const& toolCurves, bvector<PK_INTERVAL_t> const& toolIntervals, DVec3dCP direction = nullptr, bool extend = true, bool connectSides = true);
BREPCORE_EXPORT static BentleyStatus CoverWires(PK_BODY_t bodyTag);

BREPCORE_EXPORT static double CalculateToleranceFromMinCurvature(PK_CURVE_t curveTag, PK_INTERVAL_t* intervalP);
BREPCORE_EXPORT static void NormalizeBsplineCurve(MSBsplineCurveR curve);
BREPCORE_EXPORT static BentleyStatus FixupNonG1BodyGeometry(PK_BODY_t bodyTag);

BREPCORE_EXPORT static BentleyStatus CreateTransf(PK_TRANSF_t& transfTag, TransformCR transform);
BREPCORE_EXPORT static BentleyStatus ApplyTransform(PK_BODY_t bodyTag, PK_TRANSF_t transfTag);
BREPCORE_EXPORT static void GetTransforms(TransformR solidToUor, TransformR uorToSolid, DPoint3dCP origin = nullptr, double solidScale = 1.0);

BREPCORE_EXPORT static BentleyStatus ConvertSolidBodyToSheet(PK_BODY_t body);

BREPCORE_EXPORT static BentleyStatus SweepBodyVector(PK_BODY_t bodyTag, DVec3dCR direction, double distance);
BREPCORE_EXPORT static BentleyStatus SweepBodyAxis(PK_BODY_t bodyTag, DVec3dCR revolveAxis, DPoint3dCR center, double sweep);

BREPCORE_EXPORT static BentleyStatus Boolean(bvector<PK_BODY_t>* results, PK_boolean_function_t boolOpIn, bool generalTopology, PK_BODY_t& blankBodyIn, PK_BODY_t* pToolBodies, int numToolBodiesIn, PKIBooleanOptionEnum booleanOptions);
BREPCORE_EXPORT static BentleyStatus DisjoinBody(bvector<PK_BODY_t>& bodies, PK_BODY_t body);
BREPCORE_EXPORT static BentleyStatus CombineBody(PK_BODY_t targetBodyTag, PK_BODY_t toolBodyTag);
BREPCORE_EXPORT static BentleyStatus TransformBody(PK_BODY_t body, TransformCR transform);
BREPCORE_EXPORT static BentleyStatus FixBlends(PK_BODY_t bodyTag);

BREPCORE_EXPORT static BentleyStatus CheckBody(PK_BODY_t body, bool checkGeometry, bool checkTopology, bool checkSize);
BREPCORE_EXPORT static bool AreBodiesEqual(PK_BODY_t body1Tag, PK_BODY_t body2Tag, double tolerance, TransformCP deltaTransform1To2); // NOTE: Caller should first filter on body box to avoid more expensive coincident face compare...
BREPCORE_EXPORT static BentleyStatus MassProperties(double* amount, double* periphery, DPoint3dP centroid, double inertia[3][3], PK_BODY_t bodyTag, TransformCP transform, double tolerance);

BREPCORE_EXPORT static bool LocateSubEntities(PK_ENTITY_t bodyTag, TransformCR bodyTransform, bvector<PK_ENTITY_t>& subEntities, bvector<DPoint3d>& intersectPts, bvector<DPoint2d>& intersectParams, size_t maxFace, size_t maxEdge, size_t maxVertex, DRay3dCR boresite, double maxEdgeDistance, double maxVertexDistance);
BREPCORE_EXPORT static bool RayTestFace(PK_FACE_t faceTag, TransformCR bodyTransform, bvector<DPoint3d>& intersectPts, bvector<DPoint2d>& intersectParams, DRay3dCR boresite);
BREPCORE_EXPORT static bool ClosestPoint(PK_ENTITY_t bodyTag, TransformCR bodyTransform, PK_ENTITY_t& entityTag, DPoint3dR point, DPoint2dR param, double& distance, DPoint3dCR testPt);
BREPCORE_EXPORT static bool ClosestPointToFace(PK_FACE_t faceTag, TransformCR bodyTransform, DPoint3dR point, DPoint2dR uvParam, double& distance, DPoint3dCR testPt);
BREPCORE_EXPORT static bool ClosestPointToEdge(PK_EDGE_t edgeTag, TransformCR bodyTransform, DPoint3dR point, double& uParam, double& distance, DPoint3dCR testPt);

BREPCORE_EXPORT static BentleyStatus DoBoolean(IBRepEntityPtr& targetEntity, IBRepEntityPtr* toolEntities, size_t nTools, PK_boolean_function_t operation, PKIBooleanOptionEnum options = PKI_BOOLEAN_OPTION_AllowDisjoint, bool resolveNodeIdConflicts = true);

}; // PSolidUtil

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidKernelManager
{
//! Must be called with asset directory containing parasolid version files and directory to use for partitioned rollback temporary files. See DgnPlatformLib::Host::Initialize.
BREPCORE_EXPORT static void Initialize(BeFileNameCR assetPathW, BeFileNameCR tempDirBaseNameW);
//! Start parasolid session and register new frustrum.
BREPCORE_EXPORT static void StartSession();
//! Stop parasolid session.
BREPCORE_EXPORT static void StopSession();
//! Return whether parasolid session is currently started.
BREPCORE_EXPORT static bool IsSessionStarted();
//! Frustrum registered and session started by external dll (V8 convert)...
BREPCORE_EXPORT static bool SetExternalFrustrum(bool isActive);

}; // PSolidKernelManager

/*=================================================================================**//**
* @bsiclass
*
*  Utilities for managing error handling when using Parasolid from more than one thread.
*  A single lightweight partition is created for each thread.  The error handler is
*  replaced with a handler that will clear exclusions and rollback to the pMark when
*  a sever error occurs.  Without this error handling, a sever error in a thread using
*  parasolid will deadlock all thread.
*
+===============+===============+===============+===============+===============+======*/
struct  PSolidThreadUtil
{
/*=================================================================================**//**
* @bsiclass
*
*  The MainThreadMark object should be included once in the main thread.  It handles setting
*  up the thread local storage used by all threads and replaces the error handler with
*  one that handles severe errors and clears exclusions.
*
+===============+===============+===============+===============+===============+======*/
struct  MainThreadMark : RefCountedBase
    {
    private:
    BeThreadLocalStorage*       m_previousLocalStorage;

    public:
    BREPCORE_EXPORT MainThreadMark ();
    BREPCORE_EXPORT ~MainThreadMark ();
    };

/*=================================================================================**//**
* @bsiclass
*
*  The WorkerThreadOuterMark should be included once in each worker thread before any
*  Parasolid processing is initated.  It sets up a local storage for the worker thread
*  and creates a new lightweight partition in which all Parasolid processing should be
*  performed.  The destructor will delete the lightweight partition created for this
*  thread.
*  These cannot nest. Do not instantiate on main thread.
*
+===============+===============+===============+===============+===============+======*/
struct WorkerThreadOuterMark : RefCountedBase
    {
    public:

    BREPCORE_EXPORT WorkerThreadOuterMark ();
    BREPCORE_EXPORT ~WorkerThreadOuterMark ();
    };

/*=================================================================================**//**
* @bsiclass
*
*  This is alternative to WorkerThreadOuterMark that does not create a partition or
*  perform any rollbacks when errors are encountered. It is intended for "read-only"
*  multi-threaded processing of Parasolid bodies, and provides a significant performance
*  boost for that case (~10x WorkerThreadOuterMark).
*  These cannot nest. Do not instantiate on main thread.
*
+===============+===============+===============+===============+===============+======*/
struct WorkerThreadErrorHandler : RefCountedBase
    {
    BREPCORE_EXPORT WorkerThreadErrorHandler();
    BREPCORE_EXPORT ~WorkerThreadErrorHandler();
    };

BREPCORE_EXPORT static PK_PARTITION_t  GetThreadPartition();
BREPCORE_EXPORT static void  SetThreadPartitionMark();

typedef  RefCountedPtr<MainThreadMark>          MainThreadMarkPtr;
typedef  RefCountedPtr<WorkerThreadOuterMark>   WorkerThreadOuterMarkPtr;
typedef  RefCountedPtr<WorkerThreadErrorHandler> WorkerThreadErrorHandlerPtr;
};

END_BENTLEY_DGN_NAMESPACE

