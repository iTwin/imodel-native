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

#define PKI_ENTITY_ID_ATTRIB_NAME           "BSI_EntityId"
#define PKI_FACE_UNCHANGED_ATTRIB_NAME      "BSI_OldFace"
#define PKI_USERDATA_ATTRIB_NAME            "BSI_UserData"

BEGIN_BENTLEY_DGN_NAMESPACE

struct EdgeToCurveIdMap : bmap <uint32_t, CurvePrimitiveIdCP> {};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  01/01
+===============+===============+===============+===============+===============+======*/
struct PSolidUtil
{
DGNPLATFORM_EXPORT static BentleyStatus IdFromEntity (FaceId& faceId, PK_ENTITY_t entityTag, bool useHighestId) {return ERROR;}

DGNPLATFORM_EXPORT static BentleyStatus GetBodyFaces (bvector<PK_FACE_t>& faces, PK_BODY_t body);
DGNPLATFORM_EXPORT static BentleyStatus GetCurveOfEdge (PK_CURVE_t& curveTagOut, double* startParamP, double* endParamP, bool* reversedP, PK_EDGE_t edgeTagIn);
DGNPLATFORM_EXPORT static bool HasCurvedFacesOrEdges (PK_BODY_t entity);

DGNPLATFORM_EXPORT static CurveVectorPtr FaceToUVCurveVector (PK_FACE_t faceTag, PK_UVBOX_t* uvBox, bool splineParameterization);
DGNPLATFORM_EXPORT static CurveVectorPtr PlanarFaceToCurveVector (PK_FACE_t face, EdgeToCurveIdMap const* idMap = nullptr);
DGNPLATFORM_EXPORT static ISolidPrimitivePtr FaceToSolidPrimitive (PK_FACE_t faceTag, CurveVectorPtr* uvBoundaries = nullptr);
DGNPLATFORM_EXPORT static StatusInt FaceToBSplineSurface (MSBsplineSurfacePtr& bSurface, CurveVectorPtr& uvBoundaries, PK_FACE_t faceTag);

DGNPLATFORM_EXPORT static ICurvePrimitivePtr GetAsCurvePrimitive (PK_CURVE_t curve, PK_INTERVAL_t interval, bool reverseDirection);

DGNPLATFORM_EXPORT static void ExtractStartAndSweepFromInterval (double& start, double& sweep, PK_INTERVAL_t const& interval, bool reverse);
DGNPLATFORM_EXPORT static void NormalizeBsplineCurve (MSBsplineCurveR curve);
DGNPLATFORM_EXPORT static double CalculateToleranceFromMinCurvature (PK_CURVE_t curve, PK_INTERVAL_t* intervalP);
DGNPLATFORM_EXPORT static BentleyStatus FixupNonG1BodyGeometry (PK_BODY_t bodyTag);

DGNPLATFORM_EXPORT static BentleyStatus CreateBCurveFromSPCurve (PK_BCURVE_t& bCurveTag, PK_SPCURVE_t spCurve, PK_INTERVAL_t* intervalP, PK_LOGICAL_t& isExact, bool makeNonPeriodic = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineCurveFromBCurve (MSBsplineCurveR curve, PK_BCURVE_t curveTag, bool normalizeKnotVector = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineCurveFromSPCurve (MSBsplineCurveR curve, PK_SPCURVE_t spCurveTag, PK_INTERVAL_t* intervalP, bool* isExactP = NULL);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineCurveFromCurve (MSBsplineCurveR curve, PK_CURVE_t curveTag, PK_INTERVAL_t& interval, bool reverse = false, double tolerance = 1.0E-5, bool* isExactP = NULL);
DGNPLATFORM_EXPORT static BentleyStatus CreateCurveFromMSBsplineCurve (PK_CURVE_t* curveTag, MSBsplineCurveCR curve);
DGNPLATFORM_EXPORT static BentleyStatus CreateCurveFromMSBsplineCurve2d (PK_CURVE_t* curveTag, MSBsplineCurveCR curve);

DGNPLATFORM_EXPORT static BentleyStatus CreateBSurfaceFromSurface (PK_BSURF_t& bSurfaceTag, PK_PARAM_sf_t param[2], PK_SURF_t surfTag, bool makeNonPeriodic = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineSurfaceFromFace (MSBsplineSurfaceR surface, PK_FACE_t faceTag, int uRules, int vRules, double tolerance);
DGNPLATFORM_EXPORT static BentleyStatus CreateMSBsplineSurfaceFromSurface (MSBsplineSurfaceR surface, PK_SURF_t surfTag, PK_SURF_trim_data_t* trimData, PK_GEOM_t* spaceCurves, PK_INTERVAL_t* interval, int uRules, int vRules, double tolerance, bool normalizeSurface = true);
DGNPLATFORM_EXPORT static BentleyStatus CreateSurfaceFromMSBsplineSurface (PK_BSURF_t* surfTag, MSBsplineSurfaceCR surface);
DGNPLATFORM_EXPORT static BentleyStatus CreateSheetBodyFromTrimmedSurface (PK_ENTITY_t* bodyTagP, PK_ENTITY_t** spaceCurveEntitiesPP, PK_ENTITY_t** uvCurveEntitiesPP, int preferSpaceCurvesFlag, double** trimPP, int boundCount, PK_ENTITY_t surfaceTag, double tolerance, int adaptive);
DGNPLATFORM_EXPORT static BentleyStatus BodyFromMSBsplineSurface (PK_BODY_t& bodyTag, MSBsplineSurfaceCR surface);

}; // PSolidUtil

END_BENTLEY_DGN_NAMESPACE

